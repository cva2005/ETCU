#include <stdlib.h>
#include "timers.h"
#include "arm_math.h"
#include "_control.h"
#include "bcu.h"
#include "servo.h"

uint32_t ForwCurrMax, RevCurrMax, ErrCurrMax, CurrFilterOut, CurrTmpVal;
static stime_t time;
static servo_st state;
stime_t err_time; // искл. токов включения СП
bool servo_stop = false;
bool curr_null = false;
bool Pulse = false;
bool CurrInitForw = false;
bool CurrInitRev = false;
uint32_t ServoCount = 0;
float32_t TaskCount = 0, FloatCount = 0;
int32_t FullCount;
uint32_t StateCnt = 0;
static uint32_t NullCnt = 0;
static uint32_t FullCnt = 0;
bool NullState = true;
bool FullState = false;
uint32_t CurrDelCnt = 0;

bool null_sens(void)
{
	uint8_t in_data = bcu_get_in();
	bool in_st = (NULL_SENSOR);
	if (in_st != NullState) {
			if (NullCnt++ > DSENS_STATE) {
				NullState = in_st;
				NullCnt = 0;
			}
		} else NullCnt = 0;
	return NullState;
}

bool full_sens(void)
{
	uint8_t in_data = bcu_get_in();
	bool in_st = (FULL_SENSOR);
	if (in_st != FullState) {
			if (FullCnt++ > DSENS_STATE) {
				FullState = in_st;
				FullCnt = 0;
			}
		} else FullCnt = 0;
	return FullState;
}

void pulse_check(mov_t mov)
{
	uint8_t in_data = bcu_get_in();
	bool pulse_in = in_data & PULSE_MASK;
	if (pulse_in != Pulse) {
		if (StateCnt++ >= TRUE_STATE) {
			StateCnt = 0;
			Pulse = pulse_in;
			if (pulse_in)  {
				if (mov == SERVO_FORWARD) ServoCount++;
				else ServoCount--;
				FloatCount = (float32_t)ServoCount;
			}
		}
	} else StateCnt = 0;
}

static mov_t mov_state(void)
{
	if (null_sens()) {
		if (st(FORWARD_MOV)) { // положение "0"
			return SERVO_FORWARD;
		}
		if (state != SERVO_NOT_INIT) {
			ServoCount = 0;
			FloatCount = 0;
		}
stop_null:
		set(REVERS_MOV, OFF);
		servo_stop = false;
		curr_null = true;
		return SERVO_STOP_NULL;
	} else if (full_sens()) {
		if (st(REVERS_MOV)) { // положение "MAX"
			return SERVO_REVERS;
		}
		ServoCount = FullCount;
		FloatCount = (float32_t)FullCount;;
stop_full:
		set(FORWARD_MOV, OFF);
		return SERVO_STOP_FULL;
	} else if (st(FORWARD_MOV)) {
		pulse_check(SERVO_FORWARD);
		curr_null = false;
		/*if (state != SERVO_NOT_INIT) {
			if (CURR_FORW_HIGH) { // превышение тока открытие
				ServoCount = FullCount + (CNT_SAVE_VAL * 2);
				goto stop_full; // коррекция "FULL" по току
			}
		}*/
		return SERVO_FORWARD;
	} else if (st(REVERS_MOV)) {
		pulse_check(SERVO_REVERS);
		/*if (state != SERVO_NOT_INIT) {
			if (servo_stop && !curr_null) { // переход в режим "стоп"
				if (CURR_REVR_HIGH) { // превышение тока закрытие
					ServoCount = -CNT_SAVE_VAL;
					goto stop_null; // коррекция "0" по току
				}
			}
		}*/
		return SERVO_REVERS;
	}
	return SERVO_STOP_ALL;
}

void servo_init(void)
{
#if MODEL_NO_SERVO
	state = SERVO_READY;
	FullCount = MODEL_NO_SERVO;
	ForwCurrMax = CURR_DEBUG;
	RevCurrMax = CURR_DEBUG;
	ErrCurrMax = CURR_DEBUG * 2;
#else
	FullCount = CNT_FULL_VAL;
	state = SERVO_NOT_INIT;
	ForwCurrMax = RevCurrMax = ErrCurrMax = CURR_ERR;
	CurrFilterOut = 0;
	time = timers_get_finish_time(SERVO_INIT_TIME);
	set(FORWARD_MOV, ON);
	err_time = timers_get_finish_time(SERVO_ON_ERR);
#endif
}

