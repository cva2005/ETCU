/*
 * i_sens.h
 *
 *  Created on: 3 ����. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DEVICES_I_SENS_H_
#define APP_DEVICES_I_SENS_H_
#include "types.h"

//#define I_HARD_CALC //����������, ���� ��� ����������� ��������� ���������� �� ��� ��� ����������� ���� �����

#ifndef I_HARD_CALC
#define I_SENS1_SHUNT 100000	//�������� ������������ ���� �� ����� � ��
#define I_SENS1_ADC_U 3289		//�������� ����������� � �� �� ��� ��� ����������� ����
#define I_SENS1_ADC 12			//����� ��� �� ������� ���������� ������ ����

#define I_SENS2_SHUNT 100000	//�������� ������������ ���� �� ����� � ��
#define I_SENS2_ADC_U 3214		//�������� ����������� � �� �� ��� ��� ����������� ����
#define I_SENS2_ADC 11			//����� ��� �� ������� ���������� ������ ����

#else
#define I_SENS1_SHUNT_I 100000	//�������� ���� �� ����� � ��
#define I_SENS1_SHUNT_U 75		//�������� ���������� �� ����� ��� ��������� ���� ��
#define I_SENS1_R_IN 3500		//������� ������������� ���������, ��
#define I_SENS1_R_FB 150000		//������������� � �������� �����, ��
#define I_SENS1_ADC 12			//����� ��� �� ������� ���������� ������ ����
//#define I_SENS1_INVERT			//���� ��������, �� ��������� �������������

#define I_SENS2_SHUNT_I 100000	//�������� ���� �� ����� � ��
#define I_SENS2_SHUNT_U 75		//�������� ���������� �� ����� ��� ��������� ���� ��
#define I_SENS2_R_IN 3500		//������� ������������� ���������, ��
#define I_SENS2_R_FB 150000		//������������� � �������� �����, ��
#define I_SENS2_ADC 11			//����� ��� �� ������� ���������� ������ ����
#define I_SENS2_INVERT			//���� ��������, �� ��������� �������������
#endif

int32_t i_sens_get_val(uint8_t chan);	//�������� ������ ������� ����������
int32_t i_sens_get_k(uint8_t chan);		//�������� �������� ������������ K
int32_t i_sens_get_b(uint8_t chan);		//�������� �������� ������������ B
uint8_t i_sens_get_d(uint8_t chan);		//�������� �������� ������������ D
void i_sens_set_k(uint8_t chan, int32_t val);	//���������� �������� ������������ K
void i_sens_set_b(uint8_t chan, int32_t val);	//���������� �������� ������������ B
void i_sens_set_d(uint8_t chan, uint8_t val);	//���������� �������� ������������ D
static uint8_t i_sens_get_element(uint8_t adc); //���������� ����� �������� � �������, ��� ������� ������ ���


#endif /* APP_DEVICES_I_SENS_H_ */