/*
 * timers.c
 *
 *  Created on: 4 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "timers.h"

/**
  * @brief  ��������� �������� ���������� ������� ��� �������� �������� �������� ������� ��������
  *
  * @param  delay: �������� ����� �������� � ��.
  *
  * @retval �������� ���������� ������� ��� ���������� �������� ��������
  */
stime_t timers_get_finish_time(uint32_t delay)
{
	stime_t time;

	time.run = HAL_GetTick();
	time.del = delay;
	return time;
}
/**
  * @brief  ��������� ������� �������� ������� �� ���������� ��������� �������� ���������� �������
  *
  * @param  time: �������� ���������� �������, �������� �������� ���������� �������
  *
  * @retval �������� � ��, ������ �������� �� ���������� ��������� �������
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
