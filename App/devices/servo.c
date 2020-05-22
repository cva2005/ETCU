#include "servo.h"
#include "timers.h"
#include <stdbool.h>
#include <stdlib.h>
#include "arm_math.h"
#include "_control.h"
#include "bcu.h"

uint32_t FullTime, CurrTime, TaskTime;
static stime_t time;
static servo_st state;

mov_t mov_state(void)
{
#ifndef SERVO_MODEL
	uint8_t in_data = bcu_get_in();
	if (in_data & NULL_MOV_MASK) {
#else
		if (CurrTime == 0) {
#endif
		set(REVERS_MOV, OFF);
		return SERVO_STOP_NULL;
#ifndef SERVO_MODEL
	} else if (in_data & FULL_MOV_MASK) {
#else
	} else if (CurrTime >= FullTime) {
#endif
		set(FORWARD_MOV, OFF);
		return SERVO_STOP_FULL;
	} else if (st(FORWARD_MOV)) {
		return SERVO_FORWARD;
	} else if (st(REVERS_MOV)) {
		return SERVO_REVERS;
	}
	return SERVO_STOP_ALL;
}

void servo_init(void)
{
#ifndef SERVO_MODEL
	state = SERVO_NOT_INIT;
	time = timers_get_finish_time(SERVO_INIT_TIME);
	set(FORWARD_MOV, ON);
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
	if (bcu_err_link()) {
error_state:
		state = SERVO_STOP_ERR;
		set(FORWARD_MOV, OFF);
		set(REVERS_MOV, OFF);
		return;
	}
	mov_t mov = mov_state();
	if (state == SERVO_NOT_INIT) {
		if (timers_get_time_left(time) == 0) goto error_state;
		if (mov == SERVO_STOP_FULL) {
			time = timers_get_finish_time(SERVO_INIT_TIME);
			set(FORWARD_MOV, OFF);
			set(REVERS_MOV, ON);
		} else if (mov == SERVO_STOP_NULL) {
			FullTime = SERVO_INIT_TIME - timers_get_time_left(time);
			CurrTime = 0;
			set(REVERS_MOV, OFF);
			state = SERVO_READY;
			time = timers_get_finish_time(SERVO_STEP_TIME);
		}
	} else { // state == SERVO_READY
		if (timers_get_time_left(time) == 0) {
			time = timers_get_finish_time(SERVO_STEP_TIME);
			int32_t diff = TaskTime - CurrTime;
			if (diff > 0) {
				if (diff >= SERVO_STEP_TIME) {
					CurrTime += SERVO_STEP_TIME;
					set(FORWARD_MOV, ON);
					set(REVERS_MOV, OFF);
				} else goto servo_stop;
			} else { // diff <= 0
				if (diff <= -SERVO_STEP_TIME) {
					CurrTime -= SERVO_STEP_TIME;
					set(FORWARD_MOV, OFF);
					set(REVERS_MOV, ON);
				} else {
servo_stop:
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
int32_t servo_get_pos(void)
{
	return (CurrTime * 100000) / FullTime;
}

#ifdef SERVO_MODEL
float32_t servo_pos_ready(void)
{
	float32_t ready = (float32_t)(TaskTime - CurrTime);
	ready /= (float32_t)(FullTime);
	return (1.0 - ready);
}
#endif

/* set position of control */
void servo_set_out(float32_t pid_out)
{
	float32_t task = pid_out * (float32_t)FullTime;
	TaskTime = (uint32_t)(task * SERVO_MUL);
	if (TaskTime > FullTime) TaskTime = FullTime;
	if (TaskTime < 0) TaskTime = 0;
}
