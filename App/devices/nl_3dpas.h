/*
 * nl_3dpas.h
 *
 *  Created on: 31 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DEVICES_NL_3DPAS_H_
#define APP_DEVICES_NL_3DPAS_H_
#include "types.h"

#define NL_3DPAS_DATA_CONVERT_TIME 1300	//Врямя для измерения измерения параметров NL-3DPAS
#define NL_3DPAS_DATA_REINIT_TIME	200 //Время между чтением параметров и повторным перезапуском на преобравозание
#define NL_3DPAS_MAX_ERR_SEND	2		//Максимально допустимое количество запросов без ответа

void nl_3dpas_init(uint8_t adr);				//Инициализация датчика параметров  атмосферы
void nl_3dpas_step(void);						//Шаг обработки данных датчика параметров атмосферы
uint8_t nl_3dpas_err_link (void);				//Возвращает состояние связи с NL-3DPAS
uint32_t nl_3dpas_get_temperature(void);		//Возвращает значение температуры
uint32_t nl_3dpas_get_pressure(void);			//Возвращает значение атмосферного давления
uint32_t nl_3dpas_get_humidity(void);			//Возвращает значение влажности
void nl_3dpas_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function);	//Обработчик пакетов принятых по ModBus от NL-3DPAS

typedef struct
	{
	uint16_t temperature;
	uint16_t pressure;
	uint16_t humidity;
	}nl_3dpas_rx_data_t;

enum
	{
	NL_3DPAS_CONVERT, //шаг обработки данных - запустиьт преобразование
	NL_3DPAS_READ,	  //шаг обработки данных - запросить результат
	};

#endif /* APP_DEVICES_NL_3DPAS_H_ */
