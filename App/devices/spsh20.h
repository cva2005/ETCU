/*
 * spsh20.h
 *
 *  Created on: 26 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DEVICES_SPSH20_H_
#define APP_DEVICES_SPSH20_H_
#include "types.h"

//#define SPSH20_LOOP_CONTROL			//: ���� ����������, �� ����� ���������� ���������


#define SPSH20_CONNECT_TIME 200		//������������ ���������� ����� �������� �������, ��� ���������� ��������� ����� � BCU ����������
#define SPSH20_DATA_TX_TIME 20		//������� ����� ������������� �������� �������
#define SPSH20_DATA_TX_RQ_TIME 2	//������� ����� ������������� �������� �������

#define SPSH20_MAX_POSITION 	 0
#define SPSH20_MIN_POSITION 	-420000

#define MAX_SERVO_POSITION 	SPSH20_MAX_POSITION
#define MIN_SERVO_POSITION	SPSH20_MIN_POSITION

typedef enum { //��������� : ����� �������� �������
	PR_CT3_POS = 0x0402,	// �������� ������� 0x0204 (������ ���������� ��� ��������� ������� ��������� -2147483647;2147483647)
	PR_DD8_POS = 0x0C04,	// ������� ������� 0x040C
	PR_UP16_I = 0x1007,		// ������� ��� 0x0710
	PR_DD11_ST = 0x0F04,	// ��������� ������� 0x040F
	PR_CT2_SP = 0x0202,		// ������� �������� ��/��� 0x0202 (������ ���������� ��� ����������� ������� ��������� -12000;12000)
	PR_DD4_SP = 0x0804,		// ������� �������� �������� ��/��� 0x0408 (������� �������� �������� -5000;5000)
	PR_DD5_SP = 0x0904,		// �������� �������� �������� ��/��� 0x0409 (�������� �������� �������� -5000;5000)
} spsh20_pr_t; //��������� ������� ���-20

typedef enum {
	STATE_GET = 0,
	POSITION_GET,
	POSITION_SET
} spsh20_tx_t; // ��� ������� ���-20


#pragma pack(1)
typedef union
	{
	struct
		{
		int32_t pos;		//���������/��������
		struct
			{
			uint8_t out0 :1;	//�������� ������ 1
			uint8_t out1 :1;	//�������� ������ 2
			uint8_t en :1;		//������ ���������� ������ ���
			uint32_t speed :29;	//�������� ��������
			}s;
		}f;
	uint8_t byte[8];
	uint16_t word[4];
	uint32_t dword[2];
	}spsh20_tx_data_t; //��������� ������������� ������

typedef union
	{
	struct
		{
		int32_t pos;		//���������
		struct
			{
			uint8_t din :4;	//�������� ���������� ������
			uint8_t st :2;		//��������� �������
			uint16_t ain0 :12;	//�������� ����������� ����� 1
			uint16_t ain1 :12;	//�������� ����������� ����� 2
			}s;
		}f;
	uint8_t byte[8];
	uint16_t word[4];
	uint32_t dword[2];
	}spsh20_rx_data_t; //��������� ������������� ������
#pragma pack()

void spsh20_init(uint8_t id);			//������������� ���������� ���������� ������������� (BCU)
void spsh20_step(void);					//��� ��������� ������ ���������� ���������� ������������� (BCU)
void spsh20_set_pos (int32_t data);		//������������� �������� �������
int32_t spsh20_get_pos (void);				//���������� ��������� �������
uint8_t spsh20_get_status (void);			//���������� ������ �������
uint8_t spsh20_err_link (void);				//���������� ��������� �����
void spsh20_update_data (char *data, uint8_t len, uint32_t adr);	//���������� ������� �������� �� ������������


#endif /* APP_DEVICES_SPSH20_H_ */