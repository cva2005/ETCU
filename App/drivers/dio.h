/*
 * dio.h
 *
 *  Created on: 7 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DRIVERS_DIO_H_
#define APP_DRIVERS_DIO_H_
#include "stm32f4xx_hal.h"
#include "mxconstants.h"

#define  RELAY1 RELAY1_GPIO_Port, RELAY1_Pin
#define  RELAY2 RELAY2_GPIO_Port, RELAY2_Pin
#define  RELAY3 RELAY3_GPIO_Port, RELAY3_Pin
#define  RELAY4 RELAY4_GPIO_Port, RELAY4_Pin
#define  RELAY5 RELAY5_GPIO_Port, RELAY5_Pin
#define  RELAY6 RELAY6_GPIO_Port, RELAY6_Pin
#define  RELAY7 RELAY7_GPIO_Port, RELAY7_Pin
#define  RELAY8 RELAY8_GPIO_Port, RELAY8_Pin

uint8_t dio_out_read(void);			//Читает текущее сосотяние выходов
void dio_out_write(uint8_t data);	//Установливает заданное значение дискретных выходов

#endif /* APP_DRIVERS_DIO_H_ */
