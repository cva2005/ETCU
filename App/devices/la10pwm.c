#include <stdlib.h>
#include <stdbool.h>
#include "timers.h"
#include "arm_math.h"
#include "_control.h"
#include "la10pwm.h"

static la10p_st state = LA10P_POWERED;
static stime_t step_time;
static stime_t err_time;
static float32_t SensMax, SensMin;
static float32_t TaskVal;
static float32_t CurrNull;
static float32_t FullTime;
#if MODEL_NO_SERVO
static float32_t StateVal;
#endif
int32_t curr;
extern TIM_HandleTypeDef pwm_tim;

static void TIM_PWM_Init(void);
static void set_PWM_out(float32_t out);
static float32_t get_PWM_out(void);

void la10p_init(void) {
	TIM_PWM_Init();
	SensMin = SENS_MAX_VAL;
	SensMax = SENS_MIN_VAL;
	err_time = timers_get_finish_time(LA10P_ERR_TIME);
	step_time = timers_get_finish_time(STEP_TIME);
	state = LA10P_NOT_INIT;
	TaskVal = SENS_MIN_VAL;
#if MODEL_NO_SERVO
	StateVal = SENS_MIN_VAL;
#endif
}

void la10p_step(void) {
	if (timers_get_time_left(step_time) == 0) {
		step_time = timers_get_finish_time(STEP_TIME);
		if (state == LA10P_STOP_ERR) { // ошибка сервопривода
			return;
		} else if (state == LA10P_NOT_INIT) {
			CurrNull = CURR_SENS_VAL;
			state = LA10P_INIT_RUN;
			set_PWM_out(FORW_MAX);
		} else if (state == LA10P_INIT_RUN) {
			if (timers_get_time_left(err_time) == 0) {
				la10p_error:
				set_PWM_out(STOP_MOV);
				state = LA10P_STOP_ERR;
				return;
			}
#if MODEL_NO_SERVO
			float32_t diff = (float32_t)STEP_TIME / (float32_t)LA10P_FULL_TIME;
			diff *= SENS_MAX_VAL - SENS_MIN_VAL;
			if (FORW_MOV) { // движение вперед
				StateVal += diff;
				if (StateVal > SENS_MAX_VAL) StateVal = SENS_MAX_VAL;
			} else if (REVR_MOV) { // движение назад
				StateVal -= diff;
				if (StateVal < SENS_MIN_VAL) StateVal = SENS_MIN_VAL;
			}
#endif
			float32_t st_sens = STATE_SENS();
			float32_t curr_sens = /*CURR_SENS_A*/0;
			if (FORW_MOV) { // движение вперед
				if ((curr_sens > SENS_I_MAX) /*|| (curr_sens < SENS_I_OFF)*/) {
					goto forw_stop;
				}
				if (st_sens > SensMax) {
					SensMax = st_sens;
				} else {
					forw_stop:
					set_PWM_out(STOP_MOV);
					// ToDo: в крайнем выдвинутом положении сразу назад
					if ((SensMax == SENS_MIN_VAL) || // не двигались
						(SensMax > SENS_MAX_VAL) || // велико показ. датчика
						(SensMax < SENS_MAX_VAL / 4)) { // мало показ. датчика
						goto la10p_error;
					}
					set_PWM_out(REVR_MAX);
					FullTime = 0;
					err_time = timers_get_finish_time(LA10P_ERR_TIME);
				}
			} else { // движение назад
				// ToDo: почему срабатывает?
				/*if ((curr_sens > SENS_I_MAX) || (curr_sens < SENS_I_OFF)) {
					goto revers_stop;
				}*/
				if (st_sens < SensMin) {
					SensMin = st_sens;
					FullTime += STEP_TIME;
				} else {
					revers_stop:
					if ((SensMin == SENS_MAX_VAL) || // не двигались
						(SensMin < SENS_MIN_VAL) || // мало показ. датчика
						(timers_get_time_left(err_time) > LA10P_MIN_TIME) || // мало время хода
						(SensMax - SensMin < SENS_MAX_VAL / 2) || // мал диапаз. знач. датчика
						(SensMin > SENS_MAX_VAL / 2)) { // велико показ. датчика
						goto la10p_error;
					}
					state = LA10P_READY;
					set_PWM_out(STOP_MOV);
					float32_t diff = SensMax - SensMin;
					diff *= 0.10; // коррекция диапазона 10...90%
					SensMin += diff;
					SensMax -= diff;
				}
			}
		} else { // state == LA10P_READY
#if 0
			float32_t curr_sens = CURR_SENS_A;
			curr = (curr_sens * 1000.0);
			if (curr_sens > SENS_I_MAX) goto la10p_error;
#endif
			float32_t diff;
#if MODEL_NO_SERVO
			diff = get_PWM_out() * STEP_TIME;
			//diff *= TIME_CORR;
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
#if MODEL_NO_SERVO
	float32_t pos = ((STATE_SENS() - SENS_MIN_VAL)
			/ (SENS_MAX_VAL - SENS_MIN_VAL)) * 100.0;
#else
	float32_t pos;
	if (state == LA10P_READY) {
		pos = ((STATE_SENS() - SensMin)
				/ (SensMax - SensMin)) * 100.0;
	} else {
		pos = ((STATE_SENS() - SENS_MIN_VAL)
				/ (SENS_MAX_VAL - SENS_MIN_VAL)) * 100.0;
	}
#endif
	if (pos < 0) pos = 0;
	if (pos > 100) pos = 100;
	return pos;
}

/* set position of control */
void la10p_set_out(float32_t pid_out) {
	TaskVal = SensMin + pid_out * LA10P_MUL;
	if (TaskVal > (float32_t)SensMax) TaskVal = SensMax;
	if (TaskVal < (float32_t)SensMin) TaskVal = SensMin;
}

la10p_st la10p_state(void) {
	return state;
}

static void set_PWM_out(float32_t out) {
	bool revers;
	if (out > -ZONE_DEAD && out < ZONE_DEAD) {
		FORW_DUTY = 0;
		REVR_DUTY = /*PWM_PRD_VAL*/0;
		return;
	}
	if (out < 0) {
		revers = true;
		out *= -1.0f;
		FORW_DUTY = 0;
	} else {
		revers = false;
		REVR_DUTY = /*PWM_PRD_VAL*/0;
	}
	if (out > DUTY_MAX) out = DUTY_MAX;
	out *= (DUTY_MAX - DUTY_MIN);
	out += DUTY_MIN;
	out *= PWM_PRD_VAL;
	uint16_t duty = (uint16_t)out;
	if (revers) {
		REVR_DUTY = /*PWM_PRD_VAL -*/ duty;
	}
	else FORW_DUTY = duty;
}

static float32_t get_PWM_out(void) {
	float32_t out; bool revers = false;
	if (FORW_DUTY) {
		out = (float32_t)FORW_DUTY;
	} else {
		revers = true;
		out = (float32_t)(PWM_PRD_VAL - REVR_DUTY);
	}
	out /= PWM_PRD_VAL;
	/*if (out > DUTY_MIN) {
		out -= DUTY_MIN;
		out /= DUTY_MAX - DUTY_MIN;
	}*/
	if (revers) out *= -1.0f;
	return out;
}

/*
 * PWM Period: 100uS (Fpwm = 100kHz)
 */
static void TIM_PWM_Init(void) {
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;
	pwm_tim.Instance = TIM_INST;
	pwm_tim.Init.Prescaler = 0;
	pwm_tim.Init.CounterMode = TIM_COUNTERMODE_UP;
	pwm_tim.Init.Period = PWM_PRD_VAL;
	pwm_tim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&pwm_tim);
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&pwm_tim, &sMasterConfig);
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&pwm_tim, &sConfigOC, PWM_CH_REVR);
	HAL_TIM_PWM_ConfigChannel(&pwm_tim, &sConfigOC, PWM_CH_FORW);
	HAL_TIM_PWM_Start(&htim1, PWM_CH_REVR);
	HAL_TIM_PWM_Start(&htim1, PWM_CH_FORW);
	REVR_DUTY = PWM_PRD_VAL;
}
