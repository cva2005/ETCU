/*
 * can_1.h
 *
 *  Created on: 5 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DRIVERS_CAN_1_H_
#define APP_DRIVERS_CAN_1_H_
#include "stm32f4xx_hal.h"

#define CAN_1_BUF_SIZE_RX 50		//������ ������ �� ���� (� �������)
#define CAN_1_END_FILTER 13			//��������� ����� ������� ��� CAN1, ���� 2 ������ CAN, �� ����� ����� ������� ������ ������ ���� ������ �� 1 ��� ����� ���������� ������� ��� CAN2 (CAN_1_END_FILTER<CAN_2_START_FILTER)
#define CAN_ALL_FILTERS 28			//����� ����� �������� CAN ��� ������� ����������������, ������ ������ ��� CAN_1_END_FILTER (CAN_1_END_FILTER<CAN_ALL_FILTERS)
#define CAN1_TX_TIMEOUT 10		//�����������-���������� ����� ��� �������� ������

#define CAN_1 hcan1				//���������� ����� �� ������� ����� CAN
#define CAN_1_RX0_IRQ 		can_1_rx0_irq(&CAN_1); //���������� ���������� CAN FIFO0
#define CAN_1_RX1_IRQ 		can_1_rx1_irq(&CAN_1); //���������� ���������� CAN FIFO1

void can_1_init(uint8_t prescaler);					//������������� ���������� CAN ����� 1
uint8_t can_1_set_filter32(uint32_t id, uint8_t ide_rtr1, uint32_t id_mask, uint8_t ide_rtr2, uint8_t type);//��������� 32-������� ������� �� ���� (��������������� ����� 2 ������� ��� �����)
void can_1_clear_buf(void);				//������� ����� �����
void* can_1_get_buf_pointer(void);		//���������� ����� ������ � ������
uint32_t can_1_get_buf_shift(void);		//����������  ����� �������� ������ ���� ����� ������� ��������� ����
uint32_t can_1_get_buf_overflow(void);	//���������� ���������� ������������ ������
uint8_t can_1_read_rx_data(uint32_t* id, uint8_t* length, uint8_t* data); //�������� �� ������ ��������, �� �� ������������ ������
uint32_t can_1_get_rx_size(void);		//���������� ���������� ��������, �� �� ������������ ������� �������� �������
uint8_t can_1_write_tx_data(uint32_t id, uint8_t length, uint8_t* data);//�������� � ����� �������� ������ � ���������� � CAN
void can_1_buf_point_move(void);	//����������� ��������� � �������� ������ �� ��������� ��������� �����

void can_1_rx0_irq(CAN_HandleTypeDef *hcan);//���������� ���������� CAN1 ������� FIFO0
void can_1_rx1_irq(CAN_HandleTypeDef *hcan);//���������� ���������� CAN1 ������� FIFO1

//-------------------------CAN BUS SPEED-------------------------------------------
#define CAN_1_SPEED_20K 300
#define CAN_1_SPEED_125K 48
#define CAN_1_SPEED_250K 24
#define CAN_1_SPEED_500K 12
#define CAN_1_SPEED_1000K 6

#endif /* APP_DRIVERS_CAN_1_H_ */