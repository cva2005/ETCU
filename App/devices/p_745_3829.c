/*
 * p_745_3829.c
 *
 *  Created on: 3 февр. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "p_745_3829.h"
#include "adc.h"
#include "power.h"

static int32_t p_745_k=(P_745_R1+P_745_R2)*1000/P_745_R2;
static int32_t p_745_b=0;
static uint8_t p_745_d=6; //т.к. при расчёте Uацп в мВ (3 нуля) и ещё умножили на 1000 числитель (U_SENS1_R1+U_SENS1_R2) (ещё 3 нуля)

/**
  * @brief  Получить текущее напряжение с датчика
  *
  * @retval значение датчика в мВ
  */
int32_t p_745_get_u(void)
{
	return(adc_get_calc(P_745_ADC, p_745_k, p_745_b, p_745_d, 3));
}

/**
  * @brief  Получить текущее значение давления с датчика (Uвых = Uпит.ном(0,01 * Pi - 0,12))
  *
  * @retval давление в "Па"
  */
int32_t p_745_get_val(void)
{uint32_t u, u_5v, pi;

	u=p_745_get_u();
	u_5v=power_get_5v();
	pi=(u_5v*12+u*100)*1000/(u_5v*1);

	return(pi);
}
