#ifndef SERVO_H_
#define SERVO_H_
#include "arm_math.h"

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
#define FORWARD_MOV			DO_OIL_HEATER
#define REVERS_MOV			DO_COOLANT_HEATER
#define SERVO_INIT_TIME		60000
#define SERVO_STEP_TIME		500
#define SERVO_MUL			0.001f

void servo_init(void);
void servo_step(void);
servo_st servo_state(void);
int32_t servo_get_pos(void);
void servo_set_out(float32_t pid_out);
#ifdef SERVO_MODEL
float32_t servo_pos_ready(void);
#endif

#endif /* SERVO_H_ */
