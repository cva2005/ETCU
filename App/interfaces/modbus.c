/*
 * modbus.c
 *
 *  Created on: 29 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "modbus.h"
#include "rs485_1.h"
#include "rs485_2.h"
#include "timers.h"
#include "crc.h"
#include <string.h>

modbus_rx_t modbus_rx[MODBUS_MAX_DEV]; //��������� �� ������� ����������� ������� �� ��������� CanOpen
static uint8_t modbus_channel=0;//������� ����� RS-485 �� ������� ���������������� ���������� ModBus
static stime_t modbus_tx_time; //����� ����� �������� ��������� ����
static stime_t modbus_rx_time; //���� ����� ������ ���� ������ ��������� ����
static uint8_t modbus_busy;		//��������� ���������� ModBus: 0-��������, 1-�������� ������ ������ �� ������
static uint16_t modbus_rx_size; //������ �������� ������
static uint8_t modbus_buf[256];
static uint8_t modbus_rq_tx[MODBUS_MAX_DEV]; //������� �������� �������� �� ������ ���� ����� ������ ��������� ������ ��� ���������
static uint8_t modbus_rq_point=0; //��������� � ������� �������� �� ���������� � ������������ ����������� ��������: ���� ������� ����� ����� ������������� ���� ���������� �� ���� ������ ����������� ������ � ��������� ��������

/**
  * @brief  ������������� ���������� ModBus
  *
  * @param  chan: ����� RS485 �� ������� ����������� ���������������� ����������� ModBus: 1 ��� 2
  */
void modbus_init(uint8_t chan)
{uint8_t cnt;
	for (cnt=0; cnt<MODBUS_MAX_DEV; cnt++)
		{
		modbus_rx[cnt]=NULL;
		modbus_rq_tx[cnt]=0;
		}
	modbus_busy=0;
	modbus_tx_time=timers_get_finish_time(0);
	modbus_rx_time=timers_get_finish_time(0);
	modbus_rx_size=0;
	modbus_rq_point=0;


#ifdef RS485_1_UART_NUMBER
	if (chan==1)
		{
		modbus_channel=1;
		}
#endif
#ifdef RS485_2_UART_NUMBER
	if (chan==2)
		{
		modbus_channel=2;
		}
#endif
}

/**
  * @brief  ��� ������ ModBus: ���������� �������� � �������� ����
  *
  */
void modbus_step(void)
{uint32_t rx_size;
 uint16_t crc_rx, crc_calc;
 uint8_t cnt;

 	 if (timers_get_time_left(modbus_tx_time)==0)
 		 modbus_busy=0; //���� �� ��������� ������, �� ����������  ���������

#ifdef RS485_1_UART_NUMBER
	if (modbus_channel==1)	//���� ModBus �� ������ 1
		rx_size=rs485_1_get_rx_size();	//��������� ���-�� �������� ����
#endif
#ifdef RS485_2_UART_NUMBER
	if (modbus_channel==2)	//���� ModBus �� ������ 2
		rx_size=rs485_2_get_rx_size();	//��������� ���-�� �������� ����
#endif
	if (rx_size!=modbus_rx_size) //���� ���������� �������� ���� �����������
		{
		modbus_rx_size=rx_size;	//�������� ���-�� �������� ����
		modbus_rx_time=timers_get_finish_time(MODBUS_BYTE_RX_PAUSE);	//���������� ����������� ���������� ����� ����� ���������� �����
		return;
		}

	if (rx_size!=0)	//���� ���� �������� �����
		{
		if (timers_get_time_left(modbus_rx_time)==0)	//���� ������� ����� �������� ���������� �����: ���� ��������
			{
#ifdef RS485_1_UART_NUMBER
			if (modbus_channel==1)	//���� ModBus �� ������ 1
				rs485_1_read_rx_data(modbus_buf, rx_size);	//��������� ��� �������� ����� �� ������
#endif
#ifdef RS485_2_UART_NUMBER
			if (modbus_channel==2)	//���� ModBus �� ������ 1
				rs485_2_read_rx_data(modbus_buf, rx_size);	//��������� ��� �������� ����� �� ������
#endif
			crc_rx=((uint16_t)modbus_buf[rx_size-2]<<8)|modbus_buf[rx_size-1];		//��������� CRC ��������� ������
			crc_calc=modbus_crc_rtu(modbus_buf, rx_size-2);	//��������� CRC ��������� ������
			if (crc_rx==crc_calc)							//���� CRC c��������
				{
				modbus_busy=0; //���� ������ �����, �� ����������  ���������
				for(cnt=0; cnt<MODBUS_MAX_DEV; cnt++)		//����� ����������� �������� �������
					if (modbus_rx[cnt]!=NULL)
						modbus_rx[cnt](&modbus_buf[3], modbus_buf[2], modbus_buf[0], modbus_buf[1]); //������� ���������� �������� �������
				}
			}
		}
}


