/*
 * CRC.h
 *
 *  Created on: 01 ����. 2014 �.
 *      Author: ������ �.�.
 */

#ifndef CRC_H_
#define CRC_H_
#include "stm32f4xx_hal.h"

uint32_t reverse_32(uint32_t data);//������ ������� (������) ���� � ������� �����
uint32_t crc32(uint32_t Data);// ������������ 32-bit CRC ��������� ���������� ��������� � ����� ��������.
uint32_t crc32_ether(char *buf, int len, int clear);//��������� 32-bit CRC ��� ������ ������ �� ��������� ������������ ���������� CRC � ��������� Ethernet.
unsigned char crc8(unsigned char data, unsigned char crc);// ��������� 8-bit CRC ��������� ���������� ��������� � ����� ��������.
uint8_t crc8_1wire(void *buf, int len, unsigned char clear);//��������� 32-bit CRC ��� ������ ������ �� ��������� ������������ ���������� CRC � ��������� 1-wire.
uint16_t modbus_crc_rtu(uint8_t* data, uint16_t len);	//������� �������� CRC ��� ����� ModBus_RTU

#endif /* CRC_H_ */