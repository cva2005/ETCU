#include <stdlib.h>
#include "timers.h"
#include "arm_math.h"
#include "_control.h"
#include "la10pwm.h"

static stime_t step_time;
static stime_t err_time;
static la10p_st state;
static float32_t SensMax, SensMin;
static float32_t TaskVal = 0;
static float32_t CurrNull;
static float32_t FullTime;
#ifdef MODEL_NO_SERVO
static float32_t StateVal = 0;
#endif
int32_t curr;
TIM_HandleTypeDef pwm_tim;

static void TIM_PWM_Init(void);
static void set_PWM_out(float32_t out);
static float32_t get_PWM_out(void);

void la10p_init(void) {
	TIM_PWM_Init();
#ifdef MODEL_NO_SERVO
	SensMax = SENS_MAX_VAL;
	SensMin = SENS_MIN_VAL;
	FullTime = LA10P_FULL_TIME;
#else
	SensMin = SENS_MAX_VAL;
	SensMax = SENS_MIN_VAL;
	err_time = timers_get_finish_time(LA10P_ERR_TIME);
#endif
	state = LA10PWM_NOT_INIT;
	step_time = timers_get_finish_time(STEP_TIME);
}

void la10p_step(void) {
	if (timers_get_time_left(step_time) == 0) {
		step_time = timers_get_finish_time(STEP_TIME);
		if (state == LA10PWM_STOP_ERR) { // ������ ������������
			return;
		} else if (state == LA10PWM_NOT_INIT) {
			CurrNull = CURR_SENS_VAL;
#ifdef MODEL_NO_SERVO
			state = LA10PWM_READY;
#else
			state = LA10PWM_INIT_RUN;
			set_PWM_out(1);
#endif
		} else if (state == LA10PWM_INIT_RUN) {
			if (timers_get_time_left(err_time) == 0) {
				la10p_error:
				set_PWM_out(0);
				state = LA10PWM_STOP_ERR;
				return;
			}
			float32_t st_sens = STATE_SENS();
			float32_t curr_sens = CURR_SENS_A;
			if (get_PWM_out()) { // �������� ������
				if ((curr_sens > SENS_I_MAX) || (curr_sens < SENS_I_OFF)) {
					goto forw_stop;
				}
				if (st_sens > SensMax) {
					SensMax = st_sens;
				} else {
					forw_stop:
					set_PWM_out(0);
					// ToDo: � ������� ���������� ��������� ����� �����
					if ((SensMax == SENS_MIN_VAL) || // �� ���������
						(SensMax > SENS_MAX_VAL) || // ������ �����. �������
						(SensMax < SENS_MAX_VAL / 4)) { // ���� �����. �������
						goto la10p_error;
					}
					set_PWM_out(-1.0);
					FullTime = 0;
					err_time = timers_get_finish_time(LA10P_ERR_TIME);
				}
			} else { // �������� �����
				// ToDo: ������ �����������?
				/*if ((curr_sens > SENS_I_MAX) || (curr_sens < SENS_I_OFF)) {
					goto revers_stop;
				}*/
				if (st_sens < SensMin) {
					SensMin = st_sens;
					FullTime += STEP_TIME;
				} else {
					revers_stop:
					if ((SensMin == SENS_MAX_VAL) || // �� ���������
						(SensMin < SENS_MIN_VAL) || // ���� �����. �������
						(timers_get_time_left(err_time) > LA10P_MIN_TIME) || // ���� ����� ����
						(SensMax - SensMin < SENS_MAX_VAL / 2) || // ��� ������. ����. �������
						(SensMin > SENS_MAX_VAL / 2)) { // ������ �����. �������
						goto la10p_error;
					}
					state = LA10PWM_READY;
					set_PWM_out(0);
					// ToDo: ��������� ��������� 10...90%
				}
			}
		} else { // state == LA10P_READY
			float32_t curr_sens = CURR_SENS_A;
			curr = (curr_sens * 1000.0);
			if (curr_sens > SENS_I_MAX) goto la10p_error;
			float32_t diff;
#ifdef MODEL_NO_SERVO
			diff = get_PWM_out() * STEP_TIME;
			diff *= TIME_CORR;
			diff /= FullTime;
			diff *= SensMax - SensMin;
			StateVal += diff;
			if (StateVal > SensMax) StateVal = SensMax;
			else if (StateVal < SensMin) StateVal = SensMin;
#endif
			diff = TaskVal - STATE_SENS();
			diff /= SensMax - SensMin;
			set_PWM_out(diff);
		}
	}
}

/* servo position in % */
float32_t la10p_get_pos(void) {
	float32_t pos = ((STATE_SENS() - SensMin) / (SensMax - SensMin)) * 100.0;
	if (pos < 0) pos = 0;
	if (pos > 100) pos = 100;
	return pos;
}

/* set position of control */
void la10p_set_out(float32_t pid_out) {
	TaskVal = pid_out * LA10P_MUL;
	if (TaskVal > (float32_t)SensMax) TaskVal = SensMax;
	if (TaskVal < (float32_t)SensMin) TaskVal = SensMin;
}

la10p_st la10p_state(void) {
	return state;
}

static void set_PWM_out(float32_t out) {
	if (out > -ZONE_DEAD && out < ZONE_DEAD) {
		FORW_DUTY = 0;
		REVR_DUTY = 0;
		return;
	}
	if (out < -DUTY_MAX) out = -DUTY_MAX;
	if (out > DUTY_MAX) out = DUTY_MAX;
	out *= (DUTY_MAX - DUTY_MIN);
	if (out > 0) {
		out += DUTY_MIN;
		out *= PWM_PRD_VAL;
		REVR_DUTY = 0;
		FORW_DUTY = (uint16_t)out;
	} else {
		out -= DUTY_MIN;
		out *= PWM_PRD_VAL;
		FORW_DUTY = 0;
		REVR_DUTY = (uint16_t)(-out);
	}
}

static float32_t get_PWM_out(void) {
	float32_t out;
	if (FORW_DUTY) out = (float32_t)FORW_DUTY;
	else out = (float32_t)REVR_DUTY * -1.0;
	out /= PWM_PRD_VAL;
	if (out > DUTY_MIN) {
		out -= DUTY_MIN;
		out /= DUTY_MAX - DUTY_MIN;
	}
	return out;
}

static void TIM_PWM_Init(void) {
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;
	GPIO_InitTypeDef GPIO_InitStruct;
	pwm_tim.Instance = TIM_INST;
	pwm_tim.Init.Prescaler = 0;
	pwm_tim.Init.CounterMode = TIM_COUNTERMODE_UP;
	pwm_tim.Init.Period = PWM_PRD_VAL;
	pwm_tim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	TIM_CLK_ENABLE();
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&pwm_tim, &sClockSourceConfig);
	HAL_TIM_PWM_Init(&pwm_tim);
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&pwm_tim, &sMasterConfig);
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	HAL_TIM_PWM_ConfigChannel(&pwm_tim, &sConfigOC, PWM_CH_REVR);
	sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
	HAL_TIM_PWM_ConfigChannel(&pwm_tim, &sConfigOC, PWM_CH_FORW);
	GPIO_InitStruct.Pin = PWM_F_PIN | PWM_R_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF1_INST;
	HAL_GPIO_Init(PWM_PORT, &GPIO_InitStruct);
	HAL_TIM_PWM_Start(&pwm_tim, PWM_CH_REVR);
	HAL_TIM_PWM_Start(&pwm_tim, PWM_CH_FORW);
}