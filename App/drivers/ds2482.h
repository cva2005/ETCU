/*
 * ds2482.h
 *
 *  Created on: 7 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DRIVERS_DS2482_H_
#define APP_DRIVERS_DS2482_H_
#include "stm32f4xx_hal.h"

#define DS2482_I2C hi2c1 //����� I2C � �������� ���������� ���������� DS2482

#define DS2482_ADRESS_AD 1 //����� ����������, ���������� ��������� ������� AD2, AD1, AD0

#define DS2482_RAPEAT_CNT 10	//���������� ������� �������� ������� DS2482 � ������ ������ I2C
#define DS2482_RAPEAT_DELAY 2	//������� ����� ��������� ������� � I2C � ������ ������
#define DS2482_MAX_ERR	15		//������������ ���������� ��������� ������� ������/������, ����� ���� ���������� ��������� �����������

void ds2482_init(void);						//������������ ���������� DS2482
uint8_t ds2482_set_chanel(uint8_t chan);	//�������� ������� ����� ���������� DS2482
uint8_t ds2482_get_busy(void);				//������ ��������� �������� ������� DS18B20 � ���������� ��������� ���� 1WB
uint8_t ds2482_send_present_1wire(void);	//��������� present ������� � ����� 1-wire
uint8_t ds2482_get_present_1wire(void);		//���������� ����� �� ��������� present �������: ������ ������������ ������ ����� ds2482_get_busy()==0
uint8_t ds2482_wr_1wire(uint8_t data);		//�������� 1 ���� �� 1-wire
uint8_t ds2482_rd_bit_cmd_1wire(void);		//�������: ������ ������ ������ ���� ������ �� 1-wire
uint8_t ds2482_rd_bit_result_1wire(void);	//���������� ��������� ������ ������ ���� ������ �� 1-wire: ������ ������������� ������ ���� ds2482_get_busy()==0
uint8_t ds2482_rd_byte_cmd_1wire(void);		//�������: ������ ������ ����� ������ �� 1-wire
uint8_t ds2482_rd_byte_result_1wire(void);	//���������� ��������� ������ ����� ������ �� 1-wire: ������ ������������� ������ ���� ds2482_get_busy()==0
static uint8_t ds2482_wr_repeat(uint8_t* data, uint8_t size, uint8_t repeat); //���������� ���������� DS2482 �������� ���������� ����, � ������ ������ ��������, ��������� �������� �������� ���������� ���
static uint8_t ds2482_rd_repeat(uint8_t* data, uint8_t size, uint8_t repeat); //������ �� ���������� DS2482 �������� ���������� ����, � ������ ������ ������, ��������� ������ �������� ���������� ���

#define DS2482_ADRESS_BASE 0x18	//������� ����� ���������� DS2482
#define DS2482_ADRESS (DS2482_ADRESS_BASE+DS2482_ADRESS_AD) //�������������� �����: ������� + ���������� ������� AD
#define DS2482_ADRESS_7BIT(__ADDRESS__) ((uint8_t)((__ADDRESS__) << 1)& (~I2C_OAR1_ADD0) ) //�������������� ������ � 7-�� ������

typedef union
	{
	struct
		{
		uint8_t WB: 1;	//1-wire ������
		uint8_t PPD: 1;	//����� �� present pulse
		uint8_t SD: 1;
		uint8_t LL: 1;
		uint8_t RST: 1;
		uint8_t SBR: 1;	//����� �� ������ ���������� ����
		uint8_t TSB: 1;
		uint8_t DIR: 1;
		}sr;
	uint8_t byte;
	}ds2482_resp_t; //��������� �������� �������

//---------------������� DS2482---------------
#define DS2482_DRST 0xF0	//����� DS2482: ���������� ���
#define DS2482_SRP 0xE1		//���������� ��������� �� �������: 1 �������� - ����� ��������
#define DS2482_WCFG 0xD2	//�������� ������������: 1 �������� - ���� ������������
#define DS2482_CHSL 0xC3	//������� �����: 1 �������� - ��� ������ ������
#define DS2482_1WRS 0xB4	//������������� Reset/Presence detect ���� �� ����� 1-Wire: ���������� ���
#define DS2482_1WWB 0xA5	//�������� ���� ������ � ����� 1-wire: 1 �������� - ���� ������
#define DS2482_1WRB 0x96	//��������� ���� ������ �� ����� 1-wire: ���������� ���
#define DS2482_1WSB 0x87	//1-wire ��������� ���: 1 �������� - ������� ����
#define DS2482_1WT 0x78		//������� 1-wire (2 ������ + 1 ������): 1 �������� - ���� ���������� ��� ������

//--------------��������� ������� SPR---------
#define DS2482_SRP_SR 0xF0		//������� �������
#define DS2482_SRP_RDR 0xE1		//������� ������ ������ �� 1-wire
#define DS2482_SRP_CHSL 0xD2	//������� ������ ������
#define DS2482_SRP_CFG 0xC3		//������� ������������
//--------------���� ������������ ��� ������� WCFG---------
#define DS2482_CFG_RESET 0xF0	//��������� ��������� �������� ������������
#define DS2482_CFG_APU	0xE1	//������: Active Pullup (APU)
#define DS2482_CFG_PPM	0xD2	//������:Presence Pulse Masking (PPM)
#define DS2482_CFG_SPU	0xB4	//������:Strong Pullup (SPU)
#define DS2482_CFG_1WS	0x78	//������:1-Wire Speed (1WS)
//--------------��� ������ ������ ��� ������� CHSL---------
#define DS2482_CHSL_IO0_WR 0xF0 //��� ������ ������ ������ 0
#define DS2482_CHSL_IO1_WR 0xE1 //��� ������ ������ ������ 1
#define DS2482_CHSL_IO2_WR 0xD2 //��� ������ ������ ������ 2
#define DS2482_CHSL_IO3_WR 0xC3 //��� ������ ������ ������ 3
#define DS2482_CHSL_IO4_WR 0xB4 //��� ������ ������ ������ 4
#define DS2482_CHSL_IO5_WR 0xA5 //��� ������ ������ ������ 5
#define DS2482_CHSL_IO6_WR 0x96 //��� ������ ������ ������ 6
#define DS2482_CHSL_IO7_WR 0x87 //��� ������ ������ ������ 7

#define DS2482_CHSL_IO0_RD 0xB8	//��� ������ ������ ������ 0
#define DS2482_CHSL_IO1_RD 0xB1	//��� ������ ������ ������ 1
#define DS2482_CHSL_IO2_RD 0xAA	//��� ������ ������ ������ 2
#define DS2482_CHSL_IO3_RD 0xA3	//��� ������ ������ ������ 3
#define DS2482_CHSL_IO4_RD 0x9C	//��� ������ ������ ������ 4
#define DS2482_CHSL_IO5_RD 0x95	//��� ������ ������ ������ 5
#define DS2482_CHSL_IO6_RD 0x8E	//��� ������ ������ ������ 6
#define DS2482_CHSL_IO7_RD 0x87	//��� ������ ������ ������ 7
//--------------������� ���� ��� ������� 1WSB � 1WT-------------
#define DS2482_BITBYTE_0 0x00	//��� = 0 ��� ������� ������ ����
#define DS2482_BITBYTE_1 0x80	//��� = 1 ��� ������� ������ ����

#endif /* APP_DRIVERS_DS2482_H_ */