/*
 * bcu.h
 *
 *  Created on: 10 ���. 2016 �.
 *      Author: ������ �.�.
 */
/* ������ ���������� �������������
 * ������ ��������: DMP 330L-4003-1-100-500-01R-��
 * ������ �����������: 421.3828
 */
#ifndef APP_DEVICES_BCU_H_
#define APP_DEVICES_BCU_H_
#include "types.h"

#define BCU_CONNECT_TIME 200	//������������ ���������� ���� �������� ������� �� BCU, ��� ���������� ��������� ����� � BCU ����������
#define BCU_DATA_TX_TIME 50		//������� ����� ������������� �������� � BCU
#define BCU_DATA_TX_INIT 50		//������� ����� ������������� �������� �������������

#define	BCU_MAX_PRESSURE 400000  //������������ �������� ������� DMP330L, ����
#define BCU_I_MIN_PRESSURE 4000	 //����������� �������� ���� � ��� ��� 0 ������� �������� DMP330L
#define BCU_I_MAX_PRESSURE 20000 //����������� �������� ���� � ��� ��� ��������� ������� �������� DMP330L
#define BCU_FAN_MASK		0x01 // RELE 1 ���������� ���������� �����
#define BCU_PUMP_MASK		0x02 // RELE 2 ���. ������� ������������
#define BCU_LIGHT_MASK		0x04 // RELE 3 �������� ���������
#define BCU_SOUND_MASK		0x08 // RELE 4 �������� ���������
#define BCU_I_MAX_PRESSURE 20000	//����������� �������� ���� � ��� ��� ��������� ������� �������� DMP330L
#define BCU_I_MAX_PRESSURE 20000	//����������� �������� ���� � ��� ��� ��������� ������� �������� DMP330L

#pragma pack(1)
typedef union
	{
	struct
		{
		uint16_t pwm1;		//���1
		uint16_t pwm2;		//���2
		uint16_t position;	//��������� ������������
		uint8_t reserv;		//
		uint8_t out;		//�������� ���������� �������
		}fld;
	uint8_t byte[8];
	uint16_t word[4];
	uint32_t dword[2];
	}bcu_tx_data_t; //��������� ������������� ������

#pragma pack()

void bcu_init(uint8_t node_id);			//������������� ���������� ���������� ������������� (BCU)
void bcu_step(void);					//��� ��������� ������ ���������� ���������� ������������� (BCU)
void bcu_set_pwm1 (int32_t data);		//������������� �������� ���1
void bcu_set_pwm2 (int32_t data);		//������������� �������� ���2
void bcu_set_position (int32_t data);	//������������� ��������� ������������
void bcu_set_out (uint8_t data);			//������������� �������� ���������� �������
uint8_t bcu_get_in (void);				//���������� ��������� ���������� ������
int32_t bcu_get_t (void);				//���������� �������� ������� �����������
int32_t bcu_get_p (void);				//���������� �������� ������� ��������
int32_t bcu_get_position (void);		//���������� ��������� ������������
int32_t bcu_get_torque (void);			//���������� �������� ��������� ������� � ������� �������
int32_t bcu_get_frequency(void);		//���������� �������� ������� � ������� �������
int32_t bcu_get_power(void);			//���������� �������� �������� � ������� �������
uint8_t bcu_err_link (void);			//���������� ��������� ����� � BCU
void bcu_update_data (char *data, uint8_t len, uint32_t adr);	//���������� ������� �������� �� CanOpen �� BCU

#endif /* APP_DEVICES_BCU_H_ */