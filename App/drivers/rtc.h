/*
 * rtc.h
 *
 *  Created on: 7 янв. 2016 г.
 *      Author: Перчиц. А.Н.
 */

#ifndef APP_DRIVERS_RTC_H_
#define APP_DRIVERS_RTC_H_
#include "stm32f4xx_hal.h"

#define RTC_FORMAT_MODE RTC_FORMAT_BIN //формат данных для работы часов: RTC_FORMAT_BIN или RTC_FORMAT_BCD

typedef struct
	{
	uint8_t sec;	//секунды
	uint8_t min;	//минуты
	uint8_t hour;	//часы
	uint8_t day;	//день
	uint8_t month;	//месяц
	uint8_t year;	//год
	}rtc_date_time_t;

rtc_date_time_t rtc_read(void);	//Возвращает текущее значение даты и времени
void rtc_write(rtc_date_time_t dt); //Устанавливает заданное значение часов реального времени

#endif /* APP_DRIVERS_RTC_H_ */
