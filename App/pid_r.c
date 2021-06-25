#include <string.h>
#include <stdbool.h>
#include "timers.h"
#include "pid_r.h"

static pid_r_instance *pS;
static float32_t Ufz; // выход фазосдвигающего фильтра
static float32_t Inf; // выход входного фильтра
static float32_t Kp;
static float32_t Ti;
static float32_t Td;
static float32_t Xi;
static float32_t Xd;
static tune_st State = TUNE_NOT_USED;
static stime_t Etime;
static stime_t BrTime;
static pf_ctl pContrl;
static float32_t *pInp;
static float32_t Amax;
static float32_t Amin;
static float32_t inpRef, Kmul;
static uint32_t T, T_1, T_0, i;
static bool compl;

void pid_r_init (pid_r_instance *S) {
	memset(S->i, 0, (ST_SIZE + 2) * sizeof(float32_t));
}

void pid_tune_new (pid_r_instance *s, float32_t *pi, pf_ctl contrl) {
	pS = s;
	pInp = pi;
	Inf = *pi;
	pContrl = contrl;
	inpRef = s->u;
	Kmul = s->Tf;
	Kp = s->Kp;
	Ti = s->Ti;
	Td = s->Td;
	Xi = s->Xi;
	Xd = s->Xd;
	memset(&s->Ti, 0, 5 * sizeof(float32_t));
	pid_r_init(s);
	State = TUNE_NEW_START;
	Amin = +inpRef;
	Amax = -inpRef;
	Ufz = 0;
	i = 0;
	compl = false;
}

void pid_tune_step (void) {
	if (State == TUNE_NEW_START) {
		Etime = timers_get_finish_time(FULL_TIME);
		BrTime = timers_get_finish_time(T_BREAK);
		State = TUNE_PROCEED;
	}
	if (State != TUNE_PROCEED) return;
	if (timers_get_time_left(Etime) == 0) {
tune_err:
		State = TUNE_STOP_ERR;
		pS->Kp = Kp;
		pS->Ti = Ti;
		pS->Td = Td;
tune_end:
		pContrl(0.0);
    	pS->Xi = Xi;
    	pS->Xd = Xd;
    	pid_r_init(pS);
		return;
	}
	i++; // счетчик временных интервалов
	float32_t Tfz = pS->Ti * B1_CONST;
	Inf = (1 - 1 / IF_TAU) * Inf // входной фильтр
    		+ (1 / IF_TAU) * *pInp;
	if (timers_get_time_left(BrTime) == 0) {
		if (compl == true) {
			State = TUNE_COMPLETE;
			if (Kmul > 0) pS->Kp *= Kmul;
			pS->Ti = T * CONST_TI;
			pS->Td = T * CONST_TD;
			goto tune_end;
		} else {
			goto tune_err;
		}
	}
	float32_t inp = inpRef - Inf;
	float32_t u = pid_r(pS, inp);
    pContrl(u); // управл€ющее воздействие
    inp = Inf;
    if (inp < Amin - DY) {
    	Amin = inp;
        T_0 = i;
    }
    if (inp > Amin + DY) {
        if (inp > Amax + DY) {
        	Amax = inp;
            T_1 = i;
        }
        if (inp < Amax - DY) {
            float32_t A = (Amax - Amin) / 2;
            T = (T_1 - T_0) * 2;
    		compl = true;
			pS->Kp *= 0.8;
			Amin = inpRef + inpRef / 2;
        	Amax = inpRef - inpRef / 2;
        	uint32_t prd_t = timers_get_time_left(BrTime);
			BrTime = timers_get_finish_time(T_BREAK - prd_t);
			pid_r_init (pS);
			pS->u = u;
        }
    }
}

tune_st pid_tune_state (void) {
	return State;
}

float32_t pid_tune_out (void) {
	return pS->u;
}

