/*
 * ds18b20.c
 *
 *  Created on: 21 ���. 2016 �.
 *      Author: ������ �.�.
 */
#ifdef LOCAL_TEMP
#include "ds18b20.h"
#include "ds2482.h"
#include "timers.h"
#include "crc.h"
//#include <string.h>

static ds18b20_step_st_t ds18b20_step_st=DS18B20_SELECT_CHAN;	//��������� �������� ������ �������
static uint8_t	ds18b20_init_chan=0; //������������������ ������
static uint8_t ds18b20_chan=0;		//������� ����� ������
static uint8_t ds18b20_fault=0;	//������� ������ �������� �������
static uint8_t ds18b20_wait=0;		//����: ���������� �������� ���������� �������
static uint8_t ds18b20_buf[9]={0,0,0,0,0,0,0,0,0};	//����� ��� ������ ������ DS18B20
static uint8_t ds18b20_byte_cnt=0;	//��������� �� ��������� ������� ������ ������ ������ DS18B20
static stime_t ds18b20_step_timeout;	//������� ����� ����������� ����� ��������� �����������
static int32_t ds18b20_temperature[DS18B20_MAX_CHANNELS]; //�������� �����������
static uint8_t ds18b20_error[DS18B20_MAX_CHANNELS];		//�������� ������ ��� ������� ������

/**
  * @brief  ���������������� ������ DS18B20
  *
  * @param  chan: ����� ������� �� 1 �� DS18B20_MAX_CHANNELS
  */
void ds18b20_init(uint8_t chan)
{
	if (chan>DS18B20_MAX_CHANNELS) chan=DS18B20_MAX_CHANNELS;
	if (chan>0) chan--;

	ds18b20_error[chan]=0;
	if (ds18b20_init_chan==0) ds18b20_chan=chan;
	ds18b20_init_chan|=(1<<chan);
	ds18b20_temperature[chan]=0;
}

/**
  * @brief  �������� ������ �����������
  *
  * @param  chan: ����� ������� �� 1 �� DS18B20_MAX_CHANNELS
  *
  * @retval �����������*1000 (m�C)
  */
int32_t ds18b20_get_temp(uint8_t chan)
{
	if (chan>DS18B20_MAX_CHANNELS) chan=DS18B20_MAX_CHANNELS;
	if (chan>0) chan--;

	return(ds18b20_temperature[chan]);
}

/**
  * @brief  �������� ������ �� ������� �������
  *
  * @param  chan: ����� ������� �� 1 �� DS18B20_MAX_CHANNELS
  *
  * @retval ���������� ������ ������
  */
int32_t ds18b20_get_error(uint8_t chan)
{
	if (chan>DS18B20_MAX_CHANNELS) chan=DS18B20_MAX_CHANNELS;
	if (chan>0) chan--;

	return(ds18b20_error[chan]);
}

/**
  * @brief  ��� ��������� ������ �������� ����������� DS18B20: ���������� �������� � �������� ��� ���������
  */
