/*
 * pulse_in.h
 *
 *  Created on: 14 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DRIVERS_PULSE_IN_H_
#define APP_DRIVERS_PULSE_IN_H_
#include "stm32f4xx_hal.h"
/* ----���������� ��������� �������� � STM32CubeMX----
 *	1: BASE_MODE==1: ������ �������� �� ������� CLK (clk ����������� �����), ������������ ������� ���������� ��������� �� ����� �� ������ ������ �������� �������
 *		Slave Mode: External Clock Mode 1
 *		Trigger Source: TI1_ED ��� ETR1 ��� TI1FP1 (������� �� ����� ���� ��������� ���������� ����)
 *		���������: Disable
 *		Prescaler=1 (��� TI1_ED) Prescaler=0 (��� ETR1)
 *		Counter Period = 0xFFFF
 *		IRQ - ���������
 *	2: BASE_MODE==0: ������ �������� � ������ "input compare" - �� ��������� ������ ��������� �������� ������� � ��������������� (��������� ������� ����� ����������)
 *		Slave Mode: Reset Mode
 *		Trigger Source: TI1FP1 ��� TI2FP2 (������� �� ����� ���� ��������� ���������� ����)
 *		Clock Source: Internal Clock
 *		Channel1 ��� Channel2 (������� �� Trigger Source): Input Capture direct mode
 *		���������: Disable
 *		Prescaler = 0 - ������� �� ������������ ������� �� �����, ���  ������ Prescaler, ��� ������ �������� ���������� �������, �� ��� ���� ���� ����������� IRQ
 *		Counter Period = 0xFFFF
 *		Polarity: Rising Edge
 *		IC Selection: Direct
 *		Input Filter: 0
 *		���������: �� ���������
 */

#define SYS_CLOCK_APB1 84	//������� APB1:�������� ����� �� STM32CubeMX
#define SYS_CLOCK_APB2 168	//������� APB2:�������� ����� �� STM32CubeMX

#define PULSE_IN_NUMBER 3	//���������� ���������� ������

#if PULSE_IN_NUMBER>0		//��������� ����������� ����� 1
	#define PULSE_IN_1 htim3		//������ ���������� �� ���������� ���� 1
	#define PULSE_IN_1_BASE_MODE 1	//����� ������ 1 - ������� �������� � ������ ����� �� ����������� ����� (��������� ������������ �������� �������) 0 - ������ � ������ ��������� ������� (input compare)
#endif
#if PULSE_IN_NUMBER>1		//��������� ����������� ����� 2
	#define PULSE_IN_2 htim4		//������ ���������� �� ���������� ���� 2
	#define PULSE_IN_2_BASE_MODE 1	//����� ������ 1 (� ������� ��������) ��� 0 (��������� �������)
#endif
#if PULSE_IN_NUMBER>2		//��������� ����������� ����� 3
	#define PULSE_IN_3 htim9		//������ ���������� �� ���������� ���� 3
	#define PULSE_IN_3_BASE_MODE 0	//����� ������ 1 (� ������� ��������) ��� 0 (��������� �������)
#endif
#if PULSE_IN_NUMBER>3		//��������� ����������� ����� 4
	#define PULSE_IN_4 htim		//������ ���������� �� ���������� ���� 4
	#define PULSE_IN_4_BASE_MODE 0	//����� ������ 1 (� ������� ��������) ��� 0 (��������� �������)
#endif
#if PULSE_IN_NUMBER>4		//��������� ����������� ����� 5
	#define PULSE_IN_5 htim		//������ ���������� �� ���������� ���� 5
	#define PULSE_IN_5_BASE_MODE 0	//����� ������ 1 (� ������� ��������) ��� 0 (��������� �������)
#endif
#if PULSE_IN_NUMBER>5		//��������� ����������� ����� 6
	#define PULSE_IN_6 htim		//������ ���������� �� ���������� ���� 6
	#define PULSE_IN_6_BASE_MODE 0	//����� ������ 1 (� ������� ��������) ��� 0 (��������� �������)
#endif

#if PULSE_IN_1_BASE_MODE||PULSE_IN_2_BASE_MODE||PULSE_IN_3_BASE_MODE\
	||PULSE_IN_4_BASE_MODE||PULSE_IN_5_BASE_MODE||PULSE_IN_6_BASE_MODE\
	||PULSE_IN_7_BASE_MODE||PULSE_IN_8_BASE_MODE||PULSE_IN_9_BASE_MODE\
	||PULSE_IN_10_BASE_MODE||PULSE_IN_11_BASE_MODE||PULSE_IN_12_BASE_MODE\
	||PULSE_IN_13_BASE_MODE||PULSE_IN_14_BASE_MODE

