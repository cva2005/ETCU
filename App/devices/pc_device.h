/*
 * pc_device.h
 *
 *  Created on: 11 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DEVICES_PC_DEVICE_H_
#define APP_DEVICES_PC_DEVICE_H_
#include "types.h"

#define PC_DEVICE_MAX_SESSION 2 		//������������ ����� ����������� (���������)
#define PC_DEVICE_TIMEOUT_SESSION 5000	//����� �� ��������� �������� ������ ������������� �����������, ���� �� ���� ������� ������
#define PC_DEVICE_TX_TIME 100			//�������� ����� ���������� ������� ������� �� ������������ ���������

//#define PC_DEVICE_DATA_SZIE 1000		//������������ ������ ���� ������ (������ ���� �� ������ ��� ������ ���� ������ � ������ pc_link)

#define PC_DEVICE_RQ_CONNECT 0x10		//������ ����������
#define PC_DEVICE_RSP_CONNECT 0x11		//����� �� ������ ����������
//#define PC_DEVICE_SIG 1					//������� ��������
//#define PC_DEVICE_DATA 2				//����� ������
//#define PC_DEVICE_SYS 3					//����� ��������� ������
//#define PC_DEVICE_RST 5					//����� � ��������� ����������

#define FLD_MODE_CONNECTION 0
#define FLD_ERR_CONNECTION 1
#define FLD_SESSION 2
//#define FLD_RQ_PARAMETR 1
//#define FLD_WR_PARAMETR 1
//#define FLD_STATUS 0

#define DT_CONNECTION_ERR 0 //������ ����������� ���������� (��� ��������� ������)
#define DT_CONNECTION_CTR 1	//����������� � ������ ����������
#define DT_CONNECTION_MNT 2	//����������� � ������ �����������
#define DT_SESSION_BROADCAST 0	//������ ����������� (����������������� ����� ��� ������ ������)

//#define RESULT_NO_PARAMETR  0x01
//#define RESULT_NOT_ALL_PR	0x02
//#define RESULT_NOT_WRITE	0x04

typedef struct
	{
	uint16_t session; //������������� ���������� (������)
	stime_t time; //����� ��� ���������� �������� ������� ��������� �����������
	stime_t tx_time; //����� ����� ���� ��������� ��������� �����
	uint8_t mode;	//����� ������ ������ (��������� ��� ��������)
	uint8_t type;	//��� �������, ������� ��������� ���������
	}pc_device_session_t;

void pc_device_init(void);	//������������� ���������� ��
void pc_device_step(void);	//��� ��������� ������ ���������� ��
void pc_device_rx_callback(uint8_t type, uint8_t size, void *pack); //����: ��������������� ������� ������ ����������� �������� �������: ���������� ���������� ��� ���������� ��������� ����������
static void pc_device_process_rx(void);	//����������� ������ ���������� ��� ��
static void pc_device_prepare_tx(void);	//��������� ������ ��� �������� �� ��

#endif /* APP_DEVICES_PC_DEVICE_H_ */