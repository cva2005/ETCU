#ifndef PID_R_H
#define PID_R_H

#include <stdlib.h>
#include <arm_math.h>

#ifdef	__cplusplus
extern "C"
{
#endif

typedef struct {
	float32_t Xi;	/* integration zone */
	float32_t Kp;	/* gain factor */
	float32_t Ti;	/* integration time */
	float32_t Tf;	/* derivative filter tau */
	float32_t Td;	/* derivative time */
	float32_t i[4];	/* old input states */
	float32_t u;	/* old output state */
	float32_t d;	/* old derivative state */
	float32_t Xd;	/* dead zone */
} pid_r_instance;

#define NO_INTEGRAL_ZONE	-1.0f
#define NO_DEAD_ZONE		-1.0f

void pid_r_init (pid_r_instance *S);

static __INLINE float32_t pid_r (pid_r_instance *S, float32_t in) {
    float32_t e[5], D, Df, P, I, out;
    if (fabs(in) < S->Xd) {
    		in = 0;
    } else {
    	if (in > 0) in -= S->Xd;
        else in += S->Xd;
    }
    e[0] = in;
	memcpy(&e[1], S->i, 4u * sizeof(float32_t));
    D = (S->Td / 6.0) * (e[0] + 2.0 * e[1] - 6.0 * e[2] + 2.0 * e[3] + e[4]);
    Df = S->d * (1 - S->Tf) + D * (S->Tf);
    S->d = Df;
    //D(i)=Td*(e(i)-2*e(i-1)+e(i-2));
    P = e[0] - e[1];
    if (S->Ti > 0) {
        if (fabs(e[0]) > S->Xi) I = 0;
        else I = (1.0 / S->Ti) * e[1];
    } else {
        I = 0;
    }
    out = S->u + S->Kp * (P + I + D);
    S->u = out;
	memcpy(S->i, e, 4u * sizeof(float32_t));
	return (out);
}

#ifdef	__cplusplus
}
#endif

#endif /* PID_R_H */
