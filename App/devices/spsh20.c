/*
 * spsh20.c
 *
 *  Created on: 26 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "spsh20.h"
#include "servotech_link.h"
#include "timers.h"
#include <string.h>

//--отправляемые данные--
spsh20_tx_data_t spsh20_tx_data; //формат для работы в режиме контурного управления
//--принимаемые данные--
spsh20_rx_data_t spsh20_rx_data; //формат для работы в режиме контурного управления
static int16_t spsh20_rx_i=0;
static uint16_t spsh20_rx_st=0;

static udata8_t spsh20_err;			//ошибка

static uint8_t spsh20_adr=0;		//адрес устйроства
static stime_t spsh20_connect_time;	//таймер ожидания ответов
static stime_t spsh20_tx_time;		//таймер отправки пакетов
static spsh20_tx_t  spsh20_tx_type = STATE_GET;


/**
  * @brief Инициализация сервопривода
  *
  * @param  адрес устйроства на шине servotech_link
  */
void spsh20_init(uint8_t adr)
{uint8_t cnt=0;
extern servotech_link_rx_t servotech_link_rx[SERVOTECH_LINK_MAX_DEV]; //указатели на функции обработчики пакетов от устройств CanOpen

	memset(&spsh20_tx_data.byte[0], 0, sizeof(spsh20_tx_data));
	memset(&spsh20_rx_data.byte[0], 0, sizeof(spsh20_rx_data));

	spsh20_err.byte = 0;
	spsh20_rx_i=0;
	spsh20_rx_st=0;

	if (adr<=6)
		{
		while ((servotech_link_rx[cnt]!=NULL)&&(cnt<SERVOTECH_LINK_MAX_DEV))	cnt++; //найти свободный указатель
		if (cnt<SERVOTECH_LINK_MAX_DEV)
			{
			servotech_link_rx[cnt]=spsh20_update_data; //указать обработчик принатых пакетов
			spsh20_adr=adr;				  //сохранить адрес
			spsh20_tx_time=timers_get_finish_time(SPSH20_DATA_TX_TIME); 	   //установить время отправки следующего пакета
			spsh20_connect_time=timers_get_finish_time(SPSH20_CONNECT_TIME); //Установить ограничение времени когда должен быть принят пакет PDO от slave устйроства
			}
		}
}

/**
  * @brief  Обработчик пакетов принятых по сервопривода
  *
  * @param  *data: указатель на данные
  * 		len: длина принятых данных
  * 		adr: адрес привода
  */
void spsh20_update_data (char *data, uint8_t len, uint32_t adr)
{uint16_t cmd;

	if (servotech_link_get_src_adr(adr)==spsh20_adr)
		{
		spsh20_connect_time=timers_get_finish_time(SPSH20_CONNECT_TIME); //установить счётчик ожидания подключения
		cmd=servotech_link_get_cmd(adr);
		if (cmd==CMD_ST_POS)
			{
			memcpy(spsh20_rx_data.byte, data, len);
			}
		if (cmd == CMD_RX_USB)
			{
			if (servotech_link_get_tech_cmd(data)==TECH_RSP)
				{
				if (servotech_link_get_pr(data) == PR_DD8_POS)
					memcpy(&spsh20_rx_data.f.pos, &data[3], 4);
				else if (servotech_link_get_pr(data) == PR_DD11_ST)
					memcpy(&spsh20_rx_st, &data[3], 1);
				}
			spsh20_tx_time = timers_get_finish_time(SPSH20_DATA_TX_RQ_TIME);
			}
		}
}

void spsh20_step(void) {
	if (timers_get_time_left(spsh20_tx_time) == 0) {
		if (spsh20_tx_type == STATE_GET) { // запросить сосотяние привода
			if (servotech_link_tx_tech(spsh20_adr, TECH_RQ, PR_DD11_ST, 0))
				spsh20_tx_type = POSITION_GET;
		} else if (spsh20_tx_type == POSITION_GET) { // запросить позицию привода
			if (servotech_link_tx_tech(spsh20_adr, TECH_RQ, PR_DD8_POS, 0))
				spsh20_tx_type = POSITION_SET;
		} else { // SPEED_SET
			if (servotech_link_tx_tech(spsh20_adr, TECH_SET, PR_CT3_POS, spsh20_tx_data.f.pos))
				spsh20_tx_type = STATE_GET;
		}
		spsh20_tx_time = timers_get_finish_time(SPSH20_DATA_TX_TIME); // время следующего запроса
	}
}

/**
  * @brief  Возвращает статус привода
  *
  * @retval Статус привода
  */
uint8_t spsh20_get_status (void)
{
	return(spsh20_rx_st);
}

/**
  * @brief  Возвращает позицию привода
  *
  * @retval Позиция
  */
int32_t spsh20_get_pos (void)
{
	return(spsh20_rx_data.f.pos);
}

/**
  * @brief  Возвращает состояние связи
  *
  * @retval состояние связи: 1-ошибка связи (нет данных от сервопривоа) 0-связь с сервоприводом исправна
  */
uint8_t spsh20_err_link (void)
{
	if (timers_get_time_left(spsh20_connect_time)==0) return(1);
	else return(0);
}

/**
  * @brief  Устанавливает значение позиции
  *
  * @param  data: Значение позиции
  */
void spsh20_set_pos (int32_t data)
{
	if (data > SPSH20_MAX_POSITION) data = SPSH20_MAX_POSITION;
	if (data < SPSH20_MIN_POSITION) data = SPSH20_MIN_POSITION;
	spsh20_tx_data.f.pos = data;
}
