/*
 * rtc_sens.h
 *
 *  Created on: 24 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DEVICES_RTC_SENS_H_
#define APP_DEVICES_RTC_SENS_H_
#include "types.h"

uint32_t rtc_sens_get_time(void); //���������� �������� �������� �������
uint32_t rtc_sens_get_date(void); //���������� �������� ������� ����
void rtc_sens_set_datetime(uint32_t date, uint32_t time);//���������� �������� ���� � �������

#endif /* APP_DEVICES_RTC_SENS_H_ */