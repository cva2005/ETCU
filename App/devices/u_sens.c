/*
 * u_sens.c
 *
 *  Created on: 3 ����. 2016 �.
 *      Author: ������ �.�.
 */
#include "u_sens.h"
#include "adc.h"
/* ������ K
 U��=U���*((R1+R2)/R2), K=(R1+R2)/R2;
 */
static int32_t u_sens1_k=(U_SENS1_R1+U_SENS1_R2)*1000/U_SENS1_R2;
static int32_t u_sens1_b=0;
static uint8_t u_sens1_d=6; //�.�. ��� ������� U��� � �� (3 ����) � ��� �������� �� 1000 ��������� (U_SENS1_R1+U_SENS1_R2) (��� 3 ����)

/**
  * @brief  �������� ������ ������� ����������
  *
  * @retval �������� ������� ���������� � ��
  */
int32_t u_sens_get_val(void)
{
	return(adc_get_calc(U_SENS1_ADC, u_sens1_k, u_sens1_b, u_sens1_d, 3));
}

/**
  * @brief  �������� �������� ������������ K
  *
  * @retval K
  */
int32_t u_sens_get_k(void)
{
	return(u_sens1_k);
}

/**
  * @brief  �������� �������� ������������ B
  *
  * @retval B
  */
int32_t u_sens_get_b(void)
{
	return(u_sens1_b);
}

/**
  * @brief  �������� �������� ������������ D
  *
  * @retval D
  */
uint8_t u_sens_get_d(void)
{
	return(u_sens1_d);
}

/**
  * @brief ���������� �������� ������������ K
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
  * @brief ���������� �������� ������������ B
  *
  * @param  val: B
  */
void u_sens_set_b(int32_t val)
{
	u_sens1_b=val;
}

/**
  * @brief ���������� �������� ������������ D
  *
  * @param  val: D
  */
void u_sens_set_d(uint8_t val)
{
	u_sens1_d=val;
}