/*
 * ds2482.c
 *
 *  Created on: 7 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "ds2482.h"

extern I2C_HandleTypeDef DS2482_I2C; //��������� �� I2C

static uint8_t ds2482_channel=0;	//������� ��������� �����
static ds2482_resp_t ds2482_resp;	//������� �������
static uint8_t ds2482_data[2]={0,0};	//������ ��� ������������ ������ �� ��������
static uint8_t ds2482_error=0;			//������� ��������� ������� ��������/�����

/**
  * @brief  ������������ ���������� DS2482
  */
void ds2482_init(void)
{uint8_t result[9], cnt;

	ds2482_error=0;
	ds2482_resp.byte=0;
	ds2482_resp.sr.WB=1;

	ds2482_data[0]=DS2482_DRST;
	if (!ds2482_wr_repeat(ds2482_data, 1, DS2482_RAPEAT_CNT)) return;
	if (!ds2482_rd_repeat(&ds2482_resp.byte, 1, DS2482_RAPEAT_CNT)) return;
	ds2482_channel=1; //����� ������ �������������� ����� 1 �������������

	ds2482_data[0]=DS2482_WCFG;
	ds2482_data[1]=DS2482_CFG_APU;
	if (!ds2482_wr_repeat(ds2482_data, 2, DS2482_RAPEAT_CNT)) return;
	if (!ds2482_rd_repeat(ds2482_data, 1, DS2482_RAPEAT_CNT)) return;
	return;
}

/**
  * @brief  �������� ������� ����� ���������� DS2482
  *
  * @param  chan: ����� ������ �� 1 �� 8
  *
  * @retval ������� ��������� �����
  */
uint8_t ds2482_set_chanel(uint8_t chan)
{uint8_t byte;

	ds2482_data[0]=DS2482_CHSL;
	if ((chan==0)||(chan==1)) ds2482_data[1]=DS2482_CHSL_IO0_WR;
	if (chan==2)	ds2482_data[1]=DS2482_CHSL_IO1_WR;
	if (chan==3)	ds2482_data[1]=DS2482_CHSL_IO2_WR;
	if (chan==4)	ds2482_data[1]=DS2482_CHSL_IO3_WR;
	if (chan==5)	ds2482_data[1]=DS2482_CHSL_IO4_WR;
	if (chan==6)	ds2482_data[1]=DS2482_CHSL_IO5_WR;
	if (chan==7)	ds2482_data[1]=DS2482_CHSL_IO6_WR;
	if (chan>=8)	ds2482_data[1]=DS2482_CHSL_IO7_WR;

	if (!ds2482_wr_repeat(ds2482_data, 2, 1)) return(ds2482_channel);
	if (!ds2482_rd_repeat(&byte, 1, 1)) return(ds2482_channel);
	if (byte==DS2482_CHSL_IO0_RD) ds2482_channel=1;
	if (byte==DS2482_CHSL_IO1_RD) ds2482_channel=2;
	if (byte==DS2482_CHSL_IO2_RD) ds2482_channel=3;
	if (byte==DS2482_CHSL_IO3_RD) ds2482_channel=4;
	if (byte==DS2482_CHSL_IO4_RD) ds2482_channel=5;
	if (byte==DS2482_CHSL_IO5_RD) ds2482_channel=6;
	if (byte==DS2482_CHSL_IO6_RD) ds2482_channel=7;
	if (byte==DS2482_CHSL_IO7_RD) ds2482_channel=8;

	return(ds2482_channel);
}

/**
  * @brief  ������ ��������� �������� ������� DS18B20
  *
  * @retval ��������� ���� 1WB: 1-���� 1-wire ������, 0-����� 1-wire ��������
  */
uint8_t ds2482_get_busy(void)
{
	if (!ds2482_rd_repeat(&ds2482_resp.byte, 1, 1)) ds2482_resp.sr.WB=1;
	return (ds2482_resp.sr.WB);
}

/**
  * @brief  ��������� present ������� � ����� 1-wire
  *
  * @retval ��������� ��������: 1-������� ���������, 0-������ �� ������ ��������
  */
uint8_t ds2482_send_present_1wire(void)
{
	ds2482_data[0]=DS2482_1WRS;
	if (!ds2482_wr_repeat(ds2482_data, 1, 1)) return(0);
	ds2482_resp.sr.WB=1;
	ds2482_resp.sr.PPD=0;
	return(1);
}

/**
  * @brief  ���������� ����� �� ��������� present �������: ������ ������������ ������ ����� ds2482_get_busy()==0
  *
  * @retval ��������� ��������: 1-������� ���������, 0-������ �� ������ ��������
  */
uint8_t ds2482_get_present_1wire(void)
{
	return (ds2482_resp.sr.PPD);
}

