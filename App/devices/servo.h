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
	SERVO_STOP_NULL,
	SERVO_STOP_FULL,
	SERVO_STOP_ALL,
	SERVO_FORWARD,
	SERVO_REVERS
} mov_t;

#define NULL_MOV_MASK		0x02
#define FULL_MOV_MASK		0x04
#define PULSE_MASK			0x08
#define FORWARD_MOV			DO_OIL_HEATER
#define REVERS_MOV			DO_COOLANT_HEATER
#define SERVO_INIT_TIME		200000
#define SERVO_STEP_TIME		1000
#define INIT_ERROR_TIME		(SERVO_INIT_TIME - 4000)
#define SERVO_MUL			0.001f
#define CURR_REV			2150
#define CURR_FORW			1900
#define CURR_ERR			2300
#define CURR_MIN			1500
#define CORRECT_CURR		(CurrTmpVal * 10) / 8
#define SERVO_TIME_ERR		1000
#define SERVO_ON_ERR		500
#define CNT_INIT_VAL		10
#define SERVO_CURRENT 		st(AI_VALVE_POSITION)
#define CURR_FORW_HIGH 		(SERVO_CURRENT > ForwCurrMax)
#define CURR_REVR_HIGH		(SERVO_CURRENT > RevCurrMax)
#define SERVO_CURR_HIGH 	(SERVO_CURRENT > ErrCurrMax)
#define SERVO_DEL_TIME		30000
#define CORRECT_FULL		15000
#define TRUE_STATE			80
#define CNT_SAVE_VAL		4
#define CNT_TIME_ERR		0.01f

void servo_init(void);
void servo_step(void);
servo_st servo_state(void);
float32_t servo_get_pos(void);
void servo_set_out(float32_t pid_out);

extern stime_t err_time;
extern bool servo_stop;
extern uint32_t ErrCurrMax;

#endif /* SERVO_H_ */
