#include <stdlib.h>
#include "timers.h"
#include "arm_math.h"
#include "_control.h"
#include "bcu.h"
#include "servo.h"

uint32_t FullTime, CurrTime, TaskTime;
uint32_t ForwCurrMax, RevCurrMax, ErrCurrMax, CurrFilterOut, CurrTmpVal;
static stime_t time;
static servo_st state;
stime_t err_time; // искл. токов включения СП
bool servo_stop = false;
bool curr_null = false;
bool Pulse = false;
bool CurrInitForw = false;
bool CurrInitRev = false;
int32_t ServoCount = 0;
int32_t FullCount = 0;
uint32_t StateCnt = 0;

mov_t mov_state(void)
{
#ifndef SERVO_MODEL
	uint8_t in_data = bcu_get_in();
	bool pulse_in = in_data & PULSE_MASK;
	if (in_data & NULL_MOV_MASK) {
#else
	if (CurrTime <= 0) {
#endif
stop_null:
		CurrTime = 0;
		set(REVERS_MOV, OFF);
		servo_stop = false;
		curr_null = true;
		return SERVO_STOP_NULL;
#ifndef SERVO_MODEL
	} else if (in_data & FULL_MOV_MASK) {
#else
	} else if (CurrTime >= FullTime) {
#endif
		CurrTime = FullTime;
		set(FORWARD_MOV, OFF);
		return SERVO_STOP_FULL;
	} else if (st(FORWARD_MOV)) {
		if (pulse_in != Pulse) {
			if (StateCnt++ >= TRUE_STATE) {
				Pulse = pulse_in;
				if (pulse_in)  ServoCount++;
			}
		} else StateCnt = 0;
		curr_null = false;
		return SERVO_FORWARD;
	} else if (st(REVERS_MOV)) {
		if (pulse_in != Pulse) {
			if (StateCnt++ >= TRUE_STATE) {
				Pulse = pulse_in;
				if (pulse_in)  ServoCount--;
			}
		} else StateCnt = 0;
		return SERVO_REVERS;
	} else if (servo_stop && !curr_null) {
		if (CURR_REVR_HIGH) { // превышение тока закрытие
			ServoCount = -CNT_SAVE_VAL;
			goto stop_null;
		} else {
			if (ServoCount > 0) { // коррекция положения "0"
				if (CurrTime == 0) {
					CurrTime = SERVO_STEP_TIME * ServoCount;
				}
			} else {
				ServoCount = 0;
				goto stop_null;
			}
		}

	}
	return SERVO_STOP_ALL;
}

void servo_init(void)
{
#ifndef SERVO_MODEL
	state = SERVO_NOT_INIT;
	ForwCurrMax = RevCurrMax = ErrCurrMax = CURR_ERR;
	CurrFilterOut = 0;
	time = timers_get_finish_time(SERVO_INIT_TIME);
	set(FORWARD_MOV, ON);
	err_time = timers_get_finish_time(SERVO_ON_ERR);
#else
	set(FORWARD_MOV, OFF);
	set(REVERS_MOV, OFF);
	state = SERVO_READY;
	time = timers_get_finish_time(SERVO_STEP_TIME);
	FullTime = SERVO_INIT_TIME / 2;
	CurrTime = TaskTime = 0;
#endif
}

void servo_step(void)
{
	uint32_t tmp;
	mov_t mov = mov_state();
	CurrFilterOut = CURR_FLT_OUT();
	if (state == SERVO_NOT_INIT) {
		tmp = timers_get_time_left(time);
		if (tmp < INIT_ERROR_TIME) {
			if (ServoCount == 0) tmp = 0;
		}
		if (tmp == 0) {
			state = SERVO_STOP_ERR;
			set(FORWARD_MOV, OFF);
			set(REVERS_MOV, OFF);
			return;
		}
		if (mov == SERVO_STOP_FULL) {
stop_full: 	time = timers_get_finish_time(SERVO_INIT_TIME);
			ServoCount = 0;
			set(FORWARD_MOV, OFF);
			set(REVERS_MOV, ON);
			err_time = timers_get_finish_time(SERVO_ON_ERR);
		} else if (mov == SERVO_STOP_NULL) {
stop_null: 	FullTime = SERVO_INIT_TIME - timers_get_time_left(time);
			if (FullTime < SERVO_DEL_TIME) return;
			FullTime -= CORRECT_FULL;
			CurrTime = 0;
			FullCount = - ServoCount - (CNT_SAVE_VAL * 2);
			ServoCount = -CNT_SAVE_VAL;
			curr_null = true;
			set(REVERS_MOV, OFF);
			state = SERVO_READY;
			time = timers_get_finish_time(SERVO_STEP_TIME);
		} else if (CURR_FORW_HIGH) {
			if (timers_get_time_left(err_time) == 0) {
				if (mov == SERVO_FORWARD) {
					goto stop_full; // превышение тока открытие
				} else if (mov == SERVO_REVERS) {
					if (CURR_REVR_HIGH) { // превышение тока закрытие
						goto stop_null;
					}
				}
			}
		} else { // адаптация токового датчика
			if (timers_get_time_left(err_time) > 0) return;
			if (CurrInitForw == false) {
				if (mov == SERVO_FORWARD) {
					if (ServoCount > CNT_INIT_VAL) {
						ForwCurrMax = CORRECT_CURR;
						CurrInitForw = true;
					} else goto cur_add;
				}
			} else if (CurrInitRev == false) {
				if (mov == SERVO_REVERS) {
					if (ServoCount < CNT_INIT_VAL) {
						CurrInitRev = true;
						RevCurrMax = CORRECT_CURR;
						if (RevCurrMax > ForwCurrMax) {
							CurrTmpVal = RevCurrMax;
						} else {
							CurrTmpVal = ForwCurrMax;
						}
						ErrCurrMax = CORRECT_CURR;
					} else {
cur_add:				tmp = SERVO_CURRENT;
						if (CurrTmpVal < tmp) CurrTmpVal = tmp;
					}
				}
			}
		}
	} else { // state == SERVO_READY
		if (timers_get_time_left(time) == 0) {
			time = timers_get_finish_time(SERVO_STEP_TIME);
			float32_t cnt_pos = (float32_t)ServoCount / (float32_t)FullCount;
			float32_t time_pos = (float32_t)CurrTime / (float32_t)FullTime;
			if (fabs(time_pos - cnt_pos) > CNT_TIME_ERR) CurrTime = (uint32_t)(cnt_pos *  (float32_t)FullTime);
			int32_t diff = TaskTime - CurrTime;
			if (diff > 0) {
				if (diff >= SERVO_STEP_TIME) {
					CurrTime += SERVO_STEP_TIME;
					set(FORWARD_MOV, ON);
					set(REVERS_MOV, OFF);
					err_time = timers_get_finish_time(SERVO_ON_ERR);
				} else if (state == SERVO_FORWARD) {
					time.del += diff;
				} else goto servo_stop;
			} else { // diff <= 0
				if (diff <= -SERVO_STEP_TIME) {
					CurrTime -= SERVO_STEP_TIME;
					set(FORWARD_MOV, OFF);
					set(REVERS_MOV, ON);
					err_time = timers_get_finish_time(SERVO_ON_ERR);
				} else if (state == SERVO_REVERS) {
					time.del += diff;
				} else {
servo_stop:
					if (state == SERVO_FORWARD) {
						if (CurrTime == FullTime) {
						}
					} else if (state == SERVO_REVERS) {
						if (CurrTime == 0) {
						}
					}
					set(FORWARD_MOV, OFF);
					set(REVERS_MOV, OFF);
				}
			}
		}
	}
}

servo_st servo_state(void)
{
	return state;
}

/* servo position in % */
float32_t servo_get_pos(void)
{
	return ((float32_t)CurrTime * 100.000) / (float32_t)FullTime;
}

/* set position of control */
void servo_set_out(float32_t pid_out)
{
	float32_t task = pid_out * (float32_t)FullTime;
	TaskTime = (uint32_t)(task * SERVO_MUL);
	if (TaskTime > FullTime) TaskTime = FullTime;
	if (TaskTime < 0) TaskTime = 0;
}
