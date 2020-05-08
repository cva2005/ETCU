/*
 * pulse_sens.h
 *
 *  Created on: 28 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DEVICES_PULSE_SENS_H_
#define APP_DEVICES_PULSE_SENS_H_
#include "types.h"

#define PULSE_SENS_TYPE_CPT 5 //определяет тип импульсного датчика: тип импульсного датчика "СРТ-Х": 5 (СРТ-5) или 8 (СРТ-8)

void pulse_sens_init(uint8_t sens, uint8_t chan);	//Инициализировать импульсный датчик
int32_t pulse_sens_get_val(uint8_t sens);		//Возвращает значение импульсного датчика в единицы*1000/минуту
uint16_t pulse_sens_get_pl(uint8_t sens);			//Возвращает значение импульсов на единицу измерения
void pulse_sens_set_pl(uint8_t sens, uint16_t pl);  //Устаналвивает значение импульсов на единицу измерения


#if PULSE_SENS_TYPE_CPT==5
	#define PULSE_SENS_PL 950 //для датчика СРТ-5: 950 импульсов на литр
#elif PULSE_SENS_TYPE_CPT==8
	#define PULSE_SENS_PL 200 //для датчика СРТ-8: 200 импульсов на литр
#else
	#define PULSE_SENS_PL 1
#endif
#endif /* APP_DEVICES_PULSE_SENS_H_ */
