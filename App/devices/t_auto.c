/*
 * t_auto.c
 *
 *  Created on: 28 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "t_auto.h"
#include "adc.h"
#include "power.h"
//uint16_t t_auto_r_tab[46]={137430,100707,72458,52684,38688,28677,21448,16176,12299,9423,7282,
#if T_AUTO_TABLE==0
uint16_t t_auto_r_tab[43]={42287,32682,24340,18285,13850,10573,8139,6316,
							4939,4033,3232,2576,2066,1669,1353,1104,906,750,624,522,435,369,313,266,
							227,195,167,144,125,108,95,92,89,87,85,82,80,78,76,73,71,56,44};
#elif T_AUTO_TABLE==2
#else
uint16_t t_auto_r_tab[43]={52684,38688,28677,21448,16176,12299,9423,7282,
							5671,4449,3515,2795,2237,1801,1459,1188,972,803,667,556,466,392,332,282,
							240,205,177,152,131,114,99,97,94,92,89,87,85,82,80,78,76,59,47};
#endif
int16_t t_auto_g_tab[43]={-30,-25,-20,-15,-10,-5,0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,
						  80,85,90,95,100,105,110,115,120,121,122,123,124,125,126,127,128,129,130,140,150};



/**
  * @brief  Возвращает значение сопротивления термодатчика R2=UвыхR1/(Uвх-Uвых)
  *
  * @param  chan: номер канала АЦП (от 0 до ADC_MAX_INPUT-1)
  *
  * @retval сопротивление "Ом".
  */
int32_t t_auto_get_r(uint8_t chan)
{int32_t adc_u_out, adc_u_sub, r;

	adc_u_out=adc_get_u(chan);
	adc_u_sub=power_get_5v()-adc_u_out;
	if (adc_u_sub>0)
		r=(adc_u_out*T_AUTO_R1)/adc_u_sub;
	else return(ADC_MAX_VAL);

	return(r);

}

/**
  * @brief  Пересчитывает сопротивление датчика температуры в °C
  *
  * @param  r: сопротивление в Ом
  *
  * @retval температура "м°C".
  */
int32_t t_auto_convert_r_to_val(int32_t r)
{int32_t r1, r2, g1, g2, K, B, P;
uint8_t cnt=0;

	while ((r<t_auto_r_tab[cnt])&&(cnt<43)) cnt++;

	if (cnt==0) return(-36000);
	if (cnt>=43) return(151000);


	r1=t_auto_r_tab[cnt-1];
	r2=t_auto_r_tab[cnt];
	g1=t_auto_g_tab[cnt-1]*1000000;
	g2=t_auto_g_tab[cnt]*1000000;

	K=((g2-g1))/(r2-r1); //рассчитать зхначения K

	B=g1-(K*r1);
	P=r*K+B;
	P/=1000;

	return(P);
}

/**
  * @brief  Возвращает значение температуры
  *
  * @param  chan: номер канала АЦП (от 0 до ADC_MAX_INPUT-1)
  *
  * @retval температура "м°C".
  */
int32_t t_auto_get_val(uint8_t chan)
{int32_t r;

	if (chan>(ADC_MAX_INPUT-1)) return(0);
	r=t_auto_get_r(chan);
	return(t_auto_convert_r_to_val(r));

}
