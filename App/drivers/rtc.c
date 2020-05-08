/*
 * rtc.c
 *
 *  Created on: 7 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "rtc.h"

extern RTC_HandleTypeDef hrtc;

/**
  * @brief  Возвращает текущее значение даты и времени
  *
  * @retval дата и время в формате rtc_data_time_t
  */
rtc_date_time_t rtc_read(void)
{RTC_TimeTypeDef time;
 RTC_DateTypeDef date;
 rtc_date_time_t dt;

	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_MODE);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_MODE);

	dt.sec=time.Seconds;
	dt.min=time.Minutes;
	dt.hour=time.Hours;
	dt.day=date.Date;
	dt.month=date.Month;
	dt.year=date.Year;

	return (dt);
}

/**
  * @brief  Устанавливает заданное значение часов реального времени
  *
  * @param  dt: дата и время в формате rtc_data_time_t
  */
void rtc_write(rtc_date_time_t dt)
{RTC_TimeTypeDef sTime;
 RTC_DateTypeDef sDate;

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_MODE);
	sTime.Hours = dt.hour;
	sTime.Minutes = dt.min;
	sTime.Seconds = dt.sec;
	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_MODE);

	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_MODE);
	sDate.Month = dt.month;
	sDate.Date = dt.day;
	sDate.Year = dt.year;
	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_MODE);
}
