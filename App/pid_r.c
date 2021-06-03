#include <string.h>
#include <stdbool.h>
#include "timers.h"
#include "pid_r.h"

void pid_r_init (pid_r_instance *S) {
	memset(S->i, 0, (ST_SIZE + 2) * sizeof(float32_t));
}

#define K_GAIN_INIT			0.001f
#define TI_INIT_VAL			150.0f
#define TD_ALPHA_MUL		0.250f
#define IF_TAU				10.00f // пост. времени входного фильтра
#define DF_TAU				30.00f // пост. времени дифф. звена
#define B1_CONST			0.380f // косвенные условия оптимальности
#define B2_CONST			1.000f
#define B3_CONST			3.700f
#define B_DIFF				0.500f // допуск на косвенные условия оптимальности
#define STEP_TIME			100    // врем€ шага опроса (дискретизации), мс
#define FULL_TIME			300000 // суммарное врем€ самонастройки, мс

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
static pf_ctl pContrl;
static float32_t *pInp;
static float32_t Amax;
static float32_t Amin;
static float32_t inpRef;
static uint32_t T_1, T_0, i;
static float32_t Ufz; // выход фазосдвигающего фильтра

void pid_tune_new (pid_r_instance *s, float32_t *pi, pf_ctl contrl) {
	pS = s;
	pInp = pi;
	Inf = *pi;
	pContrl = contrl;
	d = s->d;
	inpRef = s->u;
	Kp = s->Xi;
	Ti = s->Xi;
	Td = s->Xi;
	Xi = s->Xi;
	Xd = s->Xd;
	s->Xi = 0;
	s->Xd = 0;
	Ufz = 0;
	pid_r_init(s);
	State = TUNE_PROCEED;
	Etime = timers_get_finish_time(FULL_TIME);
	Amin = inpRef + d;
	Amax = inpRef - d;
	i = 0;
}

void pid_tune_step (void) {
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
    if ((inpRef - Inf) >= 0) inp = d;
    else inp = -d;
	float32_t u = pid_r(pS, inp * 0.1f);
    if (u > 1) u = 1;
    else if (u < 0) u = 0;
    Ufz = (1 - 1 / (1 + Tfz)) * Ufz // фазосдвигающий фильтр
    		+ (1 / (1 + Tfz)) * u;
    pContrl(Ufz); // управл€ющее воздействие
    inp = Inf;
    float32_t dy = d / 100;	// зона вычисления экстремумов
    if (inp - dy > Amax) {
        Amax = inp;
        T_0 = i;
    }
    if (inp + dy < Amax) {
        if (inp + dy < Amin) {
            Amin = inp;
            T_1 = i;
        }
        if ((inp - dy > Amin) && (inp < d)) {
            float32_t A = (Amax - Amin) / 2;
            uint32_t T = abs(T_1 - T_0) * 2;
            float32_t b2_k = A / d;
            float32_t b3_k = (float32_t)T / pS->Ti;
            // проверка косв. усл. оптимальности
            if ((fabs(b2_k - B2_CONST) < B_DIFF) &&
            		(fabs(b3_k - B3_CONST) < B_DIFF)) {
            	State = TUNE_COMPLETE;
            	pS->Kp *= 0.04;
            	pS->Td = pS->Ti;
            	pS->Ti *= 10.0;
            	goto tune_end;
            	//if (T < 900) pS->Kp *= 0.3;
            	//if (T < 300) pS->Kp *= 0.1;
            } else { // вычисление параметров ѕ»ƒ следующей итерации
            	pS->Kp *= B2_CONST / b2_k;
            	pS->Ti *= b3_k / B3_CONST;
            	pS->Td = TD_ALPHA_MUL * pS->Ti;
            	Amin = inpRef + d;
            	Amax = inpRef - d;
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