/**
  * @brief �������� 1 ���� �� 1-wire
  *
  * @param  data: ���� ������ ��� ��������
  *
  * @retval ��������� ��������: 1-������ ��������, 0-������ ��������
  */
uint8_t ds2482_wr_1wire(uint8_t data)
{
	ds2482_data[0]=DS2482_1WWB;
	ds2482_data[1]=data;
	if (!ds2482_wr_repeat(ds2482_data, 2, 1)) return(0);
	ds2482_resp.sr.WB=1;
	return(1);
}

/**
  * @brief �������: ������ ������ ������ ���� ������ �� 1-wire
  *
  * @retval ���������: 1-������ ������, 0-������ ������ ������
  */
uint8_t ds2482_rd_bit_cmd_1wire(void)
{
	ds2482_data[0]=DS2482_1WSB;
	ds2482_data[1]=DS2482_BITBYTE_1;
	if (!ds2482_wr_repeat(ds2482_data, 2, 1)) return(0);
	ds2482_resp.sr.WB=1;
	return(1);
}

/**
  * @brief ���������� ��������� ������ ������ ���� ������ �� 1-wire: ������ ������������� ������ ���� ds2482_get_busy()==0
  *
  * @retval ��������� ������: 0bxxxxxxxV (������� ���)
  */
uint8_t ds2482_rd_bit_result_1wire(void)
{
	return(ds2482_resp.sr.SBR);
}

/**
  * @brief �������: ������ ������ ����� ������ �� 1-wire
  *
  * @retval ���������: 1-������ ������, 0-������ ������ ������
  */
uint8_t ds2482_rd_byte_cmd_1wire(void)
{
	ds2482_data[0]=DS2482_1WRB;
	if (!ds2482_wr_repeat(ds2482_data, 1, 1)) return(0);
	ds2482_resp.sr.WB=1;
	return(1);
}

/**
  * @brief ���������� ��������� ������ ����� ������ �� 1-wire: ������ ������������� ������ ���� ds2482_get_busy()==0
  *
  * @retval ����������� ������
  */
uint8_t ds2482_rd_byte_result_1wire(void)
{uint8_t dt;
	ds2482_data[0]=DS2482_SRP;
	ds2482_data[1]=DS2482_SRP_RDR;
	if (!ds2482_wr_repeat(ds2482_data, 2, 1)) return(0xAA);
	if (!ds2482_rd_repeat(&dt, 1, 1)) return(0xBB);
	return(dt);
}

/**
  * @brief  ���������� ���������� DS2482 �������� ���������� ����, � ������ ������ ��������, ��������� �������� �������� ���������� ���
  *
  * @param  *data: ��������� �� ����� � ������� ��� ��������
  * 		size: ���������� ������ ��� ��������
  * 		repeat: ���������� ������� ��������, � ������ ������ ��������
  *
  * @retval ��������� �������: 1-������ ������� ����������; 0-������ �� ������� ���������
  */
static uint8_t ds2482_wr_repeat(uint8_t* data, uint8_t size, uint8_t repeat)
{HAL_StatusTypeDef res;

	if (ds2482_error>=DS2482_MAX_ERR) return(0);
	do
		{
		res=HAL_I2C_Master_Transmit(&DS2482_I2C, DS2482_ADRESS_7BIT(DS2482_ADRESS), data, size, 10);
		if (repeat>0) repeat--;
		if (res!=HAL_OK)
			{
			ds2482_error++;
			if (ds2482_error>=DS2482_MAX_ERR) return(0);
			if (repeat>0) HAL_Delay(DS2482_RAPEAT_DELAY);
			else return(0);
			}
		}
	while (res!=HAL_OK);
	ds2482_error=0;
	return(1);
}

/**
  * @brief ������ �� ���������� DS2482 �������� ���������� ����, � ������ ������ ������, ��������� ������ �������� ���������� ���
  *
  * @param  *data: ��������� �� ����� ��� ������ ������
  * 		size: ���������� ������ ��� ������
  * 		repeat: ���������� ������� ������, � ������ ������
  *
  * @retval ��������� ������: 1-������ ������� ���������; 0-������ �� ������� ���������
  */
static uint8_t ds2482_rd_repeat(uint8_t* data, uint8_t size, uint8_t repeat)
{HAL_StatusTypeDef res;

	if (ds2482_error>=DS2482_MAX_ERR) return(0);
	do
		{
		res=HAL_I2C_Master_Receive(&DS2482_I2C, DS2482_ADRESS_7BIT(DS2482_ADRESS), data, size, 10);  //������ ���������
		if (repeat>0) repeat--;
		if (res!=HAL_OK)
			{
			ds2482_error++;
			if (ds2482_error>=DS2482_MAX_ERR) return(0);
			if (repeat>0) HAL_Delay(DS2482_RAPEAT_DELAY);
			else return(0);
			}
		}
	while (res!=HAL_OK);
	ds2482_error=0;
	return(1);
}