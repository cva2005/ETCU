/*
 * flash.c
 *
 *  Created on: 20 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "flash.h"
#include "crc.h"
#include "types.h"

/**
  * @brief  ������� ���� ������ FLASH ������
  */
void flash_erase(void)
{FLASH_EraseInitTypeDef pEraseInit;
uint32_t SectorError;

	HAL_FLASH_Unlock();
	pEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;
	pEraseInit.Banks=FLASH_BANK_1;
	pEraseInit.Sector=FLASH_WORK_SECTOR;
	pEraseInit.NbSectors=1;
	pEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3;
	HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
	HAL_FLASH_Lock();
}

/**
  * @brief  ������ ������� flash ������
  *
  * @param  *data: ��������� �� ����� ���� ��������� ������
  * 		adr: ����� flash-������ ������ �������� ������
  * 		len: ���������� ����, ������� ���� ���������
  */
void flash_read(uint8_t* data, uint32_t adr, uint32_t len)
{uint32_t cnt;
udata32_t d;

	for (cnt=0; cnt<len; cnt+=4)
		{
		d.dword=(*(__IO uint32_t*) (adr+cnt));
		((uint8_t*)data)[cnt]=d.byte[0];
		if (len>(cnt+1)) ((uint8_t*)data)[cnt+1]=d.byte[1];
		if (len>(cnt+2)) ((uint8_t*)data)[cnt+2]=d.byte[2];
		if (len>(cnt+3)) ((uint8_t*)data)[cnt+3]=d.byte[3];
		}
}

/**
  * @brief  ���������� ������� flash ������: FLASH ������ ���� �������������� ������������� ��� ������
  *
  * @param  *data: ��������� �� ������ ��� ������
  * 		adr: ����� flash-������ ���� �������� ������
  * 		len: ���������� ���� ������, ������� ���� ��������
  */
void flash_write(uint8_t* data, uint32_t adr, uint32_t len)
{uint32_t cnt;

	for (cnt=0; cnt<len; cnt++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (adr+cnt), data[cnt]);
}

/**
  * @brief  � ������ ������� ���� ��������� ����� ��� ������
  *
  * @param  *data: ��������� �� ���������� ���� ��������� ����� ��������� �����
  *
  * @retval ����� flash ������ � ������� ��������� ������
  */
uint32_t flash_serch_free_mem(void)
{uint32_t cnt, max_adr, adr;
uint8_t finish;
flash_record_header_t record_header;

	adr=FLASH_BASE+FLASH_SECTOR_SHIFT;
	max_adr=adr+(FLASH_SECTOR_SIZE*1024);

	do
		{
		for (cnt=0; cnt<sizeof(record_header); cnt+=4)
			record_header.dword[cnt>>2]=(*(__IO uint32_t*) (adr+cnt));//flash_read(*adr+cnt);

		finish=1;
		for (cnt=0; cnt<(sizeof(record_header)>>2); cnt++)
			{if (record_header.dword[cnt]!=0xFFFFFFFF) finish=0;}
		if (finish) return(adr);
		if ((record_header.f.reserv==0x0000)&&(record_header.f.correct==FLASH_SYMBOL_CORRECT))
			adr+=record_header.f.size;
		else
			adr+=2;
		}
	while(adr<max_adr);

	return(0);
}
/**
  * @brief  ���� ���������� ������ � �������� ID
  *
  * @param  id: ������������� ������ (��� ������)
  * 		*record_header: ��������� �� ��������� ��������� � ������� ��������� ��������� ��������� ������
  *
  * @retval ����� flash ������ � ������� ��������� ������
  */
uint32_t flash_serch_last_rec(uint8_t id, flash_record_header_t* record_header)
{uint32_t cnt, adr, adr_rec=0, max_adr;
uint8_t finish, record_found=0;
flash_record_header_t d;

	adr=FLASH_BASE+FLASH_SECTOR_SHIFT;
	max_adr=adr+(FLASH_SECTOR_SIZE*1024);

	do
		{
		for (cnt=0; cnt<sizeof(flash_record_header_t); cnt+=4)	d.dword[cnt>>2]=(*(__IO uint32_t*) (adr+cnt));//flash_read(adr+cnt);
		finish=1;
		for (cnt=0; cnt<(sizeof(record_header)>>2); cnt++)
			{if (d.dword[cnt]!=0xFFFFFFFF) finish=0;}
		if (finish)
			{
			if (adr_rec) return(adr_rec);
			else return(0);
			}
		else
			{
			if ((d.f.reserv==0x0000)&&(d.f.correct==FLASH_SYMBOL_CORRECT))
				{
				if (d.f.id==id)
					{
					adr_rec=adr;
					for (cnt=0; cnt<(sizeof(flash_record_header_t)>>2); cnt++) record_header->dword[cnt]=d.dword[cnt];
					}
				adr+=d.f.size;
				}
			else
				adr+=2;
			}
		}
	while(adr<max_adr);

	return(0);
}

/**
  * @brief  ������ ������ �� flash ������ (��� ���������)
  *
  * @param  *data: ��������� �� ����� ���� ���� ��������� ������
  * 		len: ������ ������
  * 		id: ������������� ������ (��� ������), ������� ���� ���������
  *
  * @retval ���������� ����������� ������
  */
uint16_t flash_data_rd(void* data, uint16_t len, uint8_t id)
{uint32_t adr, crc;
flash_record_header_t record_header;

	adr=flash_serch_last_rec(id, &record_header);
	if (adr)
		{
		if ((record_header.f.size-sizeof(flash_record_header_t))<len) len=record_header.f.size-sizeof(flash_record_header_t);

		flash_read((uint8_t*)data, (adr+sizeof(flash_record_header_t)), len);

		crc=crc32_ether(((uint8_t*)data), len, 1);
		if (crc!=record_header.f.crc) return(0);
		else return(1);
		}
	else
		return(0);
}

/**
  * @brief  ��������� ������ �� flash ������
  *
  * @param  *data: ��������� �� ������ ������� ���� ��������
  * 		len: ������ ������������ ������
  * 		id: ������������� ������ (��� ������)
  *
  * @retval ���������� ���������� ����� (��� ���������)
  */
uint16_t flash_data_wr(void* data, uint16_t len, uint8_t id)
{uint32_t adr, shift;
flash_record_header_t record_header;

	adr=flash_serch_free_mem();
	if (!adr) return(0);

	if (len%2) len++;

	if ((adr+len+sizeof(flash_record_header_t))>=						//���� ������ �� ���������� � ������
		(FLASH_BASE+FLASH_SECTOR_SHIFT+(FLASH_SECTOR_SIZE*1024)) )
		return(0);

	record_header.f.correct=FLASH_SYMBOL_CORRECT;
	record_header.f.crc=crc32_ether((int8_t*)data, len, 1);
	record_header.f.id=id;
	record_header.f.reserv=0x0000;
	record_header.f.time=rtc_read();
	record_header.f.size=len+sizeof(flash_record_header_t);

	HAL_FLASH_Unlock();
	flash_write(record_header.byte, adr, sizeof(record_header)); //�������� ���������
	flash_write((int8_t*)data, adr+sizeof(record_header), len);  //�������� ������
	HAL_FLASH_Lock();

	return(len);
}

/**
  * @brief  ���������� CRC ��������� ������ � ��������� ID
  *
  * @param  id: ������������� ������
  *
  * @retval CRC
  */
uint32_t flash_get_crc(uint8_t id)
{
	flash_record_header_t record_header;

	if (flash_serch_last_rec(id, &record_header))
		return(record_header.f.crc);
	else
		return(0);
}