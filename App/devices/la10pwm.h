#ifndef LA10PWM_H_
#define LA10PWM_H_
#include "arm_math.h"
#include "_signals.h"

typedef enum {
	LA10PWM_READY,
	LA10PWM_NOT_INIT,
	LA10PWM_INIT_RUN,
	LA10PWM_STOP_ERR
} la10p_st;

typedef enum {
	CH_FORW,
	CH_REVR
} pwm_ch;

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
#if MODEL_NO_SERVO
	#define STATE_SENS()	StateVal
#else
	#define STATE_SENS()	(float32_t)st(AI_T_OIL_OUT)
#endif
#define LA10P_FULL_TIME		(ST_LENGT * 1000 / ST_MOVE + 2000) // ms
#define LA10P_ERR_TIME		LA10P_FULL_TIME
#define LA10P_MIN_TIME		(LA10P_FULL_TIME - LA10P_FULL_TIME / 4)
#define LA10P_MUL			6.50f
#define STEP_TIME			100 // ����� ���� ������ �������, ��
#define RELE_TIME			200 // ����� ���� ���������� ����, ��
#define SENS_MAX_VAL		2970.0f // ������������ �������� ��������, ��
#define SENS_ERR_VAL		3300.0f // ������������ �������� ��������, ��
#define SENS_MIN_VAL		840.0f // ����������� �������� ��������, ��
#define SENS_I_MUL			0.6666666666666667f // ������� ��������
#define SENS_I_mV_A			185.0f // Sensitivity (mV/A)
#define CURR_SENS_VAL		(float32_t)st(AI_P_MANIFOLD)
#define CURR_SENS_mV		((CURR_SENS_VAL - CurrNull) / SENS_I_MUL)
#define CURR_SENS_A			fabs(CURR_SENS_mV / SENS_I_mV_A)
#define SENS_I_MAX			4.2f
#define SENS_I_OFF			0.2f
#define TIME_CORR			4.0f

#define pwm_tim				htim1
#define TIM_INST			TIM1
#define GPIO_AF1_INST		GPIO_AF1_TIM1
#define TIM_CLK_ENABLE()	__TIM1_CLK_ENABLE();
#define PWM_R_PIN			GPIO_PIN_13 // reveres pin
#define PWM_F_PIN			GPIO_PIN_14 // forward pin
#define PWM_PORT			GPIOE
#define PWM_CH_FORW			TIM_CHANNEL_4
#define PWM_CH_REVR			TIM_CHANNEL_3
#define PWM_PRD_VAL			65535
#define FORW_DUTY			TIM_INST->CCR4
#define REVR_DUTY			TIM_INST->CCR3
#define DUTY_MIN			0.50f
#define DUTY_MAX			1.00f
#define ZONE_DEAD			0.01

void la10p_init(void);
void la10p_step(void);
float32_t la10p_get_pos(void);
void la10p_set_out(float32_t pid_out);
la10p_st la10p_state(void);

#endif /* LA10PWM_H_ */