void ds18b20_step(void)
{
	if (ds18b20_init_chan==0) return; //���� ��� ������������������ �������, �� �� ������������

	if (ds18b20_fault>10)			//���� ������� ����� ������ �� ������� ������
		{
		ds18b20_next_chan(); //����� ��������� �����
		ds18b20_step_st=DS18B20_SELECT_CHAN; //������� � ���� ������ ������
		ds18b20_step_timeout=timers_get_finish_time(10);//���������� �������� ����� ��������� �� ��������� �����
		ds18b20_fault=0;	//�������� ������� ������
		return;
		}

	if (timers_get_time_left(ds18b20_step_timeout)!=0) return; //���� �� ������� ��������, �� �����
	if (ds18b20_wait)		//���� ���������� ��������� ��������� ��������
		{
		if (ds2482_get_busy())						//��������� ��������� ��������
			{
			ds18b20_step_timeout=timers_get_finish_time(1); //���� �� ��������� ��������� ����� 1 ��.
			return;
			}
		else ds18b20_wait=0;	//���� �������� ���������, �� �������� ���� �������� ����������
		}

	switch (ds18b20_step_st)
		{
		case DS18B20_SELECT_CHAN:
			{
			if (ds2482_set_chanel(ds18b20_chan+1)==(ds18b20_chan+1)) //��������� ������� ������ ������ � ��������� ������ ����� ������ ��� ���?
				{
				ds18b20_step_st=DS18B20_SEND_CONVERT_ST;	 	//���� ������ ������ ����� ������� � ���������� ����
				ds18b20_step_timeout=timers_get_finish_time(0);
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_SEND_CONVERT_ST:
			{
			if (ds2482_rd_bit_cmd_1wire())			//��������� ������� ������ 1 ����
				{
				ds18b20_step_st=DS18B20_READ_CONVERT_ST; //������� � �������� ���������� ������ 1 ����
				ds18b20_step_timeout=timers_get_finish_time(1);	//����� 1 ��.
				ds18b20_wait=1; //���������� ���� ������������� �������� ���������� ��������
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_READ_CONVERT_ST:
			{
			if (ds2482_rd_bit_result_1wire()==1)   //���� ��������� ����������� ���������
				{
				ds18b20_step_st=DS18B20_SEND_PRESET_RD;	//������� � �������� present ��������
				ds18b20_step_timeout=timers_get_finish_time(1); //���� 1 ��.
				}
			else								//���� ���, �� ������� ���������
				{
				ds18b20_step_st=DS18B20_SEND_CONVERT_ST;
				ds18b20_cmd_retry();
				}
			break;
			}
		case DS18B20_SEND_PRESET_RD:
			{
			if (ds2482_send_present_1wire())
				{
				ds18b20_step_st=DS18B20_RESULT_PRESET_RD;
				ds18b20_step_timeout=timers_get_finish_time(1); //���� 1 ��.
				ds18b20_wait=1; //���������� ���� ������������� �������� ���������� ��������
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_RESULT_PRESET_RD:
			{
			if (ds2482_get_present_1wire())			//���� ���� ����� �� present pulse
				{
				ds18b20_step_st=DS18B20_SEND_SKIP_ROM_RD;
				ds18b20_step_timeout=timers_get_finish_time(1); //���� 1 ��.
				}
			else
				{
				if (ds18b20_error[ds18b20_chan]<0xFF) ds18b20_error[ds18b20_chan]++;
				ds18b20_next_chan(); //����� ��������� �����
				ds18b20_step_st=DS18B20_SELECT_CHAN; //������� � ������ ���������� ������
				}
			break;
			}
		case DS18B20_SEND_SKIP_ROM_RD:
			{
			if (ds2482_wr_1wire(DS18B20_SKIP_ROM))
				{
				ds18b20_step_st=DS18B20_SEND_MEM_RD;
				ds18b20_step_timeout=timers_get_finish_time(1); //���� 1 ��.
				ds18b20_wait=1; //���������� ���� ������������� �������� ���������� ��������
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_SEND_MEM_RD:
			{
			if (ds2482_wr_1wire(DS18B20_MEM_RD))
				{
				ds18b20_step_st=DS18B20_SEND_READ; //������� � �������� ������� ������ �����
				ds18b20_step_timeout=timers_get_finish_time(1); //���� 1 ��.
				ds18b20_byte_cnt=0; //��������� �� ������ ������ ��� ������
				ds18b20_wait=1; //���������� ���� ������������� �������� ���������� ��������
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_SEND_READ:
			{
			if (ds2482_rd_byte_cmd_1wire())
				{
				ds18b20_step_st=DS18B20_GET_READ;
				ds18b20_step_timeout=timers_get_finish_time(1); //���� 1 ��.
				ds18b20_wait=1; //���������� ���� ������������� �������� ���������� ��������
				}
			break;
			}
		case DS18B20_GET_READ:
			{
			ds18b20_buf[ds18b20_byte_cnt]=ds2482_rd_byte_result_1wire();
			ds18b20_byte_cnt++;
			if (ds18b20_byte_cnt<sizeof(ds18b20_buf)) //���� �� ��� ����� ���������
				ds18b20_step_st=DS18B20_SEND_READ; //������� � �������� ������� ������ �����
			else
				{
				if (crc8_1wire(ds18b20_buf, 8, 1)==ds18b20_buf[8]) //���� CRC ����������
					{
					ds18b20_temperature[ds18b20_chan]=ds18b20_calc_temp(ds18b20_buf);
					ds18b20_error[ds18b20_chan]=0;
					}
				else
					{if (ds18b20_error[ds18b20_chan]<0xFF) ds18b20_error[ds18b20_chan]++;}

				ds18b20_step_st=DS18B20_SEND_PRESET_CNV;
				}
			break;
			}
		case DS18B20_SEND_PRESET_CNV:
			{
			if (ds2482_send_present_1wire())
				{
				ds18b20_step_st=DS18B20_RESULT_PRESET_CNV;
				ds18b20_step_timeout=timers_get_finish_time(1); //���� 1 ��.
				ds18b20_wait=1; //���������� ���� ������������� �������� ���������� ��������
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_RESULT_PRESET_CNV:
			{
			if (ds2482_get_present_1wire())			//���� ���� ����� �� present pulse
				{
				ds18b20_step_st=DS18B20_SEND_SKIP_ROM_CNV;
				ds18b20_step_timeout=timers_get_finish_time(1); //���� 1 ��.
				}
			else
				{
				ds18b20_next_chan(); //����� ��������� �����
				ds18b20_step_st=DS18B20_SELECT_CHAN; //������� � ������ ���������� ������
				}
			break;
			}
		case DS18B20_SEND_SKIP_ROM_CNV:
			{
			if (ds2482_wr_1wire(DS18B20_SKIP_ROM))
				{
				ds18b20_step_st=DS18B20_SEND_START_CNV;
				ds18b20_step_timeout=timers_get_finish_time(1); //���� 1 ��.
				ds18b20_wait=1; //���������� ���� ������������� �������� ���������� ��������
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_SEND_START_CNV:
			{
			if (ds2482_wr_1wire(DS18B20_START_CNV))
				{
				ds18b20_next_chan(); //����� ��������� �����
				ds18b20_step_st=DS18B20_SELECT_CHAN;
				ds18b20_step_timeout=timers_get_finish_time(1); //���� 10 ��.
				ds18b20_wait=1; //���������� ���� ������������� �������� ���������� ��������
				}
			else ds18b20_cmd_retry();
			break;
			}
		}
}

/**
  * @brief  ������������ �������� ����������� �� ������� DS18B20 � �C
  *
  * @param  *buf: ��������� �� ����� � ������ �������, ������������ �� �������
  *
  * @retval �����������*1000 (m�C)
  */
static uint32_t ds18b20_calc_temp(uint8_t *buf)
{//uint16_t deg_val[16]={0,100,100,200,300,300,400,400,500,600,600,700,800,800,900,900};
uint16_t deg_val[16]={0,062,125,188,250,313,375,438,500,563,625,688,750,813,875,938};
int8_t dg, fl;
uint32_t temp;

	dg=(int8_t)((buf[1]<<4)|(buf[0]>>4));
	fl=buf[0]&0x0F;

	temp=(dg*1000)+deg_val[fl];

	return(temp);
}

/**
  * @brief  ���������� ��������� ������� �������� ������� �������: ����������� ������� ��������� ������� � ������������� �������� �� ��������� �������
  */
static void ds18b20_cmd_retry(void)
{
	ds18b20_fault++;
	ds18b20_step_timeout=timers_get_finish_time(10); //���� ������� �� ������, �� ����������� ����� 10 ��
}

/**
  * @brief  ����� ��������� ������ (��������� �����): ���� ��������� ������������������ ������ � ������������� �� ���� ��������� ds18b20_chan
  */
static void ds18b20_next_chan(void)
{uint8_t cnt;
	ds18b20_fault=0;
	for(cnt=(ds18b20_chan+1); cnt<DS18B20_MAX_CHANNELS; cnt++)
		{
		if (ds18b20_init_chan&(1<<cnt))
			{
			ds18b20_chan=cnt;
			return;
			}
		}
	for(cnt=0; cnt<ds18b20_chan; cnt++)
		{
		if (ds18b20_init_chan&(1<<cnt))
			{
			ds18b20_chan=cnt;
			return;
			}
		}
	return;
}
#endif