/*
 * nl_3dpas.h
 *
 *  Created on: 31 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DEVICES_NL_3DPAS_H_
#define APP_DEVICES_NL_3DPAS_H_
#include "types.h"

#define NL_3DPAS_DATA_CONVERT_TIME 1300	//����� ��� ��������� ��������� ���������� NL-3DPAS
#define NL_3DPAS_DATA_REINIT_TIME	200 //����� ����� ������� ���������� � ��������� ������������ �� ��������������
#define NL_3DPAS_MAX_ERR_SEND	2		//����������� ���������� ���������� �������� ��� ������

void nl_3dpas_init(uint8_t adr);				//������������� ������� ����������  ���������
void nl_3dpas_step(void);						//��� ��������� ������ ������� ���������� ���������
uint8_t nl_3dpas_err_link (void);				//���������� ��������� ����� � NL-3DPAS
uint32_t nl_3dpas_get_temperature(void);		//���������� �������� �����������
uint32_t nl_3dpas_get_pressure(void);			//���������� �������� ������������ ��������
uint32_t nl_3dpas_get_humidity(void);			//���������� �������� ���������
void nl_3dpas_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function);	//���������� ������� �������� �� ModBus �� NL-3DPAS

typedef struct
	{
	uint16_t temperature;
	uint16_t pressure;
	uint16_t humidity;
	}nl_3dpas_rx_data_t;

enum
	{
	NL_3DPAS_CONVERT, //��� ��������� ������ - ��������� ��������������
	NL_3DPAS_READ,	  //��� ��������� ������ - ��������� ���������
	};

#endif /* APP_DEVICES_NL_3DPAS_H_ */