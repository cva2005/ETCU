#include <string.h>
#include <stdbool.h>
#include "timers.h"
#include "pid_r.h"

void pid_r_init (pid_r_instance *S) {
	memset(S->i, 0, (ST_SIZE + 3) * sizeof(float32_t));
}

#define K_GAIN_INIT			0.001f
#define TI_INIT_VAL			150.0f
#define TD_ALPHA_MUL		0.250f
#define IF_TAU				10.00f // ����. ������� �������� �������
#define DF_TAU				30.00f // ����. ������� ����. �����
#define B1_CONST			0.380f // ��������� ������� �������������
#define B2_CONST			1.000f
#define B3_CONST			3.700f
#define B_DIFF				0.500f // ������ �� ��������� ������� �������������
#define STEP_TIME			100    // ����� ���� ������ (�������������), ��
#define FULL_TIME			300000 // ��������� ����� �������������, ��

static pid_r_instance *pS;
static float32_t d; // d=An/5 ����� ��������� ��������
static float32_t Ufz; // ����� ��������������� �������
static float32_t Inf; // ����� �������� �������
static tune_st State = TUNE_COMPLETE;
static stime_t Etime;
static pf_ctl pContrl;
static float32_t *pInp;
static float32_t Amax;
static float32_t Amin;
static float32_t inpRef;
static uint32_t T_1, T_0, i;

void pid_tune_new (pid_r_instance *s, float32_t *pi, pf_ctl contrl) {
	pS = s;
	pInp = pi;
	Inf = *pi;
	pContrl = contrl;
	d = s->d;
	inpRef = s->u;
	Ufz = 0;
	pid_r_init(s);
	//s->Kp = K_GAIN_INIT;
	//s->Ti = TI_INIT_VAL;
	//s->Td = TI_INIT_VAL * TD_ALPHA_MUL;
	//s->Tf = DF_TAU;
	State = TUNE_PROCEED;
	Etime = timers_get_finish_time(FULL_TIME);
	Amin = inpRef + d;
	Amax = inpRef - d;
	i = 0;
}

tune_st pid_tune_step (void) {
	if (State != TUNE_PROCEED) goto ret_state;
	if (timers_get_time_left(Etime) == 0) {
		State = TUNE_STOP_ERR;
		goto ret_state;
	}
	i++; // ������� ��������� ����������
	float32_t Tfz = pS->Ti * B1_CONST;
	Inf = (1 - 1 / IF_TAU) * Inf // ������� ������
    		+ (1 / IF_TAU) * *pInp;
	float32_t inp;
    if ((inpRef - Inf) >= 0) inp = d;
    else inp = -d;
	float32_t u = pid_r(pS, inp/* * 0.1f*/);
    if (u > 1) u = 1;
    else if (u < 0) u = 0;
    Ufz = (1 - 1 / (1 + Tfz)) * Ufz // �������������� ������
    		+ (1 / (1 + Tfz)) * u;
    pContrl(Ufz); // ����������� �����������
    inp = Inf;
    float32_t dy = d / 100;	// ���� ���������� �����������
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
            // �������� ����. ���. �������������
            if ((fabs(b2_k - B2_CONST) < B_DIFF) &&
            		(fabs(b3_k - B3_CONST) < B_DIFF)) {
            	State = TUNE_COMPLETE;
            } else { // ���������� ���������� ��� ��������� ��������
            	pS->Kp *= B2_CONST / b2_k;
            	pS->Ti *= b3_k / B3_CONST;
            	pS->Td = TD_ALPHA_MUL * pS->Ti;
            	Amin = inpRef + d;
            	Amax = inpRef - d;
            }
        }
    }
	ret_state:
	return State;
}

float32_t pid_tune_out (void) {
	return pS->u;
}
