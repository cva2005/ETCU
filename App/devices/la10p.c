#include <stdlib.h>
#include "timers.h"
#include "arm_math.h"
#include "_control.h"
#include "la10p.h"

static stime_t step_time;
static stime_t err_time;
static la10p_st state;
static float32_t SensMax, SensMin;
static float32_t TaskVal = 0;
static float32_t CurrNull;
#ifdef MODEL_OBJ
static float32_t StateVal = 0;
static float32_t FullTime;
int32_t curr;
#endif

/*
 * Инициализация проводится при подаче питания
 * Калибровочные значения минимального и максимального
 * положения сервопривода определяются по отсутсвию приращения
 * в конечной и начальной точке рабочего диапазона.
 * Остановка происходит при увеличении момента за счет
 * падения напряжения на ограничительном резисторе в цепи
 * питания привода.
 */
void la10p_init(void) {
#ifdef MODEL_OBJ
	SensMax = SENS_MAX_VAL;
	SensMin = SENS_MIN_VAL;
	FullTime = LA10P_FULL_TIME;
#else
	SensMin = SENS_MAX_VAL;
	SensMax = SENS_MIN_VAL;
	err_time = timers_get_finish_time(LA10P_ERR_TIME);
#endif
	step_time = timers_get_finish_time(STEP_TIME);
	state = LA10P_NOT_INIT;
}

void la10p_step(void) {
	if (timers_get_time_left(step_time) == 0) {
		step_time = timers_get_finish_time(STEP_TIME);
		if (state == LA10P_STOP_ERR) { // ошибка сервопривода
			return;
		} else if (state == LA10P_NOT_INIT) {
			CurrNull = CURR_SENS_VAL;
#ifdef MODEL_OBJ
			state = LA10P_READY;
#else
			state = LA10P_INIT_RUN;
			set(FORWARD_MOV, ON);
#endif
		} else if (state == LA10P_INIT_RUN) {
			if (timers_get_time_left(err_time) == 0) {
				la10p_error:
				set(REVERS_MOV, OFF);
				set(FORWARD_MOV, OFF);
				state = LA10P_STOP_ERR;
				return;
			}
			float32_t st_sens = STATE_SENS();
			float32_t curr_sens = CURR_SENS_A;
			if (st(FORWARD_MOV)) { // движение вперед
				if ((curr_sens > SENS_I_MAX) || (curr_sens < SENS_I_OFF)) {
					goto forw_stop;
				}
				if (st_sens > SensMax) {
					SensMax = st_sens;
				} else {
					forw_stop:
					set(FORWARD_MOV, OFF);
					if ((SensMax == SENS_MIN_VAL) || // не двигались
						(SensMax > SENS_MAX_VAL) || // велико показ. датчика
						(SensMax < SENS_MAX_VAL / 4)) { // мало показ. датчика
						goto la10p_error;
					}
					set(REVERS_MOV, ON);
					FullTime = 0;
					err_time = timers_get_finish_time(LA10P_ERR_TIME);
				}
			} else { // движение назад
				if ((curr_sens > SENS_I_MAX) || (curr_sens < SENS_I_OFF)) {
					goto revers_stop;
				}
				if (st_sens < SensMin) {
					SensMin = st_sens;
#ifdef MODEL_OBJ
					FullTime += STEP_TIME;
#endif
				} else {
					revers_stop:
					if ((SensMin == SENS_MAX_VAL) || // не двигались
						(SensMin < SENS_MIN_VAL) || // мало показ. датчика
						(timers_get_time_left(err_time) < LA10P_MIN_TIME) || // мало время хода
						(SensMax - SensMin < SENS_MAX_VAL / 2) || // мал диапаз. знач. датчика
						(SensMin > SENS_MAX_VAL / 2)) { // велико показ. датчика
						goto la10p_error;
					}
					state = LA10P_READY;
					set(REVERS_MOV, OFF);
					// ToDo: коррекция диапазона 10...90%
				}
			}
		} else { // state == LA10P_READY
			float32_t curr_sens = CURR_SENS_A;
			if (curr_sens > SENS_I_MAX) goto la10p_error;
			float32_t diff;
#ifdef MODEL_OBJ
			curr = (curr_sens * 1000.0);
			diff = STEP_TIME / FullTime;
			diff *= SensMax - SensMin;
			if (st(FORWARD_MOV)) { // движение вперед
				StateVal += diff;
				if (StateVal > SensMax) StateVal = SensMax;
			} else if (st(REVERS_MOV)) { // движение назад
				StateVal -= diff;
				if (StateVal < SensMin) StateVal = SensMin;
			}
#endif
			diff = TaskVal - STATE_SENS();
			diff /= SensMax - SensMin;
			diff *= FullTime;
			if (diff > RELE_TIME) {
				set(REVERS_MOV, OFF);
				set(FORWARD_MOV, ON);
			} else if (diff < -RELE_TIME) {
				set(FORWARD_MOV, OFF);
				set(REVERS_MOV, ON);
			} else {
				if (st(FORWARD_MOV)) { // движение вперед
					if (curr_sens < SENS_I_OFF) goto la10p_error;
					if (diff < STEP_TIME / 2) set(FORWARD_MOV, OFF);
				} else if (st(REVERS_MOV)) { // движение назад
					if (curr_sens < SENS_I_OFF) goto la10p_error;
					if (diff > -STEP_TIME / 2) set(REVERS_MOV, OFF);
				}
			}
		}
	}
}

/* servo position in % */
float32_t la10p_get_pos(void) {
	float32_t full = SensMax - SensMin;
	return (STATE_SENS() / full) * 100.0;
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
