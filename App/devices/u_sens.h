/*
 * u_sens.h
 *
 *  Created on: 3 февр. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DEVICES_U_SENS_H_
#define APP_DEVICES_U_SENS_H_
#include "types.h"

#define U_SENS1_R1 31000	//верхнее сопротивление делителя напряжения, Ом
#define U_SENS1_R2 3000		//нижнее сопротивление делителя напряжения, Ом
#define U_SENS1_ADC 13		//канал АЦП на котором установлен датчик напряжения

int32_t u_sens_get_val(void);	//Получить данные датчика напряжения
int32_t u_sens_get_k(void);		//Получить значения коэффициента K
int32_t u_sens_get_b(void);		//Получить значения коэффициента B
uint8_t u_sens_get_d(void);		//Получить значения коэффициента D
void u_sens_set_k(int32_t val);	//Установить значение коэффициента K
void u_sens_set_b(int32_t val);	//Установить значение коэффициента B
void u_sens_set_d(uint8_t val);	//Установить значение коэффициента D

#endif /* APP_DEVICES_UI_SENS_H_ */
