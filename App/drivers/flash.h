/*
 * flash.h
 *
 *  Created on: 20 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DRIVERS_FLASH_H_
#define APP_DRIVERS_FLASH_H_
#include "stm32f4xx_hal.h"
#include "rtc.h"

#define FLASH_WORK_SECTOR FLASH_SECTOR_11	//����� �������
#define FLASH_SECTOR_SHIFT 0x000E0000		//�������� ���������� ������� � ������
#define FLASH_SECTOR_SIZE	128				//������ �������: �����

#define FLASH_SYMBOL_CORRECT 0xAA			//���� �������� ������������ ���������

#pragma pack(1)
typedef union
	{
	struct
		{
		uint16_t size; //������ ����� (������), ������� ���������
		uint8_t correct; //���� �������� �� ������������ ���������
		uint8_t id;		 //������������� ������
		uint16_t reserv;
		uint32_t crc;	//����������� ����� ����� ������ (��� ���������)
		rtc_date_time_t time;	//����� ������
		}f;
	uint8_t byte[16];
	uint16_t word[8];
	uint32_t dword[4];
	uint64_t qword[2];
	}flash_record_header_t; //��������� ������ �� flash ������
#pragma pack()

void flash_erase(void);											//������� ���� ������ FLASH ������
void flash_read(uint8_t* data, uint32_t adr, uint32_t len);		//������ ������� flash ������
void flash_write(uint8_t* data, uint32_t adr, uint32_t len);	//���������� ������� flash ������: FLASH ������ ���� �������������� ������������� ��� ������
uint32_t flash_serch_free_mem(void);							//� ������ ������� ���� ��������� ����� ��� ������
uint32_t flash_serch_last_rec(uint8_t id, flash_record_header_t* record_header);//���� ���������� ������ � �������� ID
uint16_t flash_data_rd(void* data, uint16_t len, uint8_t id);	//������ ������ �� flash ������
uint16_t flash_data_wr(void* data, uint16_t len, uint8_t id);	//��������� ������ �� flash ������
uint32_t flash_get_crc(uint8_t id);								//���������� CRC ��������� ������ � ��������� ID

#endif /* APP_DRIVERS_FLASH_H_ */