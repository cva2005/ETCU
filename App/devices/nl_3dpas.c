/*
 * nl_3dpas.c
 *
 *  Created on: 31 янв. 2016 г.
 *      Author: Перчиц А.Н
 */
#include "nl_3dpas.h"
#include "modbus.h"
#include "timers.h"
#include <string.h>

nl_3dpas_rx_data_t nl_3dpas_rx_data; //данные принимаемые от датчика параметров атмосферы

uint8_t nl_3dpas_err_send;			//счётчик пакетов без ответа
uint8_t nl_3dpas_adr;				//адрес датчика NL-3DPAS
stime_t nl_3dpas_tx_time;			//время следующей отправки пакетов
uint8_t nl_3dpas_tx_type=NL_3DPAS_CONVERT;	//номер шага обмена: преобразование или чтения параметров

/**
  * @brief Инициализация датчика параметров  атмосферы
  *
  * @param  adr: адрес датчика
  */
void nl_3dpas_init(uint8_t adr)
{uint8_t cnt=0;
extern modbus_rx_t modbus_rx[MODBUS_MAX_DEV]; //указатели на функции обработчики пакетов от modbus

	nl_3dpas_err_send = 0;
	nl_3dpas_adr=0;
	nl_3dpas_tx_type=NL_3DPAS_CONVERT;

	if ((adr<=247)&&(adr>0))
		{
		while ((modbus_rx[cnt]!=NULL)&&(cnt<MODBUS_MAX_DEV))	cnt++; //найти свободный указатель
		if (cnt<MODBUS_MAX_DEV)
			{
			modbus_rx[cnt]=nl_3dpas_update_data; 	//указать обработчик принатых пакетов
			nl_3dpas_adr=adr;					  		//сохранить адрес
			nl_3dpas_tx_time=timers_get_finish_time(0);	//установить время отправки следующего пакета
			}
		}
}

/**
  * @brief  Обработчик пакетов принятых по ModBus от NL-3DPAS
  *
  * @param  *data: указатель на данные принятые по ModBus
  * 		len: длина принятых данных
  * 		adr: Адрес устйроства от которого пришли данные
  * 		function: номер функции
  */
void nl_3dpas_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function)
{uint16_t object;

	if (adr==nl_3dpas_adr) { //если адрес датчика
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
  * @brief  Шаг обработки данных датчика параметров атмосферы
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
  * @brief  Возвращает состояние связи с NL-3DPAS
  *
  * @retval состояние связи: 1-ошибка связи (нет данных от NL-3DPAS) 0-связь с NL-3DPAS исправна
  */
uint8_t nl_3dpas_err_link (void)
{
	if (nl_3dpas_err_send>NL_3DPAS_MAX_ERR_SEND) return(1);
	else return(0);
}

/**
  * @brief  Возвращает значение температуры
  *
  * @retval Температура:м°C (°C*1000)
  */
uint32_t nl_3dpas_get_temperature(void)
{
	return(nl_3dpas_rx_data.temperature*100);
}

/**
  * @brief  Возвращает значение атмосферного давления
  *
  * @retval Давление:мкм. рт.ст. (мм.рт.ст.*1000)
  */
uint32_t nl_3dpas_get_pressure(void)
{
	return(nl_3dpas_rx_data.pressure*100);
}

/**
  * @brief  Возвращает значение влажности
  *
  * @retval Влажность:м% (%*100).
  */
uint32_t nl_3dpas_get_humidity(void)
{
	return(nl_3dpas_rx_data.humidity*100);
}
