/*
 * i_sens.c
 *
 *  Created on: 3 февр. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "i_sens.h"
#include "adc.h"
#ifdef I_HARD_CALC
/* Расчёт К тока:
Вариант1:
Инвертирующий: Uацп=Uвх*(R_FB/R_IN) => Uвх=Uацп*(R_IN/R_FB); I=Uвх*SHUNT_I/SHUNT_U => I=Uацп*(R_IN/R_FB)*(SHUNT_I/SHUNT_U) => K=(R_IN/R_FB)*(SHUNT_I/SHUNT_U)
Прямой: Uацп=Uвх*(1+R_FB/R_IN) => Uвх=Uацп*(R_IN/(R_IN+R_FB)); I=Uвх*SHUNT_I/SHUNT_U => I=Uацп*(R_IN/(R_IN+R_FB))*(SHUNT_I/SHUNT_U) => K=(R_IN/(R_IN+R_FB))*(SHUNT_I/SHUNT_U)
Вариант2:
K=SHINT_I/Uацп (значение Uацп при входном SHUNT_U),
Uацп=SHUNT_U*(R_FB/R_IN) => K=(SHUNT_I*R_IN)/(SHUNT_U*R_FB)
Uацп=SHUNT_U*(1+R_FB/R_IN) => Uацп=SHUNT_U*((R_IN+R_FB)/R_IN) => K=(SHUNT_I*R_IN)/(SHUNT_U*(R_IN+R_FB))
*/
static int32_t i_sens_k[2]=
{
#ifdef I_SENS1_INVERT
		(I_SENS1_R_IN*1000/I_SENS1_R_FB)*(I_SENS1_SHUNT_I/I_SENS1_SHUNT_U),//(I_SENS1_SHUNT_I*I_SENS1_R_IN*1000)/(I_SENS1_SHUNT_U*I_SENS1_R_FB),//
#else
		(I_SENS1_R_IN*1000/(I_SENS1_R_IN+I_SENS1_R_FB))*(I_SENS1_SHUNT_I/I_SENS1_SHUNT_U),//(I_SENS1_SHUNT_I*I_SENS1_R_IN*1000ULL)/(I_SENS1_SHUNT_U*(I_SENS1_R_IN+I_SENS1_R_FB)),
#endif
#ifdef I_SENS2_INVERT
		(I_SENS2_R_IN*1000/I_SENS2_R_FB)*(I_SENS2_SHUNT_I/I_SENS2_SHUNT_U),//(I_SENS2_SHUNT_I*I_SENS2_R_IN*1000)/(I_SENS2_SHUNT_U*I_SENS2_R_FB),
#else
		((I_SENS2_R_IN*1000/(I_SENS2_R_IN+I_SENS2_R_FB))*(I_SENS2_SHUNT_I/I_SENS2_SHUNT_U),//(I_SENS2_SHUNT_I*I_SENS2_R_IN*1000)/(I_SENS2_SHUNT_U*(I_SENS2_R_IN+I_SENS2_R_FB)),
#endif
};
#else
static int32_t i_sens_k[2]=
		{
		I_SENS1_SHUNT*1000/I_SENS1_ADC_U,
		I_SENS2_SHUNT*1000/I_SENS2_ADC_U,
		};
#endif
static int32_t i_sens_b[2]={0,0};
static int32_t i_sens_d[2]={6,6}; //т.к. значения АЦП в мВ (3 нуля) и ещё I_SENS1_SHUNT*1000 (ещё 3 нуля)

/**
  * @brief  Получить данные датчика тока
  *
  * @param  chan: номер канала АЦП (от 0 до ADC_MAX_INPUT-1)
  *
  * @retval значение датчика тока в мА
  */
int32_t i_sens_get_val(uint8_t chan)
{uint8_t i=0;
	i=i_sens_get_element(chan);
	return(adc_get_calc(chan, i_sens_k[i], i_sens_b[i], i_sens_d[i], 3));
}

/**
  * @brief  Получить значения коэффициента K
  *
  * @retval K
  */
int32_t i_sens_get_k(uint8_t chan)
{uint8_t i=0;
	i=i_sens_get_element(chan);
	return(i_sens_k[i]);
}

/**
  * @brief  Получить значения коэффициента B
  *
  * @retval B
  */
int32_t i_sens_get_b(uint8_t chan)
{uint8_t i=0;
	i=i_sens_get_element(chan);
	return(i_sens_b[i]);
}

/**
  * @brief  Получить значения коэффициента D
  *
  * @retval D
  */
uint8_t i_sens_get_d(uint8_t chan)
{uint8_t i=0;
	i=i_sens_get_element(chan);
	return(i_sens_d[i]);
}

/**
  * @brief Установить значение коэффициента K
  *
  * @param  val: K
  */
void i_sens_set_k(uint8_t chan, int32_t val)
{uint8_t i=0;
	i=i_sens_get_element(chan);
	if (val>0)
		i_sens_k[i]=val;
	else
		i_sens_k[i]=1;
}

/**
  * @brief Установить значение коэффициента B
  *
  * @param  val: B
  */
void i_sens_set_b(uint8_t chan, int32_t val)
{uint8_t i=0;
i=i_sens_get_element(chan);
	i_sens_b[i]=val;
}

/**
  * @brief Установить значение коэффициента D
  *
  * @param  val: D
  */
void i_sens_set_d(uint8_t chan, uint8_t val)
{uint8_t i=0;
	i=i_sens_get_element(chan);
	i_sens_d[i]=val;
}

//Возвращает номер элемента в массиве, для данного канала АЦП
static uint8_t i_sens_get_element(uint8_t adc)
{
	if (adc>(ADC_MAX_INPUT-1)) return(0);
	if (adc==I_SENS2_ADC) return(1);
	else return(0);
}
