/*
 * rs485_2.h
 *
 *  Created on: 17 ���. 2015 �.
 *      Author: ������ �.�.
 *      ������� RS-485 �� UART-�
 *      v.2.0 ���������:
 *      - ��������� ����������� ������ � ���������� ��������� ����������� �����
 *      - DMA �� ���� ������ ������ ���� �������� ��� "cyclic"
 *      - ��������� �������� � ���������� RS-485 ��� ������������ ������ �� �����
 *      - ��������� ���������� ���������� DMA ��� ���������� �������� ������ RX
 */

#ifndef APP_DRIVERS_RS485_2_H_
#define APP_DRIVERS_RS485_2_H_
#include "stm32f4xx_hal.h"
//#include "main.h"
#include "mxconstants.h"

#define RS485_2_UART_NUMBER 1 //����� ������ UART �� ������� �������� RS485

//#define RS485_2_MODE_IRQ //���� ����������, �� ���� �������� � ������ IRQ, ����� � ������ DMA
//#define RS485_2_HFC_EN		//���� ����������, �� ������� ���������� �������� ���������� ��������� "Hardware Flow Control"

#define RS485_2_BUF_SIZE_RX 4096		//������ ������ �� ����
#define RS485_2_BUF_SIZE_TX 256			//������ ������ �� ��������

#if RS485_2_UART_NUMBER==1
#define RS485_2_UART huart1				//UART �� ������� ����� RS485
#define RS485_2_DMA_RX hdma_usart1_rx	//����� DMA ������� ��������� ������
#define RS485_2_DMA_TX hdma_usart1_tx	//����� DMA ������� ������� ������
#define DMA_RX_USART1_IRQ_POST 			rs485_2_reinit_dma()
#ifndef RS485_2_HFC_EN
#define UART1_IRQ_PRE					rs485_2_irq(&RS485_2_UART)
#endif
#ifdef RS485_2_MODE_IRQ
#define UART1_IRQ_POST					if (RS485_2_UART.RxXferCount==0) rs485_2_reinit_irq()
#endif
#endif
#if RS485_2_UART_NUMBER==2
#define RS485_2_UART huart2				//UART �� ������� ����� RS485
#define RS485_2_DMA_RX hdma_usart2_rx	//����� DMA ������� ��������� ������
#define RS485_2_DMA_TX hdma_usart2_tx	//����� DMA ������� ������� ������
#define DMA_RX_USART2_IRQ_POST 				rs485_2_reinit_dma()//if(__HAL_DMA_GET_COUNTER(&RS485_2_DMA_RX)==0)	rs485_2_reinit_dma()
#ifndef RS485_2_HFC_EN
#define UART2_IRQ_PRE					rs485_2_irq(&RS485_2_UART)
#endif
#ifdef RS485_2_MODE_IRQ
#define UART2_IRQ_POST					if (RS485_2_UART.RxXferCount==0) rs485_2_reinit_irq()
#endif
#endif
#if RS485_2_UART_NUMBER==3
#define RS485_2_UART huart3				//UART �� ������� ����� RS485
#define RS485_2_DMA_RX hdma_usart3_rx	//����� DMA ������� ��������� ������
#define RS485_2_DMA_TX hdma_usart3_tx	//����� DMA ������� ������� ������
#define DMA_RX_USART3_IRQ_POST 			rs485_2_reinit_dma()
#ifndef RS485_2_HFC_EN
#define UART3_IRQ_PRE					rs485_2_irq(&RS485_2_UART)
#endif
#ifdef RS485_2_MODE_IRQ
#define UART3_IRQ_POST					if (RS485_2_UART.RxXferCount==0) rs485_2_reinit_irq()
#endif
#endif

#ifndef RS485_2_HFC_EN
#define RS485_2_RE_PIN RS485_RE2_Pin	//����� ����� �� ����� ��� ����� ������ ���������� �������
#define RS485_2_RE_PPORT RS485_RE2_GPIO_Port //���� �� ������� ����� ����� ���������� �������
#endif

void rs485_2_init (uint32_t speed);		//������������� ���������� RS482 ����� 2
void rs485_2_clear_buf(void);			//������� ����� �����;
void* rs485_2_get_buf_pointer(void);	//���������� ����� ������ � ������
uint32_t rs485_2_get_buf_shift(void);	//���������� �������� ������������ ������ ������, ���� � ������ ����� ������� ��������� ����
void rs485_2_reinit_dma(void);			//��������������� ������ (��������� ��������� ����� �� ������ ������) � ������ DMA
uint32_t rs485_2_get_buf_overflow(void);//���������� ���������� ������������ ������
uint32_t rs485_2_read_rx_data(void* buf, uint32_t size);//�������� �� ������ ��������, �� �� ������������ ������� �������� ������
uint32_t rs485_2_get_rx_size(void); 	//���������� ���������� ��������, �� �� ������������ ������� �������� ������
uint32_t rs485_2_write_tx_data(void* buf, uint32_t size); //�������� � ����� �������� ������ � ���������� � RS485_2
uint32_t rs485_2_direct_tx_data(void* buf, uint32_t size);//������� ������ �� ��������� ������� ������: ����� ������ �������� ���������� ������������� ������ � ������ �� ��������� ��������
uint32_t rs485_2_get_tx_status(void);	//���������� ��������� ����������: ���������� ���������� ��� �� ���������� ����
#ifdef RS485_2_MODE_IRQ
void rs485_2_reinit_irq(void);			//��������������� ������ (��������� ��������� ����� �� ������ ������) � ������ IRQ
#endif
#ifndef RS485_2_HFC_EN
void rs485_2_irq(UART_HandleTypeDef *huart); //���������� ���������� UART
#endif

#endif /* APP_DRIVERS_RS485_2_H_ */