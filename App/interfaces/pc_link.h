/*
 * pc_link.h
 *
 *  Created on: 11 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_INTERFACES_PC_LINK_H_
#define APP_INTERFACES_PC_LINK_H_
#include "types.h"

#define PC_LINK_REV 1000				//начальная версия протокола (без учёта данных)

#define PC_LINK_MIN_TX_TIME 30 			//минимальный таймаут между отправками пакетов
#define PC_LINK_MAX_RX_TIMEOUT 1000 	//максимально допустимый интервал между приёмами пакетов
#define PC_LINK_MAX_RX_PACK_TIME 30		//маскимально-допустимое время приёма одного пакета

#define PC_LINK_BUF_SIZE 1500			//размер буфера на прём/передачу (должен быть не меньше максимального размера пакета)

#define PC_LINK_LOSS 0  				//нет данных от ПК
//-----------------------------ПРОТОКОЛ ОБМЕНА С ПК--------------------
#pragma pack(1)
typedef struct
	{
	uint8_t start;
	uint16_t session;
	uint16_t number;
	uint16_t version;
	uint8_t type;
	uint16_t size;
	}header_t;
typedef union
	{
	 struct
		{
		header_t header;
		uint8_t data[PC_LINK_BUF_SIZE-sizeof(header_t)];
		}fld;
	uint8_t byte[PC_LINK_BUF_SIZE];
	uint16_t word[PC_LINK_BUF_SIZE>>1];
	uint32_t dword[PC_LINK_BUF_SIZE>>2];
	}pc_link_pack_t; //формат пакета
#pragma pack()

void pc_link_init(void);									//Инициализация модуля обмена с ПК
void pc_link_reset(void);									//Сброс модуля обмена с ПК (переинициализация)
uint8_t pc_link_get_rx_mode(void);							//Возвращает состояние режима обмена для последних принятых данных
uint16_t pc_link_get_pc_session(void);						//Возвращает номер сессии для последних принятых данных
//uint8_t pc_link_get_rev(void);							//
uint16_t pc_link_read_data(void *buf, uint16_t size);		//Прочитать последний принятый пакет с верным CRC
uint16_t pc_link_write_data(uint8_t mode, uint16_t session, uint8_t *p, uint16_t size);	//Записать данные для отправки в очередь ожидания
void pc_link_set_rev(uint16_t rev);	//Устанавливает занчение поля "версия протокола", версия протокола зависит от типа данных, поэтому устанавливается внешней функцией
uint8_t pc_link_tx_ready(void);								//Определяет состояние готовности модуля отправки данных на ПК
void pc_link_step(void);				//шаг обмена;
static void pc_link_rx_data(void);		//Обработчик принятых пакетов: определяет все ли данные приняты, данные корректны и корректна ли CRC
static void pc_link_tx_data(void);		//Формирует заголовок пакета и отправляет данные из буфера (записанные pc_link_write_data)

#endif /* APP_INTERFACES_PC_LINK_H_ */