/**
  * @brief  �������: ��������� �������� �������� ������ ��� ���������� ������� ���������
  *
  * @param  adr: ����� ����������
  * 		reg: ����� �������������� ��������
  * 		number: ���������� ������������� ���������
  *
  * @retval ��������� �������: 1 - �������, 0 - ������ �� ���������
  */
uint8_t modbus_rd_in_reg(uint8_t adr, uint16_t reg, uint16_t number)
{uint8_t pack[8], st;
udata16_t crc;

	//if (modbus_busy) return(0);
	if (modbus_get_busy(adr)) return(0);

	pack[0]=adr;
	pack[1]=MODBUS_READ_INPUTS_REGISTERS;
	pack[2]=reg>>8;
	pack[3]=reg;
	pack[4]=number>>8;
	pack[5]=number;

	crc.word=modbus_crc_rtu(pack, 6);
	pack[6]=crc.byte[1];
	pack[7]=crc.byte[0];

	if (modbus_tx(pack, sizeof(pack)))
		{
		if (adr>0) adr--;
		modbus_rq_tx[adr]=0;
		modbus_rq_point++;
		if (modbus_rq_point>=MODBUS_MAX_DEV) modbus_rq_point=0;
		return(1);
		}
	return(0);
}

/**
  * @brief  �������: ��������� �������� �������� ������ ��� ���������� ��������� ��������
  *
  * @param  adr: ����� ����������
  * 		reg: ����� �������������� ��������
  * 		number: ���������� ������������� ���������
  *
  * @retval ��������� �������: 1 - �������, 0 - ������ �� ���������
  */
uint8_t modbus_rd_hold_reg(uint8_t adr, uint16_t reg, uint16_t number)
{uint8_t pack[8];
udata16_t crc;

	//if (modbus_busy) return(0);
	if (modbus_get_busy(adr)) return(0);

	pack[0]=adr;
	pack[1]=MODBUS_READ_HOLDING_REGISTERS;
	pack[2]=reg>>8;
	pack[3]=reg;
	pack[4]=number>>8;
	pack[5]=number;

	crc.word=modbus_crc_rtu(pack, 6);
	pack[6]=crc.byte[1];
	pack[7]=crc.byte[0];

	if (modbus_tx(pack, sizeof(pack)))
		{
		if (adr>0) adr--;
		modbus_rq_tx[adr]=0;
		modbus_rq_point++;
		if (modbus_rq_point>=MODBUS_MAX_DEV) modbus_rq_point=0;
		return(1);
		}
	return(0);
}

/**
  * @brief  �������: ������ ������ �������� � ������� ��������
  *
  * @param  adr: ����� ����������
  * 		reg: �����  ��������
  * 		val: ������������ ��������
  *
  * @retval ��������� �������: 1 - �������, 0 - ������ �� ���������
  */
uint8_t modbus_wr_1reg(uint8_t adr, uint16_t reg, uint16_t val)
{uint8_t pack[8];
udata16_t crc;

	//if (modbus_busy) return(0);
	if (modbus_get_busy(adr)) return(0);

	pack[0]=adr;

	pack[1]=MODBUS_FORCE_SINGLE_REGISTER;
	pack[2]=reg>>8;
	pack[3]=reg;
	pack[4]=val>>8;
	pack[5]=val;

	crc.word=modbus_crc_rtu(pack, 6);
	pack[6]=crc.byte[1];
	pack[7]=crc.byte[0];

	if (modbus_tx(pack, sizeof(pack)))
		{
		if (adr>0) adr--;
		modbus_rq_tx[adr]=0;
		modbus_rq_point++;
		if (modbus_rq_point>=MODBUS_MAX_DEV) modbus_rq_point=0;
		return(1);
		}
	return(0);
}


/**
  * @brief  �������: ���������������� �������
  *
  * @param  adr: ����� ����������
  * 		func: ����� �������
  * 		ln: ����� ������
  * 		*data: ��������� ������ �������
  *
  * @retval ��������� �������: 1 - �������, 0 - ������ �� ���������
  */
