#include <stdlib.h>
#include "timers.h"
#include "arm_math.h"
#include "_control.h"
#include "la10p.h"

static stime_t step_time;
static stime_t err_time;
static la10p_st state;

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
#ifdef MODEL_NO_SERVO
	state = LA10P_READY;
#else
	state = LA10P_NOT_INIT;
	err_time = timers_get_finish_time(LA10P_INIT_TIME);
	step_time = timers_get_finish_time(STEP_TIME);
	set(FORWARD_MOV, ON);
#endif
}

void la10p_step(void) {
	if (timers_get_time_left(step_time) == 0) {
		step_time = timers_get_finish_time(STEP_TIME);
		if (state == LA10P_NOT_INIT) {
			if (st(FORWARD_MOV)) { // движение вперед
			} else if (st(REVERS_MOV)) { // движение назад
			} else { // привод остановлен
			}
		} else { // state == LA10P_READY
		}
	}
}

/* servo position in % */
float32_t la10p_get_pos(void) {
	return STATE_SENS();
}

/* set position of control */
void la10p_set_out(float32_t pid_out) {
}
