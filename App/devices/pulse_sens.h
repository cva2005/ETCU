/*
 * pulse_sens.h
 *
 *  Created on: 28 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DEVICES_PULSE_SENS_H_
#define APP_DEVICES_PULSE_SENS_H_
#include "types.h"

#define PULSE_SENS_TYPE_CPT 5 //���������� ��� ����������� �������: ��� ����������� ������� "���-�": 5 (���-5) ��� 8 (���-8)

void pulse_sens_init(uint8_t sens, uint8_t chan);	//���������������� ���������� ������
int32_t pulse_sens_get_val(uint8_t sens);		//���������� �������� ����������� ������� � �������*1000/������
uint16_t pulse_sens_get_pl(uint8_t sens);			//���������� �������� ��������� �� ������� ���������
void pulse_sens_set_pl(uint8_t sens, uint16_t pl);  //������������� �������� ��������� �� ������� ���������


#if PULSE_SENS_TYPE_CPT==5
	#define PULSE_SENS_PL 950 //��� ������� ���-5: 950 ��������� �� ����
#elif PULSE_SENS_TYPE_CPT==8
	#define PULSE_SENS_PL 200 //��� ������� ���-8: 200 ��������� �� ����
#else
	#define PULSE_SENS_PL 1
#endif
#endif /* APP_DEVICES_PULSE_SENS_H_ */