uint8_t modbus_user_function(uint8_t adr, uint8_t func, uint8_t ln, uint8_t *data)
{uint8_t pack[255];
udata16_t crc;

	//if (modbus_busy) return(0);
	if (modbus_get_busy(adr)) return(0);
	if (ln>(sizeof(pack)-sizeof(adr)-sizeof(func)-sizeof(ln))) return(0);

	pack[0]=adr;
	pack[1]=func;
	pack[2]=ln;
	memcpy(&pack[3],data,ln);

	crc.word=modbus_crc_rtu(pack, ln+sizeof(adr)+sizeof(func)+sizeof(ln));
	pack[3+ln]=crc.byte[1];
	pack[4+ln]=crc.byte[0];

	if (modbus_tx(pack, ln+sizeof(adr)+sizeof(func)+sizeof(ln)+2))
		{
		if (adr>0) adr--;
		modbus_rq_tx[adr]=0;
		modbus_rq_point++;
		if (modbus_rq_point>=MODBUS_MAX_DEV) modbus_rq_point=0;
		return(1);
		}
	return(0);
}

/**
  * @brief  ���������� ���������� ModBus � �������� ������
  *
  * @param  adr: ����� ���������� ������ ����������� ������ � ����.
  * ���� 0 - �� ����������� ���������� ����� ��� ����� �����������
  * ���� >0 - ����������� ������ ��������, � ��������� ������������ � ������ ����������� �� ��������
  * � ������ ���� ����� ������, �� �������� ���������� �������� � ������� �������� �� ��������
  *
  * @retval ��������� ����������: 1 - ModBus �����, 0 - ModBus ��������
  */
uint8_t modbus_get_busy(uint8_t adr)
{uint8_t cnt;
	if (adr>0) //���� �������� ����������� ����� � ������ �����������
		{
		adr--;
		if (modbus_busy==0) //���� ��������
			{
			cnt=modbus_rq_point;		//�������� ��������� ������������ ������� � ����������, ������� �� ������ ���� ����� ������������ ���������
			while ((cnt!=adr)&&(modbus_rq_tx[cnt]==0)) //�������� ���� �� � ������� ���������� � ������� ����������� ��������
				{
				cnt++;
				if (cnt>=MODBUS_MAX_DEV) cnt=0;
				}
			if (cnt==adr) //���� ��� ��������� � ������� �����������
				return(0); //������� ��� ����� ��������
			else		//���� ���� ���������� � ������� ����������� ��������
				{
				modbus_rq_tx[adr]=1; //���������� ���� ������� �� ��������
				return(1); //������� ��� ���� ������ (������� ������� �������� ����� ������������� ����������)
				}
			}
		else return(1);
		}
	else return(modbus_busy);
}

/**
  * @brief  ���������� ����� �� ���������� ModBus
  *
  * @param  *data: ��������� �� �����
  * 		len: ������ ������
  *
  * @retval ��������� ��������: 1 - �������, 0 - ������ �� ���������
  */
static uint8_t modbus_tx(uint8_t *data, uint16_t len)
{
	if (modbus_busy) return(0);

#ifdef RS485_1_UART_NUMBER
	if (modbus_channel==1)
		if (rs485_1_write_tx_data(data, len))
			{
			modbus_busy=1;
			modbus_tx_time=timers_get_finish_time(MODBUS_MAX_WAIT_TIME);
			return(1);
			}
#endif
#ifdef RS485_2_UART_NUMBER
	if (modbus_channel==2)
		if (rs485_2_write_tx_data(data, len))
			{
			modbus_busy=1;
			modbus_tx_time=timers_get_finish_time(MODBUS_MAX_WAIT_TIME);
			return(1);
			}

#endif
	return(0);
}

/**
  * @brief  �������: ������ ������ �������� � �������� ��������
  *
  * @param  adr: ����� ����������
  * 		reg: �����  1-�� ��������
  * 		num: ���������� ���������
  * 		val: ������������ ��������
  *
  * @retval ��������� �������: 1 - �������, 0 - ������ �� ���������
  */
uint8_t modbus_wr_mreg(uint8_t adr, uint16_t reg, uint16_t num, uint8_t* data)
{
	uint8_t pack[MB_TX_BUFF], i, j; udata16_t crc;

	if (modbus_get_busy(adr)) return(0);
	pack[0] = adr;
	pack[1] = MODBUS_FORCE_MULTIPLE_REGISTERS;
	pack[2] = (uint8_t)(reg >> 8);
	pack[3] = (uint8_t)reg;
	pack[4] = (uint8_t)(num >> 8);
	pack[5] = (uint8_t)num;
	num *= 2;
	pack[6] = (uint8_t)num;
	for (i = 0, j = 7; i < num; i++, j++)
		pack[j] = data[i];
	crc.word = modbus_crc_rtu(pack, j);
	pack[j++] = crc.byte[1];
	pack[j++] = crc.byte[0];
	if (modbus_tx(pack, j)) {
		if (adr > 0) adr--;
		modbus_rq_tx[adr] = 0;
		modbus_rq_point++;
		if (modbus_rq_point >= MODBUS_MAX_DEV)
			modbus_rq_point = 0;
		return 1;
	}
	return 0;
}