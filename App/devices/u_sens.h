/*
 * u_sens.h
 *
 *  Created on: 3 ����. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DEVICES_U_SENS_H_
#define APP_DEVICES_U_SENS_H_
#include "types.h"

#define U_SENS1_R1 31000	//������� ������������� �������� ����������, ��
#define U_SENS1_R2 3000		//������ ������������� �������� ����������, ��
#define U_SENS1_ADC 13		//����� ��� �� ������� ���������� ������ ����������

int32_t u_sens_get_val(void);	//�������� ������ ������� ����������
int32_t u_sens_get_k(void);		//�������� �������� ������������ K
int32_t u_sens_get_b(void);		//�������� �������� ������������ B
uint8_t u_sens_get_d(void);		//�������� �������� ������������ D
void u_sens_set_k(int32_t val);	//���������� �������� ������������ K
void u_sens_set_b(int32_t val);	//���������� �������� ������������ B
void u_sens_set_d(uint8_t val);	//���������� �������� ������������ D

#endif /* APP_DEVICES_UI_SENS_H_ */