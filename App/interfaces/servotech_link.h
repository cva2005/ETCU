/*
 * servotech_link.h
 *
 *  Created on: 26 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_INTERFACES_SERVOTECH_LINK_H_
#define APP_INTERFACES_SERVOTECH_LINK_H_
#include "types.h"

#define SERVOTECH_LINK_MAX_DEV 1 //������������ ��������� ���������� ���������
#define SERVOTECH_LINK_ADR 0 	 //����� ���������� ������� (����� �������������)
#define SERVOTECH_LINK_MODE 0	 //����� ������ �����: ������, �����, ���������� ����������, ������������

#define SYNC_ID 0x87	//ID ������ �������������


typedef enum
	{
	CMD_SET_POS = 8,	//������� ��������� �������, �������� � ������� ���������� ������
	CMD_SET_SPEED = 9,	//������� ��������� ��������
	CMD_SET_I = 10,		//������� ��������� ����
	CMD_ST_POS = 12,	//������� �������� ���������
	CMD_TX_USB = 13,	//������� �������� ������ �� ���������������� ���������
	CMD_RX_USB = 14,	//���� ����� �� ���������������� ���������
	}servotech_link_cmd_t; //������� ������ �� CAN
typedef enum
	{
	TECH_RQ = 5,	//������ �������� ���������: ������� ���������� ������� ������������ ��� ������� �������� ���������
	TECH_RSP = 6,	//�������� �������� �������� ���������: ������� ���������� �������� � ����� �� ������� 5
	TECH_SET = 7,	//��������� �������� ���������: ������� ���������� ������� ������������ ��� ��������� �������� ���������
	TECH_START = 17	//��������� ��������� ���
	}servotech_link_tech_t; //������� ���������������� ���������

#pragma pack(1)
typedef union
	{
	struct
		{
		uint8_t sink_addr: 3;		//����� ����������
		uint8_t source_addr: 3;		//����� ���������
		uint8_t cmd: 5;	//�������
		uint32_t notuse: 21;
		}f;
	uint8_t byte[4];
	uint16_t word[2];
	uint32_t dword;
	}servotech_link_id_t; //������ ��������������
#pragma pack()

typedef void (*servotech_link_rx_t) (char *data, uint8_t len, uint32_t adr); //�������� ��������� - ����������� �������� �������

void servotech_link_init(uint8_t chanal);//������������� ����������
uint8_t servotech_link_set_channel(uint8_t channel);//�������� ����� CAN ����� ������� �������� ��������
void servotech_link_step(void);//��� ������: ���������� �������� � �������� ����
uint8_t servotech_link_tx_tech(uint8_t adr, servotech_link_tech_t cmd, uint16_t parameter, uint32_t data);// ��������� ����� � ��������������� ��������
uint8_t servotech_link_tx_cmd(servotech_link_cmd_t cmd, uint8_t adr, int8_t *data, uint8_t ln);//��������� ����� �������
uint8_t servotech_link_get_src_adr(uint32_t adr);//�������� �� CAN ������: ����� ���������
uint8_t servotech_link_get_dst_adr(uint32_t adr);//�������� �� CAN ������: ����� ����������
uint8_t servotech_link_get_cmd(uint32_t adr);//�������� �� CAN ������: �������
uint16_t servotech_link_get_pr(uint8_t *data);//�������� �� ������ �������� ��������� ��� ����������������� ���������
uint8_t servotech_link_get_tech_cmd(uint8_t *data);//�������� ������� ���������������� ���������


#endif /* APP_INTERFACES_SERVOTECH_LINK_H_ */