#define PULSE_IN_BASE_NUMBER 6 //����� �������� �������
#endif

#define PULSE_MAX_OVERLOAD 65535 //������������ ���������� ����� ������������ ������� � ������ ��������� ������� (input compare)

void pulse_in_init(uint8_t channel);	//�������������� ���������� ����
uint32_t pulse_in_sec(uint8_t chan);	//���������� ���������� ��������� � ������� (��)
uint32_t pulse_in_min(uint8_t chan);	//���������� ���������� ��������� � ������� (������� ��������)
void pulse_in_capture_irq(TIM_HandleTypeDef *htim);//���������� ���������� ��� �������� �� ������ �� ���������� (input compare - � ������ ��������� ������� ����� ����������)
void pulse_in_base_irq(void);			//���������� ���������� �������� ������� (��� ������ ������ � ������ � ������� ��������)

//----------------------------------------------------------------------------------------------------------
#define TIM1_IRQ_PRE pulse_in_capture_irq(&htim1)
#define TIM2_IRQ_PRE pulse_in_capture_irq(&htim2)
#define TIM3_IRQ_PRE pulse_in_capture_irq(&htim3)
#define TIM4_IRQ_PRE pulse_in_capture_irq(&htim4)
#define TIM5_IRQ_PRE pulse_in_capture_irq(&htim5)
#define TIM6_IRQ_PRE pulse_in_capture_irq(&htim6)
#define TIM7_IRQ_PRE pulse_in_capture_irq(&htim7)
#define TIM8_IRQ_PRE pulse_in_capture_irq(&htim8)
#define TIM9_IRQ_PRE pulse_in_capture_irq(&htim9)
#define TIM10_IRQ_PRE pulse_in_capture_irq(&htim10)
#define TIM11_IRQ_PRE pulse_in_capture_irq(&htim11)
#define TIM12_IRQ_PRE pulse_in_capture_irq(&htim12)
#define TIM13_IRQ_PRE pulse_in_capture_irq(&htim13)
#define TIM14_IRQ_PRE pulse_in_capture_irq(&htim14)
#define TIM15_IRQ_PRE pulse_in_capture_irq(&htim15)
#define TIM16_IRQ_PRE pulse_in_capture_irq(&htim16)

#if PULSE_IN_1_BASE_MODE||PULSE_IN_2_BASE_MODE||PULSE_IN_3_BASE_MODE||PULSE_IN_4_BASE_MODE||PULSE_IN_5_BASE_MODE||PULSE_IN_6_BASE_MODE
	#if PULSE_IN_BASE_NUMBER==1
	#define PULSE_IN_BASE htim1
	#define TIM1_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM1
	#elif PULSE_IN_BASE_NUMBER==2
	#define PULSE_IN_BASE htim2
	#define TIM2_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM2
	#elif PULSE_IN_BASE_NUMBER==3
	#define PULSE_IN_BASE htim3
	#define TIM3_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM3
	#elif PULSE_IN_BASE_NUMBER==4
	#define PULSE_IN_BASE htim5
	#define TIM4_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM4
	#elif PULSE_IN_BASE_NUMBER==5
	#define PULSE_IN_BASE htim5
	#define TIM5_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM5
	#elif PULSE_IN_BASE_NUMBER==6
	#define PULSE_IN_BASE htim6
	#define TIM6_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM6
	#elif PULSE_IN_BASE_NUMBER==7
	#define PULSE_IN_BASE htim7
	#define TIM7_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM7
	#elif PULSE_IN_BASE_NUMBER==8
	#define PULSE_IN_BASE htim8
	#define TIM8_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM8
	#elif PULSE_IN_BASE_NUMBER==9
	#define PULSE_IN_BASE htim9
	#define TIM9_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM9
	#elif PULSE_IN_BASE_NUMBER==10
	#define PULSE_IN_BASE htim10
	#define TIM10_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM10
	#elif PULSE_IN_BASE_NUMBER==11
	#define PULSE_IN_BASE htim11
	#define TIM11_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM11
	#elif PULSE_IN_BASE_NUMBER==12
	#define PULSE_IN_BASE htim12
	#define TIM12_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM12
	#elif PULSE_IN_BASE_NUMBER==13
	#define PULSE_IN_BASE htim13
	#define TIM13_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM13
	#elif PULSE_IN_BASE_NUMBER==14
	#define PULSE_IN_BASE htim14
	#define TIM14_IRQ_POST 		pulse_in_base_irq() //���������� ���������� TIM14
	#endif
#endif

#endif /* APP_DRIVERS_PULSE_IN_H_ */