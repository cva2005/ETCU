/*
 * nl_3dpas.c
 *
 *  Created on: 31 ���. 2016 �.
 *      Author: ������ �.�
 */
#include "nl_3dpas.h"
#include "modbus.h"
#include "timers.h"
#include <string.h>

nl_3dpas_rx_data_t nl_3dpas_rx_data; //������ ����������� �� ������� ���������� ���������

uint8_t nl_3dpas_err_send;			//������� ������� ��� ������
uint8_t nl_3dpas_adr;				//����� ������� NL-3DPAS
stime_t nl_3dpas_tx_time;			//����� ��������� �������� �������
uint8_t nl_3dpas_tx_type=NL_3DPAS_CONVERT;	//����� ���� ������: �������������� ��� ������ ����������

/**
  * @brief ������������� ������� ����������  ���������
  *
  * @param  adr: ����� �������
  */
void nl_3dpas_init(uint8_t adr)
{uint8_t cnt=0;
extern modbus_rx_t modbus_rx[MODBUS_MAX_DEV]; //��������� �� ������� ����������� ������� �� modbus

	nl_3dpas_err_send = 0;
	nl_3dpas_adr=0;
	nl_3dpas_tx_type=NL_3DPAS_CONVERT;

	if ((adr<=247)&&(adr>0))
		{
		while ((modbus_rx[cnt]!=NULL)&&(cnt<MODBUS_MAX_DEV))	cnt++; //����� ��������� ���������
		if (cnt<MODBUS_MAX_DEV)
			{
			modbus_rx[cnt]=nl_3dpas_update_data; 	//������� ���������� �������� �������
			nl_3dpas_adr=adr;					  		//��������� �����
			nl_3dpas_tx_time=timers_get_finish_time(0);	//���������� ����� �������� ���������� ������
			}
		}
}

/**
  * @brief  ���������� ������� �������� �� ModBus �� NL-3DPAS
  *
  * @param  *data: ��������� �� ������ �������� �� ModBus
  * 		len: ����� �������� ������
  * 		adr: ����� ���������� �� �������� ������ ������
  * 		function: ����� �������
  */
void nl_3dpas_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function)
{uint16_t object;

	if (adr==nl_3dpas_adr) { //���� ����� �������
		nl_3dpas_err_send=0;
		if (function == MODBUS_READ_HOLDING_REGISTERS) {
			if (len>=2) nl_3dpas_rx_data.temperature = GET_UINT16(data);
			if (len>=4) nl_3dpas_rx_data.pressure = GET_UINT16(data+2);
			if (len>=6) nl_3dpas_rx_data.humidity = GET_UINT16(data+4);
			nl_3dpas_tx_time=timers_get_finish_time(NL_3DPAS_DATA_CONVERT_TIME);
		}
	}
}

/**
  * @brief  ��� ��������� ������ ������� ���������� ���������
  */
void nl_3dpas_step(void)
{
	if (timers_get_time_left(nl_3dpas_tx_time)==0)	{
		if (modbus_get_busy(nl_3dpas_adr)==0) {
			if (modbus_rd_hold_reg(nl_3dpas_adr, 0, 3)) {
				nl_3dpas_tx_time=timers_get_finish_time(NL_3DPAS_DATA_REINIT_TIME + MODBUS_MAX_WAIT_TIME);
				if (nl_3dpas_err_send<0xFF) nl_3dpas_err_send++;
			}
		}
	}
}

/**
  * @brief  ���������� ��������� ����� � NL-3DPAS
  *
  * @retval ��������� �����: 1-������ ����� (��� ������ �� NL-3DPAS) 0-����� � NL-3DPAS ��������
  */
uint8_t nl_3dpas_err_link (void)
{
	if (nl_3dpas_err_send>NL_3DPAS_MAX_ERR_SEND) return(1);
	else return(0);
}

/**
  * @brief  ���������� �������� �����������
  *
  * @retval �����������:�C (�C*1000)
  */
uint32_t nl_3dpas_get_temperature(void)
{
	return(nl_3dpas_rx_data.temperature*100);
}

/**
  * @brief  ���������� �������� ������������ ��������
  *
  * @retval ��������:���. ��.��. (��.��.��.*1000)
  */
uint32_t nl_3dpas_get_pressure(void)
{
	return(nl_3dpas_rx_data.pressure*100);
}

/**
  * @brief  ���������� �������� ���������
  *
  * @retval ���������:�% (%*100).
  */
uint32_t nl_3dpas_get_humidity(void)
{
	return(nl_3dpas_rx_data.humidity*100);
}