#ifndef PID_R_H
#define PID_R_H

#include <stdlib.h>
#include <arm_math.h>

#ifdef	__cplusplus
extern "C"
{
#endif

#define ST_SIZE		2u 		/* input array size */

typedef struct {
	float32_t Kp;			/* gain factor */
	float32_t Ti;			/* integration time */
	float32_t Tf;			/* derivative filter tau */
	float32_t Td;			/* derivative time */
	float32_t i[ST_SIZE];	/* old input states */
	float32_t u;			/* old output state */
	float32_t d;			/* old derivative state */
	float32_t Xd;			/* dead zone */
} pid_r_instance;

typedef enum {
	TUNE_COMPLETE = 0,
	TUNE_PROCEED  = 1,
	TUNE_STOP_ERR = 2
} tune_st;

typedef void (*pf_ctl) (float32_t y);

void pid_r_init (pid_r_instance *S);
void pid_tune_new (pid_r_instance *s, float32_t *pi, pf_ctl contrl);
tune_st pid_tune_step (void);
float32_t pid_tune_out (void);

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
    if (S->Ti > 0) I = (1.0 / S->Ti) * e[1];
    else I = 0;
    out = S->u + S->Kp * (P + I + Df);
    S->u = out;
	memcpy(S->i, e, ST_SIZE * sizeof(float32_t));
	return (out);
}

#ifdef	__cplusplus
}
#endif

#endif /* PID_R_H */