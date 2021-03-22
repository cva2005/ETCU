#include <stdlib.h>
#include "timers.h"
#include "arm_math.h"
#include "_control.h"
#include "la10p.h"

static stime_t step_time;
static stime_t err_time;
static la10p_st state;
static int32_t SensMax, SensMin;
static float32_t TaskVal = 0;
static float32_t FullTime;
#ifdef MODEL_OBJ
static float32_t CurrVal = 0;
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
	state = LA10P_READY;
	SensMax = SENS_MAX_VAL;
	SensMin = SENS_MIN_VAL;
	FullTime = LA10P_FULL_TIME;
#else
	SensMin = SENS_MAX_VAL;
	SensMax = SENS_MIN_VAL;
	state = LA10P_NOT_INIT;
	err_time = timers_get_finish_time(LA10P_INIT_TIME);
	step_time = timers_get_finish_time(STEP_TIME);
	set(FORWARD_MOV, ON);
#endif
}

void la10p_step(void) {
	if (timers_get_time_left(step_time) == 0) {
		step_time = timers_get_finish_time(STEP_TIME);
		if (state == LA10P_NOT_INIT) { // ToDo: обработчик ошибок
			int32_t st_sens = STATE_SENS();
			if (st(FORWARD_MOV)) { // движение вперед
				if (st_sens > SensMax) {
					SensMax = st_sens;
				} else {
					set(FORWARD_MOV, OFF);
					set(REVERS_MOV, ON);
					FullTime = 0;
				}
			} else { // движение назад
				if (st_sens < SensMin) {
					SensMin = st_sens;
					FullTime += STEP_TIME;
				} else {
					state = LA10P_READY;
					set(REVERS_MOV, OFF);
				}
			}
		} else { // state == LA10P_READY
			float32_t diff;
#ifdef MODEL_OBJ
			diff = STEP_TIME / FullTime;
			diff *= SensMax - SensMin;
			if (st(FORWARD_MOV)) { // движение вперед
				CurrVal += diff;
				if (CurrVal > SensMax) CurrVal = SensMax;
			} else if (st(REVERS_MOV)) { // движение назад
				CurrVal -= diff;
				if (CurrVal < SensMin) CurrVal = SensMin;
			}
#endif
			diff = TaskVal - STATE_SENS();
			diff /= SensMax - SensMin;
			diff *= FullTime;
			if (diff > RELE_TIME) {
				set(FORWARD_MOV, ON);
				set(REVERS_MOV, OFF);
			} else if (diff < -RELE_TIME) {
				set(REVERS_MOV, ON);
				set(FORWARD_MOV, OFF);
			} else {
				if (st(FORWARD_MOV)) { // движение вперед
					if (diff < STEP_TIME / 2) set(FORWARD_MOV, OFF);
				} else if (st(REVERS_MOV)) { // движение назад
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
