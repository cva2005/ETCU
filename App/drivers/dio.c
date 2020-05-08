/*
 * dio.c
 *
 *  Created on: 7 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "dio.h"

/**
  * @brief  Читает текущее сосотяние выходов
  *
  * @param  состояние выходов; битовые поля: 0 - выключен, 1 - включен
  */
uint8_t dio_out_read(void)
{uint8_t dout=0;

	if (HAL_GPIO_ReadPin(RELAY1)) dout|=0x01;
	if (HAL_GPIO_ReadPin(RELAY2)) dout|=0x02;
	if (HAL_GPIO_ReadPin(RELAY3)) dout|=0x04;
	if (HAL_GPIO_ReadPin(RELAY4)) dout|=0x08;
	if (HAL_GPIO_ReadPin(RELAY5)) dout|=0x10;
	if (HAL_GPIO_ReadPin(RELAY6)) dout|=0x20;
	if (HAL_GPIO_ReadPin(RELAY7)) dout|=0x40;
	if (HAL_GPIO_ReadPin(RELAY8)) dout|=0x80;

	return(dout);
}

/**
  * @brief  Установливает заданное значение дискретных выходов
  *
  * @param  data: битавые поля: 0 - включить, 1 - включить
  */
void dio_out_write(uint8_t data)
{
	(data&0x01)!=0 ? HAL_GPIO_WritePin(RELAY1, GPIO_PIN_SET) : HAL_GPIO_WritePin(RELAY1, GPIO_PIN_RESET);
	(data&0x02)!=0 ? HAL_GPIO_WritePin(RELAY2, GPIO_PIN_SET) : HAL_GPIO_WritePin(RELAY2, GPIO_PIN_RESET);
	(data&0x04)!=0 ? HAL_GPIO_WritePin(RELAY3, GPIO_PIN_SET) : HAL_GPIO_WritePin(RELAY3, GPIO_PIN_RESET);
	(data&0x08)!=0 ? HAL_GPIO_WritePin(RELAY4, GPIO_PIN_SET) : HAL_GPIO_WritePin(RELAY4, GPIO_PIN_RESET);
	(data&0x10)!=0 ? HAL_GPIO_WritePin(RELAY5, GPIO_PIN_SET) : HAL_GPIO_WritePin(RELAY5, GPIO_PIN_RESET);
	(data&0x20)!=0 ? HAL_GPIO_WritePin(RELAY6, GPIO_PIN_SET) : HAL_GPIO_WritePin(RELAY6, GPIO_PIN_RESET);
	(data&0x40)!=0 ? HAL_GPIO_WritePin(RELAY7, GPIO_PIN_SET) : HAL_GPIO_WritePin(RELAY7, GPIO_PIN_RESET);
	(data&0x80)!=0 ? HAL_GPIO_WritePin(RELAY8, GPIO_PIN_SET) : HAL_GPIO_WritePin(RELAY8, GPIO_PIN_RESET);
}
