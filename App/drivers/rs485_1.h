/*
 * rs485_1.h
 *
 *  Created on: 18 ���. 2015 �.
 *      Author: ������ �.�.
 *      ������� RS-485 �� UART-�
 *      v.2.0 ���������:
 *      - ��������� ����������� ������ � ���������� ��������� ����������� �����
 *      - DMA �� ���� ������ ������ ���� �������� ��� "cyclic"
 *      - ��������� �������� � ���������� RS-485 ��� ������������ ������ �� �����
 *      - ��������� ���������� ���������� DMA ��� ���������� �������� ������ RX
 */

#ifndef APP_DRIVERS_RS485_1_H_
#define APP_DRIVERS_RS485_1_H_
#include <stdbool.h>
#include "stm32f4xx_hal.h"
//#include "main.h"
#include "mxconstants.h"

#define RS485_1_UART_NUMBER 2 //����� ������ UART �� ������� �������� RS485

//#define RS485_1_MODE_IRQ //���� ����������, �� ���� �������� � ������ IRQ, ����� � ������ DMA
//#define RS485_1_HFC_EN		//���� ����������, �� ������� ���������� �������� ���������� ��������� "Hardware Flow Control"

#define RS485_1_BUF_SIZE_RX 4096		//������ ������ �� ����
#define RS485_1_BUF_SIZE_TX 256			//������ ������ �� ��������

#if RS485_1_UART_NUMBER==1
#define RS485_1_UART huart1				//UART �� ������� ����� RS485
#define RS485_1_DMA_RX hdma_usart1_rx	//����� DMA ������� ��������� ������
#define RS485_1_DMA_TX hdma_usart1_tx	//����� DMA ������� ������� ������
#define DMA_RX_USART1_IRQ_POST 			rs485_1_reinit_dma()
#ifndef RS485_1_HFC_EN
#define UART1_IRQ_PRE					rs485_1_irq(&RS485_1_UART)
#endif
#ifdef RS485_1_MODE_IRQ
#define UART1_IRQ_POST					if (RS485_1_UART.RxXferCount==0) rs485_1_reinit_irq()
#endif
#endif
#if RS485_1_UART_NUMBER==2
#define RS485_1_UART huart2				//UART �� ������� ����� RS485
#define RS485_1_DMA_RX hdma_usart2_rx	//����� DMA ������� ��������� ������
#define RS485_1_DMA_TX hdma_usart2_tx	//����� DMA ������� ������� ������
#define DMA_RX_USART2_IRQ_POST 				rs485_1_reinit_dma()//if(__HAL_DMA_GET_COUNTER(&RS485_1_DMA_RX)==0)	rs485_1_reinit_dma()
#ifndef RS485_1_HFC_EN
#define UART2_IRQ_PRE					rs485_1_irq(&RS485_1_UART)
#endif
#ifdef RS485_1_MODE_IRQ
#define UART2_IRQ_POST					if (RS485_1_UART.RxXferCount==0) rs485_1_reinit_irq()
#endif
#endif
#if RS485_1_UART_NUMBER==3
#define RS485_1_UART huart3				//UART �� ������� ����� RS485
#define RS485_1_DMA_RX hdma_usart3_rx	//����� DMA ������� ��������� ������
#define RS485_1_DMA_TX hdma_usart3_tx	//����� DMA ������� ������� ������
#define DMA_RX_USART3_IRQ_POST 			rs485_1_reinit_dma()
#ifndef RS485_1_HFC_EN
#define UART3_IRQ_PRE					rs485_1_irq(&RS485_1_UART)
#endif
#ifdef RS485_1_MODE_IRQ
#define UART3_IRQ_POST					if (RS485_1_UART.RxXferCount==0) rs485_1_reinit_irq()
#endif
#endif

#ifndef RS485_1_HFC_EN
#define RS485_1_RE_PIN RS485_RE1_Pin	//����� ����� �� ����� ��� ����� ������ ���������� �������
#define RS485_1_RE_PPORT RS485_RE1_GPIO_Port //���� �� ������� ����� ����� ���������� �������
#endif

bool rs485_1_reinit (uint32_t speed);
void rs485_1_init (uint32_t speed);		//������������� ���������� RS482 ����� 1
void rs485_1_clear_buf(void);			//������� ����� �����;
void* rs485_1_get_buf_pointer(void);	//���������� ����� ������ � ������
uint32_t rs485_1_get_buf_shift(void);	//���������� �������� ������������ ������ ������, ���� � ������ ����� ������� ��������� ����
void rs485_1_reinit_dma(void);			//��������������� ������ (��������� ��������� ����� �� ������ ������) � DMA ������
uint32_t rs485_1_get_buf_overflow(void);//���������� ���������� ������������ ������
uint32_t rs485_1_read_rx_data(void* buf, uint32_t size);//�������� �� ������ ��������, �� �� ������������ ������� �������� ������
uint32_t rs485_1_get_rx_size(void); 	//���������� ���������� ��������, �� �� ������������ ������� �������� ������
uint32_t rs485_1_write_tx_data(void* buf, uint32_t size); //�������� � ����� �������� ������ � ���������� � RS485_1
uint32_t rs485_1_direct_tx_data(void* buf, uint32_t size);//������� ������ �� ��������� ������� ������: ����� ������ �������� ���������� ������������� ������ � ������ �� ��������� ��������
uint32_t rs485_1_get_tx_status(void);	//���������� ��������� ����������: ���������� ���������� ��� �� ���������� ����
#ifdef RS485_1_MODE_IRQ
void rs485_1_reinit_irq(void);			//��������������� ������ (��������� ��������� ����� �� ������ ������) � IRQ ������
#endif
#ifndef RS485_1_HFC_EN
void rs485_1_irq(UART_HandleTypeDef *huart); //���������� ���������� UART
#endif


/**
  * @brief  Returns the number of remaining data units in the current DMAy Streamx transfer.
  * @param  __HANDLE__: DMA handle
  *
  * @retval The number of remaining data units in the current DMA Stream transfer.
  */
//#define __HAL_DMA_GET_COUNTER(__HANDLE__) ((__HANDLE__)->Instance->CNDTR)

#endif /* APP_DRIVERS_RS485_1_H_ */
