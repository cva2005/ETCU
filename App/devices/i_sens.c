/*
 * i_sens.c
 *
 *  Created on: 3 ����. 2016 �.
 *      Author: ������ �.�.
 */
#include "i_sens.h"
#include "adc.h"
#ifdef I_HARD_CALC
/* ������ � ����:
�������1:
�������������: U���=U��*(R_FB/R_IN) => U��=U���*(R_IN/R_FB); I=U��*SHUNT_I/SHUNT_U => I=U���*(R_IN/R_FB)*(SHUNT_I/SHUNT_U) => K=(R_IN/R_FB)*(SHUNT_I/SHUNT_U)
������: U���=U��*(1+R_FB/R_IN) => U��=U���*(R_IN/(R_IN+R_FB)); I=U��*SHUNT_I/SHUNT_U => I=U���*(R_IN/(R_IN+R_FB))*(SHUNT_I/SHUNT_U) => K=(R_IN/(R_IN+R_FB))*(SHUNT_I/SHUNT_U)
�������2:
K=SHINT_I/U��� (�������� U��� ��� ������� SHUNT_U),
U���=SHUNT_U*(R_FB/R_IN) => K=(SHUNT_I*R_IN)/(SHUNT_U*R_FB)
U���=SHUNT_U*(1+R_FB/R_IN) => U���=SHUNT_U*((R_IN+R_FB)/R_IN) => K=(SHUNT_I*R_IN)/(SHUNT_U*(R_IN+R_FB))
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
static int32_t i_sens_d[2]={6,6}; //�.�. �������� ��� � �� (3 ����) � ��� I_SENS1_SHUNT*1000 (��� 3 ����)

/**
  * @brief  �������� ������ ������� ����
  *
  * @param  chan: ����� ������ ��� (�� 0 �� ADC_MAX_INPUT-1)
  *
  * @retval �������� ������� ���� � ��
  */
int32_t i_sens_get_val(uint8_t chan)
{uint8_t i=0;
	i=i_sens_get_element(chan);
	return(adc_get_calc(chan, i_sens_k[i], i_sens_b[i], i_sens_d[i], 3));
}

/**
  * @brief  �������� �������� ������������ K
  *
  * @retval K
  */
int32_t i_sens_get_k(uint8_t chan)
{uint8_t i=0;
	i=i_sens_get_element(chan);
	return(i_sens_k[i]);
}

/**
  * @brief  �������� �������� ������������ B
  *
  * @retval B
  */
int32_t i_sens_get_b(uint8_t chan)
{uint8_t i=0;
	i=i_sens_get_element(chan);
	return(i_sens_b[i]);
}

/**
  * @brief  �������� �������� ������������ D
  *
  * @retval D
  */
uint8_t i_sens_get_d(uint8_t chan)
{uint8_t i=0;
	i=i_sens_get_element(chan);
	return(i_sens_d[i]);
}

/**
  * @brief ���������� �������� ������������ K
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
  * @brief ���������� �������� ������������ B
  *
  * @param  val: B
  */
void i_sens_set_b(uint8_t chan, int32_t val)
{uint8_t i=0;
i=i_sens_get_element(chan);
	i_sens_b[i]=val;
}

/**
  * @brief ���������� �������� ������������ D
  *
  * @param  val: D
  */
void i_sens_set_d(uint8_t chan, uint8_t val)
{uint8_t i=0;
	i=i_sens_get_element(chan);
	i_sens_d[i]=val;
}

//���������� ����� �������� � �������, ��� ������� ������ ���
static uint8_t i_sens_get_element(uint8_t adc)
{
	if (adc>(ADC_MAX_INPUT-1)) return(0);
	if (adc==I_SENS2_ADC) return(1);
	else return(0);
}