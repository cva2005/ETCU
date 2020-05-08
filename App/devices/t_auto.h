/*
 * t_auto.h
 *
 *  Created on: 28 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DEVICES_T_AUTO_H_
#define APP_DEVICES_T_AUTO_H_
#include "types.h"

#define T_AUTO_TABLE 1 //таблица зависимостей сопротивления от температуры: 0 - минимальные значения, 1 - среднее 2 - максимальные

int32_t t_auto_get_r(uint8_t chan);		//Возвращает значение сопротивления термодатчика R2=UвыхR1/(Uвх-Uвых)
int32_t t_auto_convert_r_to_val(int32_t r); //Пересчитывает сопротивление датчика температуры в °C
int32_t t_auto_get_val(uint8_t chan);	//Возвращает значение стемпературы

#define T_AUTO_R1 3000	//сопротивление R1 (верхнее) делителя напряжения для термодатчика

#endif /* APP_DEVICES_T_AUTO_H_ */
