/*
 * rtc.h
 *
 *  Created on: 7 ���. 2016 �.
 *      Author: ������. �.�.
 */

#ifndef APP_DRIVERS_RTC_H_
#define APP_DRIVERS_RTC_H_
#include "stm32f4xx_hal.h"

#define RTC_FORMAT_MODE RTC_FORMAT_BIN //������ ������ ��� ������ �����: RTC_FORMAT_BIN ��� RTC_FORMAT_BCD

typedef struct
	{
	uint8_t sec;	//�������
	uint8_t min;	//������
	uint8_t hour;	//����
	uint8_t day;	//����
	uint8_t month;	//�����
	uint8_t year;	//���
	}rtc_date_time_t;

rtc_date_time_t rtc_read(void);	//���������� ������� �������� ���� � �������
void rtc_write(rtc_date_time_t dt); //������������� �������� �������� ����� ��������� �������

#endif /* APP_DRIVERS_RTC_H_ */