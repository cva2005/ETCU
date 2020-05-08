/*
 * i_sens.h
 *
 *  Created on: 3 февр. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DEVICES_I_SENS_H_
#define APP_DEVICES_I_SENS_H_
#include "types.h"

//#define I_HARD_CALC //определить, если нет возможности посчитать напряжение на АЦП при номинальном токе шунта

#ifndef I_HARD_CALC
#define I_SENS1_SHUNT 100000	//значение номинального тока на шунте в мА
#define I_SENS1_ADC_U 3289		//значение нанпряжения в мВ на АЦП при номинальном токе
#define I_SENS1_ADC 12			//канал АЦП на котором установлен датчик тока

#define I_SENS2_SHUNT 100000	//значение номинального тока на шунте в мА
#define I_SENS2_ADC_U 3214		//значение нанпряжения в мВ на АЦП при номинальном токе
#define I_SENS2_ADC 11			//канал АЦП на котором установлен датчик тока

#else
#define I_SENS1_SHUNT_I 100000	//значение тока на шунте в мА
#define I_SENS1_SHUNT_U 75		//значение напряжения на шунте при указанном токе мВ
#define I_SENS1_R_IN 3500		//входное сопротивление усилителя, Ом
#define I_SENS1_R_FB 150000		//сопротивление в обратной связи, Ом
#define I_SENS1_ADC 12			//канал АЦП на котором установлен датчик тока
//#define I_SENS1_INVERT			//если определн, то усилитель инвертирующий

#define I_SENS2_SHUNT_I 100000	//значение тока на шунте в мА
#define I_SENS2_SHUNT_U 75		//значение напряжения на шунте при указанном токе мВ
#define I_SENS2_R_IN 3500		//входное сопротивление усилителя, Ом
#define I_SENS2_R_FB 150000		//сопротивление в обратной связи, Ом
#define I_SENS2_ADC 11			//канал АЦП на котором установлен датчик тока
#define I_SENS2_INVERT			//если определн, то усилитель инвертирующий
#endif

int32_t i_sens_get_val(uint8_t chan);	//Получить данные датчика напряжения
int32_t i_sens_get_k(uint8_t chan);		//Получить значения коэффициента K
int32_t i_sens_get_b(uint8_t chan);		//Получить значения коэффициента B
uint8_t i_sens_get_d(uint8_t chan);		//Получить значения коэффициента D
void i_sens_set_k(uint8_t chan, int32_t val);	//Установить значение коэффициента K
void i_sens_set_b(uint8_t chan, int32_t val);	//Установить значение коэффициента B
void i_sens_set_d(uint8_t chan, uint8_t val);	//Установить значение коэффициента D
static uint8_t i_sens_get_element(uint8_t adc); //Возвращает номер элемента в массиве, для данного канала АЦП


#endif /* APP_DEVICES_I_SENS_H_ */
