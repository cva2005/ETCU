/*
 * u_sens.c
 *
 *  Created on: 3 февр. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "u_sens.h"
#include "adc.h"
/* Расчёт K
 Uвх=Uацп*((R1+R2)/R2), K=(R1+R2)/R2;
 */
static int32_t u_sens1_k=(U_SENS1_R1+U_SENS1_R2)*1000/U_SENS1_R2;
static int32_t u_sens1_b=0;
static uint8_t u_sens1_d=6; //т.к. при расчёте Uацп в мВ (3 нуля) и ещё умножили на 1000 числитель (U_SENS1_R1+U_SENS1_R2) (ещё 3 нуля)

/**
  * @brief  Получить данные датчика напряжения
  *
  * @retval значение датчика напряжения в мВ
  */
int32_t u_sens_get_val(void)
{
	return(adc_get_calc(U_SENS1_ADC, u_sens1_k, u_sens1_b, u_sens1_d, 3));
}

/**
  * @brief  Получить значения коэффициента K
  *
  * @retval K
  */
int32_t u_sens_get_k(void)
{
	return(u_sens1_k);
}

/**
  * @brief  Получить значения коэффициента B
  *
  * @retval B
  */
int32_t u_sens_get_b(void)
{
	return(u_sens1_b);
}

/**
  * @brief  Получить значения коэффициента D
  *
  * @retval D
  */
uint8_t u_sens_get_d(void)
{
	return(u_sens1_d);
}

/**
  * @brief Установить значение коэффициента K
  *
  * @param  val: K
  */
void u_sens_set_k(int32_t val)
{
	if (val>0)
		u_sens1_k=val;
	else
		u_sens1_k=1;
}

/**
  * @brief Установить значение коэффициента B
  *
  * @param  val: B
  */
void u_sens_set_b(int32_t val)
{
	u_sens1_b=val;
}

/**
  * @brief Установить значение коэффициента D
  *
  * @param  val: D
  */
void u_sens_set_d(uint8_t val)
{
	u_sens1_d=val;
}
