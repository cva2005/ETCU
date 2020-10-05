/*
 * timers.c
 *
 *  Created on: 4 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "timers.h"

/**
  * @brief  Вычисляет значение системного таймера при значении которого истекает заданая задержка
  *
  * @param  delay: заданное время задержки в мс.
  *
  * @retval значение системного таймера при достижении заданной задержки
  */
stime_t timers_get_finish_time(uint32_t delay)
{
	stime_t time;

	time.run = HAL_GetTick();
	time.del = delay;
	return time;
}
/**
  * @brief  Вычисляет сколько осталось времени до достижения заданного значения системного таймера
  *
  * @param  time: значение системного таймера, значение которого необходимо достичь
  *
  * @retval значение в мс, которе осталось до достижения заданного времени
  */
uint32_t timers_get_time_left(stime_t time)
{
	uint32_t del, t = HAL_GetTick();

 	if (t < time.run) { // overflow system timer
 		del = (0xffffffff - time.run) + t;
 	} else {
 		del = t - time.run;
 	}
 	if (del > time.del) return 0;
 	else return (time.del - del);
}

uint32_t timers_get_interval(uint32_t run) {
	uint32_t del, t = HAL_GetTick();
 	if (t < run) // overflow system timer
 		del = (0xffffffff - run) + t;
 	else del = t - run;
 	return del;
}
