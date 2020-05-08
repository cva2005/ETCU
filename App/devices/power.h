/*
 * power.h
 *
 *  Created on: 24 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DEVICES_POWER_H_
#define APP_DEVICES_POWER_H_
#include "types.h"

#define	POWER_5V_CHAN	ADC_CHAN_5	//канал АЦП для измерения напряжения питания
#define	POWER_VBAT_CHAN	ADC_VBAT	//канал АЦП для оцифровки напряжения батарейки
#define POWER_5V_R_UP	10000		//Верхний резистор делителя напряжения: Ом
#define POWER_5V_R_DW	10000		//Нижний резистор делителя напряжения: Ом

#define POWER_ACCURACY_K 1000		//множитель для увеличения точности вычислений (чем бьльше, тем точнее вычисление)

//void power_init(void);			//Инициализирует датчики питания: датчик напряжения 5В и датчик батарейки
uint32_t power_get_5v(void);	//Возвращает значения напряжения 5В
uint32_t power_get_vbat(void);	//Возвращает значения напряжения батарейки

#endif /* APP_DEVICES_POWER_H_ */
