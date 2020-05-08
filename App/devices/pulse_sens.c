/*
 * pulse_sens.c
 *
 *  Created on: 28 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "pulse_sens.h"
#include "pulse_in.h"

static uint8_t pulse_sens_chan[PULSE_IN_NUMBER]={0,0,0}; //номер канала к которому подключен  импульсный датчик
static uint16_t pulse_sens_pl[PULSE_IN_NUMBER];	//количество импульсов на единицу измерения

/**
  * @brief  Инициализировать импульсный датчик
  *
  * @param  sens: номер датчика от 1 до PULSE_IN_NUMBER
  * 		chan: номер импульсного входа на котором установлен датчик (от 1 до PULSE_IN_NUMBER)
  *
  * @retval температура*1000 (m°C)
  */
void pulse_sens_init(uint8_t sens, uint8_t chan)
{
	if (sens==0) sens=1;
	if (sens>PULSE_IN_NUMBER) sens=PULSE_IN_NUMBER;
	sens--;

	pulse_sens_pl[sens]=PULSE_SENS_PL;

	if (chan==0) chan=1;
	if (chan>PULSE_IN_NUMBER) chan=PULSE_IN_NUMBER;
	pulse_sens_chan[sens]=chan;
}

/**
  * @brief  Возвращает значение испульсного датчика
  *
  * @param  sens: номер датчика от 1 до PULSE_IN_NUMBER
  *
  * @retval значение единиц*1000/минуту (например для датчика топлива: мл/мин)
  */
int32_t pulse_sens_get_val(uint8_t sens)
{int64_t pulse;

	if (sens==0) sens=1;
	if (sens>PULSE_IN_NUMBER) sens=PULSE_IN_NUMBER;
	sens--;

	pulse=pulse_in_min(pulse_sens_chan[sens]);
	pulse*=1000;
	if (pulse_sens_pl[sens]!=0)
		pulse/=pulse_sens_pl[sens];
	return(pulse);
}

/**
  * @brief  Возвращает значение импульсов на единицу измерения
  *
  * @param  sens: номер датчика от 1 до PULSE_IN_NUMBER
  *
  * @retval количество импульсов на единицу измерения (например, импульсов на литр для датчиков СРТ)
  */
uint16_t pulse_sens_get_pl(uint8_t sens)
{
	if (sens==0) sens=1;
	if (sens>PULSE_IN_NUMBER) sens=PULSE_IN_NUMBER;
	sens--;

	return(pulse_sens_pl[sens]);
}

/**
  * @brief  Устаналвивает значение импульсов на единицу измерения
  *
  * @param  sens: номер датчика от 1 до PULSE_IN_NUMBER
  * 		pl: количество импульсов на единицу измерения (например, импульсов на литр для датчиков СРТ)
  */
void pulse_sens_set_pl(uint8_t sens, uint16_t pl)
{
	if (sens==0) sens=1;
	if (sens>PULSE_IN_NUMBER) sens=PULSE_IN_NUMBER;
	sens--;

	pulse_sens_pl[sens]=pl;
}
