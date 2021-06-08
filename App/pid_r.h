#ifndef PID_R_H
#define PID_R_H

#include <stdlib.h>
#include <arm_math.h>

#ifdef	__cplusplus
extern "C"
{
#endif

#define TD_ALPHA_MUL	0.250f
#define IF_TAU			10.00f // пост. времени входного фильтра
#define B1_CONST		0.380f // косвенные условия оптимальности
#define B2_CONST		1.000f
#define B3_CONST		3.700f
#define B_DIFF			0.500f // допуск на косвенные условия оптимальности
#define STEP_TIME		100    // врем€ шага опроса (дискретизации), мс
#define FULL_TIME		300000 // суммарное врем€ самонастройки, мс
#define END_TIME		15000  // врем€ поко€ при завершении самонастройки, мс

#define ST_SIZE			2u 		/* input array size */
#define PRD_STABLE		1
#define SPEED_KP_HI		0.02f
#define SPEED_KP_LO		0.0001f

/*
 * VARYING RESPONSE - TUNING CONSTANTS
 * --------------------------------------------------------------------------
 * | CONTROLLER 		| PROPORTIONAL	| INTEGRAL TIME | DERIVATIVE TIME 	|
 * | 			 		| 	BAND Pb		| CONSTANT TI	| 	CONSTANT TD	  	|
 * --------------------------------------------------------------------------
 * | UNDER DAMPED		|		1.0 P	| 		0.5 T 	| 		0.125 T		|
 * --------------------------------------------------------------------------
 * | CRITICALLY DAMPED	| 		1.5 P	| 		0.5 T 	| 		0.125 T		|
 * --------------------------------------------------------------------------
 * | OVER DAMPED		| 		2.0	P	| 		1.5 T 	| 		0.167 T		|
 * --------------------------------------------------------------------------
  */
#define CONST_KP	1.0f
#define CONST_TI	0.5f
#define CONST_TD	0.125f

typedef struct {
	float32_t Kp;			/* gain factor */
	float32_t Ti;			/* integration time */
	float32_t Tf;			/* derivative filter tau */
	float32_t Td;			/* derivative time */
	float32_t i[ST_SIZE];	/* old input states */
	float32_t u;			/* old output state */
	float32_t d;			/* old derivative state */
	float32_t Xd;			/* dead zone */
	float32_t Xi;			/* integral zone */
} pid_r_instance;

typedef enum {
	TUNE_NOT_USED 	= 0,
	TUNE_NEW_START	= 1,
	TUNE_PROCEED  	= 2,
	TUNE_COMPLETE 	= 3,
	TUNE_STOP_ERR 	= 4,
} tune_st;

typedef enum {
	ZIEGLER_NICHOLS	= 0,
	MPEI_ENERGY		= 1,
} tune_t;

typedef void (*pf_ctl) (float32_t y);

void pid_r_init (pid_r_instance *S);
void pid_tune_step (void);
float32_t pid_tune_out (void);
tune_st pid_tune_state (void);
void pid_tune_new (pid_r_instance *s, float32_t *pi,
		pf_ctl contrl, tune_t t_type);

static __INLINE float32_t pid_r (pid_r_instance *S, float32_t in) {
    float32_t e[ST_SIZE + 1], D, Df, P, I, out;
    if (fabs(in) < S->Xd) {
    	in = 0;
    } else {
    	if (in > 0) in -= S->Xd;
        else in += S->Xd;
    }
    e[0] = in;
	memcpy(&e[1], S->i, ST_SIZE * sizeof(float32_t));
#if ST_SIZE == 2
    D = S->Td * (in - 2.0 * e[1] + e[2]);
#elif ST_SIZE == 4
    D = (S->Td / 6.0) * (in + 2.0 * e[1] - 6.0 * e[2] + 2.0 * e[3] + e[4]);
#else
	#error "input array size not defined"
#endif
    if (S->Tf) Df = S->d * (1 - 1.0 / S->Tf) + D * (1.0 / S->Tf);
    else Df = D;
    S->d = Df;
    P = in - e[1];
    if (S->Ti > 0) {
    	if (S->Xi > 0) {
    		if (fabs(in) > S->Xi) I = 0;
    		else goto integer;
    	} else {
    		integer:
    		I = (1.0 / S->Ti) * e[1];
    	}
    } else I = 0;
    out = S->u + S->Kp * (P + I + Df);
    S->u = out;
	memcpy(S->i, e, ST_SIZE * sizeof(float32_t));
	return (out);
}

#ifdef	__cplusplus
}
#endif

#endif /* PID_R_H */
