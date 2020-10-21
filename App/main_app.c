/*
 * main_app.c
 *
 *  Created on: 10 дек. 2015 г.
 *      Author: Перчиц А.Н.
 */
#include "main_app.h"
//#include "fatfs.h"
//#include "mxconstants.h"
#include "timers.h"
#include "rng.h"
#include "rtc.h"
#include "adc.h"
#include "dio.h"
#include "pulse_in.h"
#include "ds2482.h"
#include "rs485_1.h"
#include "rs485_2.h"
#include "wifi_hf.h"
#include "can_1.h"
#include "can_2.h"
#include "canopen.h"
#include "J1939.h"
#include "modbus.h"
#include "servotech_link.h"
#include "pc_link.h"
#include "bcu.h"
#include "atv61.h"
#include "pc_device.h"
#include "ds18b20.h"
#include "spsh20.h"
#include "nl_3dpas.h"
#include "mu110_6U.h"
#include "mv110_8A.h"
#include "t46.h"
#include "_control.h"
#include "timers.h"
#include "servo.h"
#include "smog.h"
#include "agm501.h"

stime_t led_blink_time;

void main_app (void) {
#ifdef HAL_IWDG_MODULE_ENABLED
	led_blink_time = timers_get_finish_time(LED_BLINK_TIME * 4);
	while (timers_get_time_left(led_blink_time));
	static IWDG_HandleTypeDef hiwdg;
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
	hiwdg.Init.Reload = 4095;
	HAL_IWDG_Init(&hiwdg);
	HAL_IWDG_Start(&hiwdg);
#endif
	adc_init();
#if ECU_CONTROL
	rs485_1_init(4800);
#else
	rs485_1_init(38400);
#endif
	rs485_2_init(115200);
	wifi_hf_init(115200);//wifi_hf_init(460800);
#ifdef SPSH_20_CONTROL
	can_1_init(CAN_1_SPEED_500K);
#elif ECU_CONTROL
	can_1_init(CAN_1_SPEED_250K);
#endif
	can_2_init(CAN_2_SPEED_250K);
	pulse_in_init(1);
	pulse_in_init(2);
	pulse_in_init(3);
	ds2482_init();
	modbus_init();
#ifdef SPSH_20_CONTROL
	servotech_link_init(1);
#endif
	CanOpen_init(2);
#if ECU_CONTROL
	canJ1939_init();
#elif SPSH_20_CONTROL
	pc_link_init();
#endif
	control_init();
	led_blink_time = timers_get_finish_time(LED_BLINK_TIME);
	while (1) {
		modbus_step();
#ifdef SPSH_20_CONTROL
		servotech_link_step();
#endif
		CanOpen_step();
#if ECU_CONTROL
		smog_step();
		agm_step();
		J1939_step();
		mv8a_step();
#ifdef ECU_CONTROL
		mu6u_step();
#endif
#elif SPSH_20_CONTROL
		pc_link_step();
		spsh20_step();
#elif SERVO_CONTROL
		servo_step();
#endif
		bcu_step();
#ifndef NO_FREQ_DRIVER
		atv61_step();
#endif
		pc_device_step();
#ifdef LOCAL_TEMP
		ds18b20_step();
#endif
#ifndef NO_3DPAS_DRIVER
		nl_3dpas_step();
#endif
#ifndef NO_TORQ_DRIVER
		t46_step();
#endif
		control_step();
		if (timers_get_time_left(led_blink_time) == 0) {
			HAL_GPIO_TogglePin(LED_MODE_GPIO_Port, LED_MODE_Pin);
			led_blink_time = timers_get_finish_time(LED_BLINK_TIME);
		}
#ifdef HAL_IWDG_MODULE_ENABLED
		HAL_IWDG_Refresh(&hiwdg);
#endif
	}
}
