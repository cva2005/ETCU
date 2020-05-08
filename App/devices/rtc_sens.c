/*
 * rtc_sens.c
 *
 *  Created on: 24 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "rtc_sens.h"
#include "rtc.h"
#include <string.h>

rtc_date_time_t rtc_sens; //переменная для чтения текущего времени и даты

/**
  * @brief  Возвращает значение текущего времени
  *
  * @retval значение времени: 0x00ЧЧММСС
  */
uint32_t rtc_sens_get_time(void)
{uint32_t time=0;

	rtc_sens=rtc_read();
	memcpy((uint8_t*)&time, (uint8_t*)&rtc_sens.sec, 3);

	return(time);
}

/**
  * @brief  Возвращает значение текущей даты
  *
  * @retval значение времени: 0x00ГГММДД
  */
uint32_t rtc_sens_get_date(void)
{uint32_t date=0;

	rtc_sens=rtc_read();
	memcpy((uint8_t*)&date, (uint8_t*)&rtc_sens.day, 3);

	return(date);
}

/**
  * @brief  Записывает значение даты и времени
  *
  * @param 	date:значение даты: 0x00ГГММДД
  * 		time:значение времени: 0x00ЧЧММСС
  */
void rtc_sens_set_datetime(uint32_t date, uint32_t time)
{
	memcpy((uint8_t*)&rtc_sens.day, (uint8_t*)&date, 3);
	memcpy((uint8_t*)&rtc_sens.sec, (uint8_t*)&time, 3);
	rtc_write(rtc_sens);
}
