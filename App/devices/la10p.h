#ifndef LA10P_H_
#define LA10P_H_
#include "arm_math.h"
#include <stdbool.h>
#include "_signals.h"

typedef enum {
	LA10P_READY = 0,
	LA10P_NOT_INIT,
	LA10P_INIT_RUN,
	LA10P_STOP_ERR
} la10p_st;

/*
 * �������� ������: 	LA10P 24�
 * ������ ���������: 	10���
 * Working duty: 		25%
 * ����������� ���: 	4A
 * �������� ��������: 	48W
 * �������� ��������: 	20[mm/s]
 * ��������: 			450N = 45��
 * 150 �� 6-�������� ���
 * ������� ����������� -26 ~ + 65 ��������
 * IP Max IP 65
 */
#define ST_LENGT			150 // ����� �����
#define ST_MOVE				20  // �������� ����������� ����� [��/���]
#ifdef MODEL_OBJ
	#define STATE_SENS()	StateVal
	//#define STATE_SENS()	(float32_t)st(AI_T_OIL_OUT)
#else
	#define STATE_SENS()	(float32_t)st(AI_T_OIL_OUT)
#endif
#define FORWARD_MOV			DO_OIL_HEATER
#define REVERS_MOV			DO_COOLANT_HEATER
#define LA10P_FULL_TIME		(ST_LENGT * 1000 / ST_MOVE) // ms
#define LA10P_ERR_TIME		LA10P_FULL_TIME
#define LA10P_MIN_TIME		(LA10P_FULL_TIME - LA10P_FULL_TIME / 4)
#define LA10P_MUL			5.00f
#define STEP_TIME			100 // ����� ���� ���������� ��������, ��
#define RELE_TIME			200 // ����� ���� ���������� ��������, ��
#define SENS_MAX_VAL		3000.0f // ������������ �������� ��������, ��
#define SENS_ERR_VAL		3300.0f // ������������ �������� ��������, ��
#define SENS_MIN_VAL		0.0f // ����������� �������� ��������, ��
#define SENS_MIN_VAL		0.0f // ����������� �������� ��������, ��
#define SENS_I_MUL			0.6666666666666667f // ������� ��������
#define SENS_I_mV_A			185.0f // Sensitivity (mV/A)
#define CURR_SENS_VAL		(float32_t)st(AI_P_MANIFOLD)
#define CURR_SENS_mV		((CURR_SENS_VAL - CurrNull) / SENS_I_MUL)
#define CURR_SENS_A			fabs(CURR_SENS_mV / SENS_I_mV_A)
#define SENS_I_MAX			2.5f

void la10p_init(void);
void la10p_step(void);
float32_t la10p_get_pos(void);
void la10p_set_out(float32_t pid_out);
la10p_st la10p_state(void);

#endif /* LA10P_H_ */