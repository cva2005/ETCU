/*
 * power.c
 *
 *  Created on: 24 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "power.h"
#include "adc.h"

uint32_t power_5v_k=((POWER_5V_R_DW+POWER_5V_R_UP)*POWER_ACCURACY_K)/POWER_5V_R_DW; //Uвх=Uвых*((R1+R2)/R2), т.е. K=((R1+R2)*точность)/R2;
uint32_t power_5v_b=0;
uint32_t power_vbat_k=1000;
uint32_t power_vbat_b=0;


/**
  * @brief  Инициализирует датчики питания: датчик напряжения 5В и датчик батарейки
  */
//void power_init(void)
//{
	//power_5v_b=0;
	//if (POWER_5V_R_DW!=0)
	//	power_5v_k=((POWER_5V_R_DW+POWER_5V_R_UP)*POWER_ACCURACY_K)/POWER_5V_R_DW; //Uвх=Uвых*((R1+R2)/R2), т.е. K=((R1+R2)*точность)/R2
	//power_vbat_b=0;
	//power_vbat_k=1000;
//}

/**
  * @brief  Возвращает значения напряжения 5В
  *
  * @retval значение в мВ
  */
uint32_t power_get_5v(void)
{uint32_t adc_val, calc_val;

	adc_val=adc_get_average(POWER_5V_CHAN)+power_5v_b;
	if (ADC_MAX_VAL!=0)
		calc_val=(adc_val*ADC_REF_VAL)/ADC_MAX_VAL;
	if (POWER_ACCURACY_K!=0)
		calc_val=(calc_val*power_5v_k)/POWER_ACCURACY_K;

	return(calc_val);
}

/**
  * @brief  Возвращает значения напряжения батарейки
  *
  * @retval значение в мВ
  */
uint32_t power_get_vbat(void)
{uint32_t adc_val, calc_val;

	adc_val=adc_get_average(POWER_VBAT_CHAN)+power_vbat_b;
	if (ADC_MAX_VAL!=0)
		calc_val=(adc_val*ADC_REF_VAL*2)/ADC_MAX_VAL;
	if (POWER_ACCURACY_K!=0)
		calc_val=(calc_val*power_vbat_k)/POWER_ACCURACY_K;

	return(calc_val);
}
