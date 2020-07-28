#ifndef SERVO_H_
#define SERVO_H_
#include "arm_math.h"
#include <stdbool.h>

typedef enum {
	SERVO_READY = 0,
	SERVO_NOT_INIT,
	SERVO_STOP_ERR
} servo_st;

typedef enum {
	SERVO_STOP_ALL,
	SERVO_STOP_NULL,
	SERVO_STOP_FULL,
	SERVO_FORWARD,
	SERVO_REVERS
} mov_t;

#define NULL_MOV_MASK		0x02
#define FULL_MOV_MASK		0x04
#define PULSE_MASK			0x08
#define NULL_SENSOR			((in_data & NULL_MOV_MASK) != 0)
#define FULL_SENSOR			((in_data & FULL_MOV_MASK) != 0)
#define FORWARD_MOV			DO_OIL_HEATER
#define REVERS_MOV			DO_COOLANT_HEATER
#define SERVO_INIT_TIME		200000
#define STEP_TIME_MS		1000 // время шага управления приводом, мс
#define STEP_TIME_SEC		((float32_t)STEP_TIME_MS / 1000.0) // время шага управления приводом, сек
#define CNT_TIME_DEF		1.714f // время на 1 оборот привода, сек (Номинальная частота вращения 35 об/мин)
#define STEP_CNT_DEF		STEP_TIME_SEC / CNT_TIME_DEF // оборотов привода за 1 шаг управления
#define INIT_ERROR_TIME		(SERVO_INIT_TIME - 4000)
#define SERVO_MUL			0.0012f
#define CURR_ERR			2400
#define CURR_MIN			1500
#define CORRECT_CURR		(CurrTmpVal * 13) / 10
#define SERVO_TIME_ERR		1000
#define SERVO_ON_ERR		1000
#define CNT_INIT_VAL		10000
#define FCURR_TAU	 		0.05
#define CURR_FLT_OUT() 		(uint32_t)((float32_t)CurrFilterOut * (1.0 - FCURR_TAU) + (float32_t)st(AI_VALVE_POSITION) * FCURR_TAU)
#define SERVO_CURRENT 		CurrFilterOut
#define CURR_FORW_HIGH 		(SERVO_CURRENT > ForwCurrMax)
#define CURR_REVR_HIGH		(SERVO_CURRENT > RevCurrMax)
#define SERVO_CURR_HIGH 	(SERVO_CURRENT > ErrCurrMax)
#define SERVO_DEL_TIME		30000
#define CORRECT_FULL		15000
#define TRUE_STATE			80
#define DSENS_STATE			40
#define CNT_FULL_VAL		63
#define CNT_SAVE_VAL		4
#define CNT_TIME_ERR		0.05f
#define TIME_CNT_DEF		1.50 // время на 1 оборот, сек

void servo_init(void);
void servo_step(void);
servo_st servo_state(void);
float32_t servo_get_pos(void);
void servo_set_out(float32_t pid_out);

extern stime_t err_time;
extern bool servo_stop;
extern uint32_t ErrCurrMax, CurrFilterOut;

#endif /* SERVO_H_ */
