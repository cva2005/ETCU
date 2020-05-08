/*
 * rtc_sens.h
 *
 *  Created on: 24 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DEVICES_RTC_SENS_H_
#define APP_DEVICES_RTC_SENS_H_
#include "types.h"

uint32_t rtc_sens_get_time(void); //Возвращает значение текущего времени
uint32_t rtc_sens_get_date(void); //Возвращает значение текущей даты
void rtc_sens_set_datetime(uint32_t date, uint32_t time);//Записывает значение даты и времени

#endif /* APP_DEVICES_RTC_SENS_H_ */
