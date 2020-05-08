/*
 * timers.h
 *
 *  Created on: 4 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DRIVERS_TIMERS_H_
#define APP_DRIVERS_TIMERS_H_
#include "stm32f4xx_hal.h"

typedef struct {
	uint32_t del;
	uint32_t run;
} stime_t;

stime_t timers_get_finish_time(uint32_t delay); //Вычисляет значение системного таймера при значении которого истекает заданая задержка
uint32_t timers_get_time_left(stime_t stime); //Вычисляет сколько осталось времени до достижения заданного значения системного таймера

#endif /* APP_DRIVERS_TIMERS_H_ */
