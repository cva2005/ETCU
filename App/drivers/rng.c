/*
 * rng.c
 *
 *  Created on: 14 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "rng.h"

#ifdef RNG_HARDWARE_EN
extern RNG_HandleTypeDef hrng;
#endif

static uint32_t Xi=0; //���������� ����� ���������� ��������� �����

/**
  * @brief  ���������� 16-������ ��������� �����, ��������� �������� ������������ ��������, ������ ����� ��� ��������� - �������� ���������� �������
  *
  * @retval ��������� �����
  */
uint16_t rng_rnd16_soft(void)
{uint16_t val;
	if (Xi==0) Xi=(uint16_t)HAL_GetTick();
	val=14013*Xi;
	val+=53011;
	val%= 0x10000;
	Xi=(14013*Xi + 53011) % 0x10000;
	return(Xi);
}
/**
  * @brief  ���������� 32-������ ��������� �����, ��������� �������� ������������ ��������, ������ ����� ��� ��������� - �������� ���������� �������
  *
  * @retval ��������� �����
  */
uint32_t rng_rnd32_soft(void)
{
	if (Xi==0) Xi=HAL_GetTick();
	Xi=(214013*Xi + 253011) % 0xFFFFFFFF;
	return(Xi);
}

#ifdef RNG_HARDWARE_EN
/**
  * @brief  ���������� 32-������ ��������� �����, ��������� ���������� ��������� ��������� �����
  *
  * @retval ��������� �����
  */
uint32_t rng_rnd32_hard(void)
{
	return(HAL_RNG_GetRandomNumber(&hrng));
}

/**
  * @brief  ���������� 16-������ ��������� �����, ��������� ���������� ��������� ��������� �����
  *
  * @retval ��������� �����
  */
uint16_t rng_rnd16_hard(void)
{
	return((uint16_t)HAL_RNG_GetRandomNumber(&hrng));
}
#endif