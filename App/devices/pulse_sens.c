/*
 * pulse_sens.c
 *
 *  Created on: 28 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "pulse_sens.h"
#include "pulse_in.h"

static uint8_t pulse_sens_chan[PULSE_IN_NUMBER]={0,0,0}; //����� ������ � �������� ���������  ���������� ������
static uint16_t pulse_sens_pl[PULSE_IN_NUMBER];	//���������� ��������� �� ������� ���������

/**
  * @brief  ���������������� ���������� ������
  *
  * @param  sens: ����� ������� �� 1 �� PULSE_IN_NUMBER
  * 		chan: ����� ����������� ����� �� ������� ���������� ������ (�� 1 �� PULSE_IN_NUMBER)
  *
  * @retval �����������*1000 (m�C)
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
  * @brief  ���������� �������� ����������� �������
  *
  * @param  sens: ����� ������� �� 1 �� PULSE_IN_NUMBER
  *
  * @retval �������� ������*1000/������ (�������� ��� ������� �������: ��/���)
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
  * @brief  ���������� �������� ��������� �� ������� ���������
  *
  * @param  sens: ����� ������� �� 1 �� PULSE_IN_NUMBER
  *
  * @retval ���������� ��������� �� ������� ��������� (��������, ��������� �� ���� ��� �������� ���)
  */
uint16_t pulse_sens_get_pl(uint8_t sens)
{
	if (sens==0) sens=1;
	if (sens>PULSE_IN_NUMBER) sens=PULSE_IN_NUMBER;
	sens--;

	return(pulse_sens_pl[sens]);
}

/**
  * @brief  ������������� �������� ��������� �� ������� ���������
  *
  * @param  sens: ����� ������� �� 1 �� PULSE_IN_NUMBER
  * 		pl: ���������� ��������� �� ������� ��������� (��������, ��������� �� ���� ��� �������� ���)
  */
void pulse_sens_set_pl(uint8_t sens, uint16_t pl)
{
	if (sens==0) sens=1;
	if (sens>PULSE_IN_NUMBER) sens=PULSE_IN_NUMBER;
	sens--;

	pulse_sens_pl[sens]=pl;
}