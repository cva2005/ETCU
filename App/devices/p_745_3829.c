/*
 * p_745_3829.c
 *
 *  Created on: 3 ����. 2016 �.
 *      Author: ������ �.�.
 */
#include "p_745_3829.h"
#include "adc.h"
#include "power.h"

static int32_t p_745_k=(P_745_R1+P_745_R2)*1000/P_745_R2;
static int32_t p_745_b=0;
static uint8_t p_745_d=6; //�.�. ��� ������� U��� � �� (3 ����) � ��� �������� �� 1000 ��������� (U_SENS1_R1+U_SENS1_R2) (��� 3 ����)

/**
  * @brief  �������� ������� ���������� � �������
  *
  * @retval �������� ������� � ��
  */
int32_t p_745_get_u(void)
{
	return(adc_get_calc(P_745_ADC, p_745_k, p_745_b, p_745_d, 3));
}

/**
  * @brief  �������� ������� �������� �������� � ������� (U��� = U���.���(0,01 * Pi - 0,12))
  *
  * @retval �������� � "��"
  */
int32_t p_745_get_val(void)
{uint32_t u, u_5v, pi;

	u=p_745_get_u();
	u_5v=power_get_5v();
	pi=(u_5v*12+u*100)*1000/(u_5v*1);

	return(pi);
}