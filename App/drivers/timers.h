/*
 * timers.h
 *
 *  Created on: 4 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DRIVERS_TIMERS_H_
#define APP_DRIVERS_TIMERS_H_
#include "stm32f4xx_hal.h"

typedef struct {
	uint32_t del;
	uint32_t run;
} stime_t;

stime_t timers_get_finish_time(uint32_t delay); //��������� �������� ���������� ������� ��� �������� �������� �������� ������� ��������
uint32_t timers_get_time_left(stime_t stime); //��������� ������� �������� ������� �� ���������� ��������� �������� ���������� �������

#endif /* APP_DRIVERS_TIMERS_H_ */