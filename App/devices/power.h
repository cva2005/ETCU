/*
 * power.h
 *
 *  Created on: 24 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DEVICES_POWER_H_
#define APP_DEVICES_POWER_H_
#include "types.h"

#define	POWER_5V_CHAN	ADC_CHAN_5	//����� ��� ��� ��������� ���������� �������
#define	POWER_VBAT_CHAN	ADC_VBAT	//����� ��� ��� ��������� ���������� ���������
#define POWER_5V_R_UP	10000		//������� �������� �������� ����������: ��
#define POWER_5V_R_DW	10000		//������ �������� �������� ����������: ��

#define POWER_ACCURACY_K 1000		//��������� ��� ���������� �������� ���������� (��� ������, ��� ������ ����������)

//void power_init(void);			//�������������� ������� �������: ������ ���������� 5� � ������ ���������
uint32_t power_get_5v(void);	//���������� �������� ���������� 5�
uint32_t power_get_vbat(void);	//���������� �������� ���������� ���������

#endif /* APP_DEVICES_POWER_H_ */