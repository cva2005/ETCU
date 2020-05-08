/*
 * CRC.h
 *
 *  Created on: 01 сент. 2014 г.
 *      Author: Перчиц А.Н.
 */

#ifndef CRC_H_
#define CRC_H_
#include "stm32f4xx_hal.h"

uint32_t reverse_32(uint32_t data);//Меняет местами (реверс) биты в двойном слове
uint32_t crc32(uint32_t Data);// Рассчитывает 32-bit CRC используя предыдущий результат и новое значение.
uint32_t crc32_ether(char *buf, int len, int clear);//Вычисляет 32-bit CRC для буфера данных по алгоритму аналогичному вычислению CRC в протокгое Ethernet.
unsigned char crc8(unsigned char data, unsigned char crc);// Вычисляет 8-bit CRC используя предыдущий результат и новое значение.
uint8_t crc8_1wire(void *buf, int len, unsigned char clear);//Вычисляет 32-bit CRC для буфера данных по алгоритму аналогичному вычислению CRC в протокгое 1-wire.
uint16_t modbus_crc_rtu(uint8_t* data, uint16_t len);	//Функция подсчёта CRC для линии ModBus_RTU

#endif /* CRC_H_ */
