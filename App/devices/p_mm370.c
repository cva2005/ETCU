/*
 * p_mm370.c
 *
 *  Created on: 3 февр. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "p_mm370.h"
#include "adc.h"
#include "power.h"

#if P_MM370_TABLE==0
	uint16_t p_mm370_r_tab[3]={287,109,9};
#elif P_MM370_TABLE==2
	uint16_t p_mm370_r_tab[3]={335,125,26};
#else
	uint16_t p_mm370_r_tab[3]={261,117,17};
#endif
int8_t p_mm370_g_tab[3]={0,5,10};

/**
  * @brief  Возвращает значение сопротивления датчика давления R2=Uвых*R1/(Uвх-Uвых)
  *
  * @param  chan: номер канала АЦП (от 0 до ADC_MAX_INPUT-1)
  *
  * @retval сопротивление "Ом".
  */
int32_t p_mm370_get_r(uint8_t chan)
{int32_t adc_u_out, adc_u_sub, r;

	adc_u_out=adc_get_u(chan);
	adc_u_sub=power_get_5v()-adc_u_out;
	if (adc_u_sub>0)
		r=(adc_u_out*P_MM370_R1)/adc_u_sub;
	else return(ADC_MAX_VAL);

	return(r);
}

/**
  * @brief  Пересчитывает сопротивление датчика давления в гс/см2
  *
  * @param  r: сопротивление в Ом
  *
  * @retval давление " гс/см2".
  */
int32_t p_mm370_convert_r_to_val(int32_t r)
{int32_t r1, r2, g1, g2, K, B, P;
uint8_t cnt=0;

	while ((r<p_mm370_r_tab[cnt])&&(cnt<3)) cnt++;

	if (cnt==0) return(0);
	if (cnt>=3) return(11);


	r1=p_mm370_r_tab[cnt-1];
	r2=p_mm370_r_tab[cnt];
	g1=p_mm370_g_tab[cnt-1]*1000000;
	g2=p_mm370_g_tab[cnt]*1000000;

	K=((g2-g1))/(r2-r1); //рассчитать зхначения K

	B=g1-(K*r1);
	P=r*K+B;
	P/=1000;

	return(P);
}

/**
  * @brief  Возвращает значение давления
  *
  * @param  chan: номер канала АЦП (от 0 до ADC_MAX_INPUT-1)
  *
  * @retval давление "гс/см2".
  */
int32_t p_mm370_get_val(uint8_t chan)
{int32_t r;

	if (chan>(ADC_MAX_INPUT-1)) return(0);
	r=p_mm370_get_r(chan);
	return(p_mm370_convert_r_to_val(r));

}
