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
 * ������������� ���������� ��� ������ �������
 * ������������� �������� ������������ � �������������
 * ��������� ������������ ������������ �� ��������� ����������
 * � �������� � ��������� ����� �������� ���������.
 * ��������� ���������� ��� ���������� ������� �� ����
 * ������� ���������� �� ��������������� ��������� � ����
 * ������� �������.
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

// ToDo: �������� ������� �������� ������ ����
// ��� ����������� ������ � ������� ������ ����������
void la10p_step(void) {
	if (timers_get_time_left(step_time) == 0) {
		step_time = timers_get_finish_time(STEP_TIME);
		if (state == LA10P_STOP_ERR) { // ������ ������������
			return;
		} else if (state == LA10P_NOT_INIT) {
			CurrNull = CURR_SENS_VAL;
#ifdef MODEL_OBJ
			state = LA10P_READY;
#else
			state = LA10P_INIT_RUN;
#endif
			set(FORWARD_MOV, ON);
		} else if (state == LA10P_INIT_RUN) {
			if (timers_get_time_left(err_time) == 0) {
				la10p_error:
				state = LA10P_STOP_ERR;
				return;
			}
			float32_t st_sens = STATE_SENS();
			if (st(FORWARD_MOV)) { // �������� ������
				if (CURR_SENS_A > SENS_I_MAX) goto forw_stop;
				if (st_sens > SensMax) {
					SensMax = st_sens;
				} else {
					forw_stop:
					set(FORWARD_MOV, OFF);
					if ((SensMax == SENS_MIN_VAL) || // �� ���������
						(SensMax > SENS_MAX_VAL) || // ������ �����. �������
						(SensMax < SENS_MAX_VAL / 4)) { // ���� �����. �������
						goto la10p_error;
					}
					set(REVERS_MOV, ON);
					FullTime = 0;
					err_time = timers_get_finish_time(LA10P_ERR_TIME);
				}
			} else { // �������� �����
				if (CURR_SENS_A > SENS_I_MAX) goto revers_stop;
				if (st_sens < SensMin) {
					SensMin = st_sens;
#ifdef MODEL_OBJ
					FullTime += STEP_TIME;
#endif
				} else {
					revers_stop:
					if ((SensMin == SENS_MAX_VAL) || // �� ���������
						(SensMin < SENS_MIN_VAL) || // ���� �����. �������
						(timers_get_time_left(err_time) < LA10P_MIN_TIME) || // ���� ����� ����
						(SensMax - SensMin < SENS_MAX_VAL / 2) || // ��� ������. ����. �������
						(SensMin > SENS_MAX_VAL / 2)) { // ������ �����. �������
						goto la10p_error;
					}
					state = LA10P_READY;
					set(REVERS_MOV, OFF);
					// ToDo: ��������� ��������� 10...90%
				}
			}
		} else { // state == LA10P_READY
			curr = (CURR_SENS_A * 1000.0);
			if (CURR_SENS_A > SENS_I_MAX) goto la10p_error;
			float32_t diff;
#ifdef MODEL_OBJ
			diff = STEP_TIME / FullTime;
			diff *= SensMax - SensMin;
			if (st(FORWARD_MOV)) { // �������� ������
				StateVal += diff;
				if (StateVal > SensMax) StateVal = SensMax;
			} else if (st(REVERS_MOV)) { // �������� �����
				StateVal -= diff;
				if (StateVal < SensMin) StateVal = SensMin;
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
				if (st(FORWARD_MOV)) { // �������� ������
					if (diff < STEP_TIME / 2) set(FORWARD_MOV, OFF);
				} else if (st(REVERS_MOV)) { // �������� �����
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