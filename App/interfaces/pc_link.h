/*
 * pc_link.h
 *
 *  Created on: 11 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_INTERFACES_PC_LINK_H_
#define APP_INTERFACES_PC_LINK_H_
#include "types.h"

#define PC_LINK_REV 1000				//��������� ������ ��������� (��� ����� ������)

#define PC_LINK_MIN_TX_TIME 30 			//����������� ������� ����� ���������� �������
#define PC_LINK_MAX_RX_TIMEOUT 1000 	//����������� ���������� �������� ����� ������� �������
#define PC_LINK_MAX_RX_PACK_TIME 30		//�����������-���������� ����� ����� ������ ������

#define PC_LINK_BUF_SIZE 1500			//������ ������ �� ���/�������� (������ ���� �� ������ ������������� ������� ������)

#define PC_LINK_LOSS 0  				//��� ������ �� ��
//-----------------------------�������� ������ � ��--------------------
#pragma pack(1)
typedef struct
	{
	uint8_t start;
	uint16_t session;
	uint16_t number;
	uint16_t version;
	uint8_t type;
	uint16_t size;
	}header_t;
typedef union
	{
	 struct
		{
		header_t header;
		uint8_t data[PC_LINK_BUF_SIZE-sizeof(header_t)];
		}fld;
	uint8_t byte[PC_LINK_BUF_SIZE];
	uint16_t word[PC_LINK_BUF_SIZE>>1];
	uint32_t dword[PC_LINK_BUF_SIZE>>2];
	}pc_link_pack_t; //������ ������
#pragma pack()

void pc_link_init(void);									//������������� ������ ������ � ��
void pc_link_reset(void);									//����� ������ ������ � �� (�����������������)
uint8_t pc_link_get_rx_mode(void);							//���������� ��������� ������ ������ ��� ��������� �������� ������
uint16_t pc_link_get_pc_session(void);						//���������� ����� ������ ��� ��������� �������� ������
//uint8_t pc_link_get_rev(void);							//
uint16_t pc_link_read_data(void *buf, uint16_t size);		//��������� ��������� �������� ����� � ������ CRC
uint16_t pc_link_write_data(uint8_t mode, uint16_t session, uint8_t *p, uint16_t size);	//�������� ������ ��� �������� � ������� ��������
void pc_link_set_rev(uint16_t rev);	//������������� �������� ���� "������ ���������", ������ ��������� ������� �� ���� ������, ������� ��������������� ������� ��������
uint8_t pc_link_tx_ready(void);								//���������� ��������� ���������� ������ �������� ������ �� ��
void pc_link_step(void);				//��� ������;
static void pc_link_rx_data(void);		//���������� �������� �������: ���������� ��� �� ������ �������, ������ ��������� � ��������� �� CRC
static void pc_link_tx_data(void);		//��������� ��������� ������ � ���������� ������ �� ������ (���������� pc_link_write_data)

#endif /* APP_INTERFACES_PC_LINK_H_ */