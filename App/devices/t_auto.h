/*
 * t_auto.h
 *
 *  Created on: 28 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DEVICES_T_AUTO_H_
#define APP_DEVICES_T_AUTO_H_
#include "types.h"

#define T_AUTO_TABLE 1 //������� ������������ ������������� �� �����������: 0 - ����������� ��������, 1 - ������� 2 - ������������

int32_t t_auto_get_r(uint8_t chan);		//���������� �������� ������������� ������������ R2=U���R1/(U��-U���)
int32_t t_auto_convert_r_to_val(int32_t r); //������������� ������������� ������� ����������� � �C
int32_t t_auto_get_val(uint8_t chan);	//���������� �������� ������������

#define T_AUTO_R1 3000	//������������� R1 (�������) �������� ���������� ��� ������������

#endif /* APP_DEVICES_T_AUTO_H_ */