/*
 * flash.h
 *
 *  Created on: 20 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DRIVERS_FLASH_H_
#define APP_DRIVERS_FLASH_H_
#include "stm32f4xx_hal.h"
#include "rtc.h"

#define FLASH_WORK_SECTOR FLASH_SECTOR_11	//номер сектора
#define FLASH_SECTOR_SHIFT 0x000E0000		//смещение указанного сектора в памяти
#define FLASH_SECTOR_SIZE	128				//размер сектора: кБайт

#define FLASH_SYMBOL_CORRECT 0xAA			//поле проверки корректности заголовка

#pragma pack(1)
typedef union
	{
	struct
		{
		uint16_t size; //размер блока (записи), включая заголовок
		uint8_t correct; //поле проверки на корректность заголовка
		uint8_t id;		 //идентификатор записи
		uint16_t reserv;
		uint32_t crc;	//контрольная сумма блока данных (без заголовка)
		rtc_date_time_t time;	//время записи
		}f;
	uint8_t byte[16];
	uint16_t word[8];
	uint32_t dword[4];
	uint64_t qword[2];
	}flash_record_header_t; //заголовок записи во flash памяти
#pragma pack()

void flash_erase(void);											//Очищает весь сектор FLASH памяти
void flash_read(uint8_t* data, uint32_t adr, uint32_t len);		//Читает область flash памяти
void flash_write(uint8_t* data, uint32_t adr, uint32_t len);	//Записывает область flash памяти: FLASH должна быть предварительно раблокирована для записи
uint32_t flash_serch_free_mem(void);							//В рамках сектора ищет свободное место для записи
uint32_t flash_serch_last_rec(uint8_t id, flash_record_header_t* record_header);//Ищет зпоследнюю запись с указаным ID
uint16_t flash_data_rd(void* data, uint16_t len, uint8_t id);	//Читает запись из flash памяти
uint16_t flash_data_wr(void* data, uint16_t len, uint8_t id);	//Формирует запись во flash памяти
uint32_t flash_get_crc(uint8_t id);								//Возвращает CRC последней записи с указанным ID

#endif /* APP_DRIVERS_FLASH_H_ */
