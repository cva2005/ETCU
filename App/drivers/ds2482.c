/*
 * ds2482.c
 *
 *  Created on: 7 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "ds2482.h"

extern I2C_HandleTypeDef DS2482_I2C; //указатель на I2C

static uint8_t ds2482_channel=0;	//текущий выбранный канал
static ds2482_resp_t ds2482_resp;	//регистр статуса
static uint8_t ds2482_data[2]={0,0};	//массив для фармирования пакета на отправку
static uint8_t ds2482_error=0;			//счётчик неудачных попыток отправки/приёма

/**
  * @brief  Инициализция микросхемы DS2482
  */
void ds2482_init(void)
{uint8_t result[9], cnt;

	ds2482_error=0;
	ds2482_resp.byte=0;
	ds2482_resp.sr.WB=1;

	ds2482_data[0]=DS2482_DRST;
	if (!ds2482_wr_repeat(ds2482_data, 1, DS2482_RAPEAT_CNT)) return;
	if (!ds2482_rd_repeat(&ds2482_resp.byte, 1, DS2482_RAPEAT_CNT)) return;
	ds2482_channel=1; //после сброса устаналивается канал 1 автоматически

	ds2482_data[0]=DS2482_WCFG;
	ds2482_data[1]=DS2482_CFG_APU;
	if (!ds2482_wr_repeat(ds2482_data, 2, DS2482_RAPEAT_CNT)) return;
	if (!ds2482_rd_repeat(ds2482_data, 1, DS2482_RAPEAT_CNT)) return;
	return;
}

/**
  * @brief  Изменяет рабочий канал микросхемы DS2482
  *
  * @param  chan: номер канала от 1 до 8
  *
  * @retval текущий выбранный канал
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
  * @brief  Читает состояние регистра статуса DS18B20
  *
  * @retval сосотяние бита 1WB: 1-линя 1-wire занята, 0-линия 1-wire свободна
  */
uint8_t ds2482_get_busy(void)
{
	if (!ds2482_rd_repeat(&ds2482_resp.byte, 1, 1)) ds2482_resp.sr.WB=1;
	return (ds2482_resp.sr.WB);
}

/**
  * @brief  Формирует present импульс в линию 1-wire
  *
  * @retval результат отправки: 1-импульс отправлен, 0-ошибка от правки импульса
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
  * @brief  Возвращает ответ на последний present импульс: данные дейтвительны только после ds2482_get_busy()==0
  *
  * @retval результат отправки: 1-импульс отправлен, 0-ошибка от правки импульса
  */
uint8_t ds2482_get_present_1wire(void)
{
	return (ds2482_resp.sr.PPD);
}

/**
  * @brief Передать 1 байт по 1-wire
  *
  * @param  data: байт данных для передачи
  *
  * @retval результат отправки: 1-данные переданы, 0-ошибка передачи
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
  * @brief Команда: начать чтение одного бита данных из 1-wire
  *
  * @retval результат: 1-чтение начато, 0-ошибка начала чтения
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
  * @brief Возвращает результат чтения одного бита данных из 1-wire: данные действительны только если ds2482_get_busy()==0
  *
  * @retval результат чтения: 0bxxxxxxxV (младший бит)
  */
uint8_t ds2482_rd_bit_result_1wire(void)
{
	return(ds2482_resp.sr.SBR);
}

/**
  * @brief Команда: начать чтение байта данных из 1-wire
  *
  * @retval результат: 1-чтение начато, 0-ошибка начала чтения
  */
uint8_t ds2482_rd_byte_cmd_1wire(void)
{
	ds2482_data[0]=DS2482_1WRB;
	if (!ds2482_wr_repeat(ds2482_data, 1, 1)) return(0);
	ds2482_resp.sr.WB=1;
	return(1);
}

/**
  * @brief Возвращает результат чтения байта данных из 1-wire: данные действительны только если ds2482_get_busy()==0
  *
  * @retval прочитанные данные
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
  * @brief  Отправляет микросхеме DS2482 заданное количество байт, в случае ошибки передачи, повторяет отправку заданное количество раз
  *
  * @param  *data: указатель на буфер с данными для отправки
  * 		size: колчиество данных для отправки
  * 		repeat: количество попыток отправки, в случае ошибок передачи
  *
  * @retval результат оправки: 1-данные успешно отправлены; 0-данные не удалось отправить
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
  * @brief Читает из микросхемы DS2482 заданное количество байт, в случае ошибки чтения, повторяет чтение заданное количество раз
  *
  * @param  *data: указатель на буфер для чтения данных
  * 		size: колчиество данных для чтения
  * 		repeat: количество попыток чтения, в случае ошибок
  *
  * @retval результат чтения: 1-данные успешно прочитаны; 0-данные не удалось прочитать
  */
static uint8_t ds2482_rd_repeat(uint8_t* data, uint8_t size, uint8_t repeat)
{HAL_StatusTypeDef res;

	if (ds2482_error>=DS2482_MAX_ERR) return(0);
	do
		{
		res=HAL_I2C_Master_Receive(&DS2482_I2C, DS2482_ADRESS_7BIT(DS2482_ADRESS), data, size, 10);  //чтение состояния
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