void servo_step(void)
{
	mov_t mov = mov_state();
	if (timers_get_time_left(err_time) == 0) {
		CurrFilterOut = CURR_FLT_OUT();
	}
	if (state == SERVO_NOT_INIT) {
		uint32_t tmp = timers_get_time_left(time);
		if (!ServoCount && (tmp < INIT_ERROR_TIME)) {
			tmp = 0;
		}
		if (tmp == 0) {
			state = SERVO_STOP_ERR;
			set(FORWARD_MOV, OFF);
			set(REVERS_MOV, OFF);
			return;
		}
		if (mov == SERVO_STOP_FULL) {
stop_full:
			time = timers_get_finish_time(SERVO_INIT_TIME);
			ServoCount = 0;
			set(FORWARD_MOV, OFF);
			set(REVERS_MOV, ON);
			err_time = timers_get_finish_time(SERVO_ON_ERR);
		} else if (mov == SERVO_STOP_NULL) {
stop_null:
			FullCount = - ServoCount - (CNT_SAVE_VAL * 2);
			ServoCount = -CNT_SAVE_VAL;
			state = SERVO_READY;
			time = timers_get_finish_time( STEP_TIME_MS);
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
					if (CurrDelCnt++ > CNT_INIT_VAL) {
						ForwCurrMax = CORRECT_CURR;
						CurrDelCnt = 0;
						CurrInitForw = true;
					} else goto cur_add;
				}
			} else if (CurrInitRev == false) {
				if (mov == SERVO_REVERS) {
					if (CurrDelCnt++ > CNT_INIT_VAL) {
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
			time = timers_get_finish_time(STEP_TIME_MS);
			float32_t diff = TaskCount - FloatCount;
			if (diff > 0) {
				if (diff > STEP_CNT_DEF) {
					FloatCount += STEP_CNT_DEF;
					if (mov != SERVO_FORWARD) {
						set(FORWARD_MOV, ON);
						set(REVERS_MOV, OFF);
						err_time = timers_get_finish_time(SERVO_ON_ERR);
					}
				} else { // 0 < diff < STEP_CNT_DEF
					if (mov == SERVO_FORWARD) {
						FloatCount += diff;
						uint32_t del = STEP_TIME_MS * diff;
						time = timers_get_finish_time(del);
					} else goto servo_stop;
				}
				if (FloatCount >= (float32_t)FullCount) {
					FloatCount = (float32_t)FullCount;
					ServoCount = FullCount;
					goto servo_stop;
				}
#if MODEL_NO_SERVO
				ServoCount = (uint32_t)(FloatCount + 0.5f);
#endif
			} else if (diff < 0) {
				if (diff < -STEP_CNT_DEF) {
					FloatCount -= STEP_CNT_DEF;
					if (mov != SERVO_REVERS) {
						set(FORWARD_MOV, OFF);
						set(REVERS_MOV, ON);
						err_time = timers_get_finish_time(SERVO_ON_ERR);
					}
				} else { // -STEP_CNT_DEF < diff < 0
					if (mov == SERVO_REVERS) {
						FloatCount += diff;
						uint32_t del = STEP_TIME_MS * -diff;
						time = timers_get_finish_time(del);
					} else goto servo_stop;
				}
				if (FloatCount <= 0) {
					FloatCount = 0;
					ServoCount = 0;
					goto servo_stop;
				}
#if MODEL_NO_SERVO
				ServoCount = (uint32_t)(FloatCount + 0.5f);
#endif
			} else { // diff == 0
servo_stop:
				set(FORWARD_MOV, OFF);
				set(REVERS_MOV, OFF);
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
	return (FloatCount * 100.000) / (float32_t)FullCount;
}

/* servo multiply K */
float32_t servoKd(void)
{
	return (float32_t)FullCount / (float32_t)CNT_FULL_VAL;
}

/* set position of control */
void servo_set_out(float32_t pid_out)
{
	float32_t task = pid_out * (float32_t)FullCount;
	TaskCount = task * SERVO_MUL;
	if (TaskCount > (float32_t)FullCount) TaskCount = FullCount;
	if (TaskCount < 0) TaskCount = 0;
}
