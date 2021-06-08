#include <string.h>
#include <stdbool.h>
#include "timers.h"
#include "pid_r.h"

void pid_r_init (pid_r_instance *S) {
	memset(S->i, 0, (ST_SIZE + 2) * sizeof(float32_t));
}

static pid_r_instance *pS;
static float32_t d; // d=An/5 выход релейного элемента
static float32_t Ufz; // выход фазосдвигающего фильтра
static float32_t Inf; // выход входного фильтра
static float32_t Kp;
static float32_t Ti;
static float32_t Td;
static float32_t Xi;
static float32_t Xd;
static tune_st State = TUNE_NOT_USED;
static stime_t Etime;
static stime_t Stime;
static pf_ctl pContrl;
static float32_t *pInp;
static float32_t Amax;
static float32_t Amin;
static float32_t inpRef;
static uint32_t T, T_1, T_0, i, p;
static float32_t Ufz; // выход фазосдвигающего фильтра
static float32_t dy; // зона вычисления экстремумов
static tune_t Ttype;

void pid_tune_new (pid_r_instance *s, float32_t *pi,
		pf_ctl contrl, tune_t t_type) {
	Ttype = t_type;
	pS = s;
	pInp = pi;
	Inf = *pi;
	pContrl = contrl;
	inpRef = s->u;
	Kp = s->Kp;
	Ti = s->Ti;
	Td = s->Td;
	Xi = s->Xi;
	Xd = s->Xd;
	s->Xi = 0;
	s->Xd = 0;
	Ufz = 0;
	if (t_type == ZIEGLER_NICHOLS) {
		s->Kp = SPEED_KP_HI;
		s->Ti = 0;
		s->Td = 0;
		dy = 40.0; // зона вычисления экстремумов
		d = 0;
	} else { // MPEI_ENERGY
		s->Kp = SPEED_KP_LO;
		d = s->d;
		dy = d / 50.0; // зона вычисления экстремумов
	}
	pid_r_init(s);
	State = TUNE_NEW_START;
	Amin = +inpRef + d;
	Amax = -inpRef - d;
	i = 0;
	p = 0; // счетчик периодов
}

void pid_tune_step (void) {
	if (State == TUNE_NEW_START) {
		Etime = timers_get_finish_time(FULL_TIME);
		Stime = timers_get_finish_time(END_TIME);
		State = TUNE_PROCEED;
	}
	if (State != TUNE_PROCEED) return;
	if (timers_get_time_left(Etime) == 0) {
		State = TUNE_STOP_ERR;
		pS->Kp = Kp;
		pS->Ti = Ti;
		pS->Td = Td;
tune_end:
    	pS->Xi = Xi;
    	pS->Xd = Xd;
    	pid_r_init(pS);
		return;
	}
	i++; // счетчик временных интервалов
	float32_t Tfz = pS->Ti * B1_CONST;
	Inf = (1 - 1 / IF_TAU) * Inf // входной фильтр
    		+ (1 / IF_TAU) * *pInp;
	float32_t inp;
	if (Ttype == ZIEGLER_NICHOLS) {
		if (timers_get_time_left(Stime) == 0) {
			State = TUNE_COMPLETE;
			pS->Kp *= CONST_KP;
			pS->Ti = T * CONST_TI;
			pS->Td = T * CONST_TD;
			goto tune_end;
		}
		inp = inpRef - Inf;
		Ufz = pid_r(pS, inp);
	} else { // MPEI_ENERGY
		if ((inpRef - Inf) >= 0) inp = d;
		else inp = -d;
		float32_t u = pid_r(pS, inp);
		if (u > 1) u = 1;
		else if (u < 0) u = 0;
		Ufz = (1 - 1 / (1 + Tfz)) * Ufz // фазосдвигающий фильтр
				+ (1 / (1 + Tfz)) * u;
	}
    pContrl(Ufz); // управл€ющее воздействие
    inp = Inf;
    if (inp - dy > Amax) {
        Amax = inp;
        T_0 = i;
    }
    if (inp + dy < Amax) {
        if (inp + dy < Amin) {
            Amin = inp;
            T_1 = i;
        }
    	if (Ttype == ZIEGLER_NICHOLS) {
            if (inp - dy > Amin) {
                float32_t A = (Amax - Amin) / 2;
                T = abs(T_1 - T_0) * 2;
            	p++; // счетчик периодов
            	if (p > PRD_STABLE) {
    				p = 0;
    				pS->Kp *= 0.75;
                	Amin = inpRef + inpRef / 2;
                	Amax = inpRef - inpRef / 2;
    				dy = A / 15.0; // зона вычисления экстремумов
    				Stime = timers_get_finish_time(END_TIME);
    			}
            }
    	} else { // MPEI_ENERGY
            if ((inp - dy > Amin) && (inp < inpRef + d)) {
                float32_t A = (Amax - Amin) / 2;
                T = abs(T_1 - T_0) * 2;
            	Amin = +inpRef + d;
            	Amax = -inpRef - d;
            	dy = A / 50.0; // зона вычисления экстремумов
				float32_t b2_k = A / d;
				float32_t b3_k = (float32_t)T / pS->Ti;
				// проверка косв. усл. оптимальности
				if ((fabs(b2_k - B2_CONST) < B_DIFF) &&
						(fabs(b3_k - B3_CONST) < B_DIFF)) {
					State = TUNE_COMPLETE;
					//if (T < 900) pS->Kp *= 0.35;
					//pS->Ti *= 0.35;
					//pS->Td *= 2.0;
					//if (T < 300) pS->Kp *= 0.1;
					goto tune_end;
				} else { // вычисление параметров ѕ»ƒ следующей итерации
					pS->Kp *= B2_CONST / b2_k;
					pS->Ti *= b3_k / B3_CONST;
					pS->Td = TD_ALPHA_MUL * pS->Ti;
				}
            }
    	}
    }
}

tune_st pid_tune_state (void) {
	return State;
}

float32_t pid_tune_out (void) {
	return pS->u;
}

