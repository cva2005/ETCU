/*
 * CanOpen.h
 *
 *  Created on: 26 ���. 2014 �.
 *      Author: ������ �.�.
 */

#ifndef CANOPEN_H_
#define CANOPEN_H_
#include "stm32f4xx_hal.h"

#define MAX_DEV_CANOPEN 32 //������������ ��������� ���������� ��������� CanOpen

typedef void (*CanOpen_rx_object_t) (char *data, uint8_t len, uint32_t adr); //�������� ��������� - ����������� �������� �������� CanOpen

void CanOpen_init(uint8_t chan);//������������� ���������� CanOpen
uint8_t CanOpen_set_channel(uint8_t channel);//�������� ����� CAN ����� ������� �������� CanOpen
void CanOpen_step(void);//��� ������ CanOpen: ���������� �������� � �������� ����
uint8_t CanOpen_tx_nmt(uint8_t type, uint8_t node_id);//��������� ����� NMT (�������� CanOpen)
uint8_t CanOpen_tx_sdo(uint8_t node_id, uint16_t index, uint8_t subindex, uint32_t data, uint8_t cmd);// ��������� ����� SDO (�������� CanOpen)
uint8_t CanOpen_tx_pdo(uint8_t pdo_n, uint8_t node_id, int8_t *data, uint8_t ln);//��������� ����� PDO (�������� CanOpen)
uint8_t CanOpen_get_nodeid(uint32_t adr);//�������� �� CAN ������ (COB_ID), ����� ������ CanOpen (NODE ID)
uint16_t CanOpen_get_object(uint32_t adr);//�������� �� CAN ������ (COB_ID), ����� ������� (OBJECT)
uint16_t CanOpen_get_index(uint8_t *data);//�������� ������ �� ������� SDO
uint8_t CanOpen_get_subindex(uint8_t *data);//�������� ��������� �� ������� SDO

//-------------------------CAN OPEN OBJECTS-------------------------------------
#define NMT 0x00
#define SYNC 0x80
#define TIME STAMP 0x100
#define EMERGENCY 0x80
#define PDO1_TX_SLAVE 0x180
#define PDO1_RX_SLAVE 0x200
#define PDO2_TX_SLAVE 0x280
#define PDO2_RX_SLAVE 0x300
#define PDO3_TX_SLAVE 0x380
#define PDO3_RX_SLAVE 0x400
#define PDO4_TX_SLAVE 0x480
#define PDO4_RX_SLAVE 0x500
#define SDO_TX_SLAVE 0x580
#define SDO_RX_SLAVE 0x600
#define BOOTUP 0x700

//-------------------------CAN OPEN NMT COMMANDS---------------------------------
#define NODE_PREOPERATIONAL 0x80
#define NODE_START 0x01
#define NODE_STOP 0x02
#define NODE_RESET 0x81
#define NODE_RESET_COMMUNICATION 0x82

//-------------------------CAN OPEN SDO COMMANDS---------------------------------
#define WR			0x22 //�������� ������ � ����������� ����������
#define WR_4BYTE	0x23
#define WR_2BYTE	0x2B
#define WR_1BYTE	0x2F

#define WR_RESPONSE 0x60 //������������� ������ �� ������������ ����������
#define RQ			0x40 //����������� ���������� ����������� ������

#define RD			0x42 //������ ������ �������� �� ������������ ����������
#define RD_4BYTE	0x42
#define RD_2BYTE	0x4B
#define RD_1BYTE	0x4F

#define ABORT_MSG	0x80 //������ ������/������
/* ������ ��������� ABORT_MSG (������ ������/������)
05040001h : Command byte is not supported
06010000h : Incorrect access to an object
06010001h : Read access to write only
06010002h : Write access to read only
06020000h : Object is not supported
06090011h : Subindex is not supported
06090030h : Value outside the limit
06090031h : Value too great
08000000h : General error
08000020h : Incorrect save signature
08000021h : Data cannot be stored
*/
#endif /* CANOPEN_H_ */