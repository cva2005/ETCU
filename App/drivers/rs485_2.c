/*
 * rs485_2.c
 *
 *  Created on: 17 ���. 2015 �.
 *      Author: ������ �.�.
 */
#include "rs485_2.h"
#include <string.h>

extern UART_HandleTypeDef RS485_2_UART;
#ifndef RS485_2_MODE_IRQ
extern DMA_HandleTypeDef RS485_2_DMA_RX;
#endif
extern DMA_HandleTypeDef RS485_2_DMA_TX;

static uint8_t rs485_2_buf[RS485_2_BUF_SIZE_RX]; //����� ��� ����� �����
static uint8_t rs485_2_buf_tx[RS485_2_BUF_SIZE_TX]; //����� ��� �������� �����
static uint8_t rs485_2_buf_overflow;		  //������� ������������ ������
static uint32_t rs485_2_buf_read_pointer; //��������� � ������ �� ������ ������� ��� �� ����������

/**
  * @brief  ������������� ���������� RS482 ����� 2
  *
  * @param  speed: �������� ������ � ���/�
  */
void rs485_2_init (uint32_t speed)
{
#ifndef RS485_2_HFC_EN
	HAL_GPIO_WritePin(RS485_2_RE_PPORT, RS485_2_RE_PIN, GPIO_PIN_RESET);
#endif
//������������� UART
//	RS485_2_UART.Instance = USART1;
	RS485_2_UART.Init.BaudRate = speed;
//	RS485_2_UART.Init.WordLength = UART_WORDLENGTH_8B;
//	RS485_2_UART.Init.StopBits = UART_STOPBITS_1;
//	RS485_2_UART.Init.Parity = UART_PARITY_NONE;
//	RS485_2_UART.Init.Mode = UART_MODE_TX_RX;
//	RS485_2_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//	RS485_2_UART.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&RS485_2_UART);
//��������� ����� �����
	rs485_2_clear_buf();
	HAL_UART_Receive_DMA(&RS485_2_UART, rs485_2_buf, sizeof(rs485_2_buf));
	__HAL_DMA_DISABLE_IT(&RS485_2_DMA_RX, DMA_IT_HT);
}

/**
  * @brief  ������� ����� �����
  */

void rs485_2_clear_buf(void)
{
#ifdef RS485_2_MODE_IRQ
	rs485_2_reinit_irq();
#else
	//HAL_UART_DMAStop(&RS485_2_UART);
	//rs485_2_reinit_dma();
	//HAL_UART_Receive_DMA(&RS485_2_UART, rs485_1_buf, sizeof(rs485_2_buf));
	//__HAL_DMA_DISABLE_IT(&RS485_2_DMA_RX, DMA_IT_HT);
#endif
	rs485_2_buf_overflow=0;
	rs485_2_buf_read_pointer=rs485_2_get_buf_shift();
}

/**
  * @brief  ���������� ����� ������ � ������
  *
  * @retval �����
  */
void* rs485_2_get_buf_pointer(void)
{
	return(rs485_2_buf);
}

/**
  * @brief  ���������� �������� ������������ ������ ������ ���� � ������ ����� ������� ��������� ����
  *
  * @retval �������� �� 0 �� RS485_2_BUF_SIZE-1
  */
uint32_t rs485_2_get_buf_shift(void)
{
#ifdef RS485_2_MODE_IRQ
	return(sizeof(rs485_2_buf)-RS485_2_UART.RxXferCount);
#else
	return(sizeof(rs485_2_buf)-__HAL_DMA_GET_COUNTER(&RS485_2_DMA_RX));
#endif
}

/**
  * @brief  ��������������� ������ (��������� ��������� ����� �� ������ ������) � ������ DMA
  *
  */
void rs485_2_reinit_dma(void)
{
#ifndef RS485_2_MODE_IRQ
	rs485_2_buf_overflow++;
#endif
}

#ifdef RS485_2_MODE_IRQ
/**
  * @brief  ��������������� ������ (��������� ��������� ����� �� ������ ������) � ������ IRQ
  *
  */
void rs485_2_reinit_irq(void)
{

	HAL_UART_Receive_IT(&RS485_2_UART, rs485_2_buf, sizeof(rs485_2_buf));
	rs485_2_buf_overflow++;
}
#endif

/**
  * @brief  ���������� ���������� ������������ ������
  *
  * @retval ���������� ������������
  */
uint32_t rs485_2_get_buf_overflow(void)
{
	return(rs485_2_buf_overflow);
}

/**
  * @brief  �������� �� ������ ��������, �� �� ������������ ������
  *
  * @param  buf: ��������� �� �����, ���� ���� ����������� ������
  *			size: ������ ������ ������� ���� �����������
  * @retval ��������� ������������� ������
  */
uint32_t rs485_2_read_rx_data(void* buf, uint32_t size)
{uint32_t cpy_size=0, rx_point, buf_point=0;

	rx_point=rs485_2_get_buf_shift();

	if (rx_point==rs485_2_buf_read_pointer) return(0);

	if (rx_point<rs485_2_buf_read_pointer) //���� ��������� ����� ��������� ����� ����� ������
		{
		cpy_size=sizeof(rs485_2_buf)-rs485_2_buf_read_pointer; //��������� ���������� �� ����������� ������ �� ����� ������
		if (cpy_size>size) cpy_size=size;				//���� ���������� ������ ������ ��� ��������������� ��� ������ �����
		memcpy((uint8_t*)buf, &rs485_2_buf[rs485_2_buf_read_pointer], cpy_size);
		rs485_2_buf_read_pointer+=cpy_size; //����������� ��������� ����������� ������
		if (rs485_2_buf_read_pointer>=sizeof(rs485_2_buf)) rs485_2_buf_read_pointer=0; //���� �������� ��� ������ �� ����� �������� ������, �� ��������� ����������� ������ ����������� �� 0
		if (rs485_2_buf_read_pointer!=0) return (cpy_size); //���� ������ ���������������� ������ �� �������� ����������� ��� ������ �� ����� �������� ������, �� ����� �.�. ������ ���������� ������
		buf_point=cpy_size;								//� ������ ��� ����������� ����������� ���������
		size-=cpy_size;									//��������� ������ ������ ��� ���������� �� ����� ������������� ������
		}

	cpy_size=rx_point-rs485_2_buf_read_pointer; //��������� ���������� �� ����������� ������
	if (cpy_size>size) cpy_size=size;				//���� ���������� ������ ������ ��� ��������������� ��� ������ �����
	memcpy(&((uint8_t*)buf)[buf_point], &rs485_2_buf[rs485_2_buf_read_pointer], cpy_size);
	rs485_2_buf_read_pointer+=cpy_size;

	return(cpy_size+buf_point);
}

/**
  * @brief  ���������� ���������� ��������, �� �� ������������ ������� �������� ������
  *
  * @retval ���������� ������
  */
uint32_t rs485_2_get_rx_size(void)
{uint32_t sz=0, rx_point, pt;

	rx_point=rs485_2_get_buf_shift();
	pt=rs485_2_buf_read_pointer;

	if (rx_point<rs485_2_buf_read_pointer) //���� ��������� ����� ��������� ����� ����� ������
		{
		sz=sizeof(rs485_2_buf)-rs485_2_buf_read_pointer;
		pt=0;
		}
	sz+=(rx_point-pt);
	if (sz==0) rs485_2_buf_overflow=0;

	return(sz);
}

/**
  * @brief  �������� � ����� �������� ������ � ���������� � RS485_2
  *
  * @param  buf: ��������� �� ����� � �������, ������� ���� ���������
  *			size: ������ ������ ������� ���� ���������
  * @retval ��������� ������������ ������
  */
uint32_t rs485_2_write_tx_data(void* buf, uint32_t size)
{

	if (size==0) return(0);

	if (rs485_2_get_tx_status()) return(0); //���� �������� ��� ���, �� ������� 0

	if (size>sizeof(rs485_2_buf_tx)) size=sizeof(rs485_2_buf_tx);
	memcpy(rs485_2_buf_tx, (uint8_t*)buf, size);

#ifndef RS485_2_HFC_EN
	HAL_GPIO_WritePin(RS485_2_RE_PPORT, RS485_2_RE_PIN, GPIO_PIN_SET);
#endif
	HAL_UART_Transmit_DMA(&RS485_2_UART, rs485_2_buf_tx, size);

	return(size);
}

/**
  * @brief  ������� ������ �� ��������� ������� ������: ����� ������ �������� ���������� ��������� ������ � ��������� ������� ������ �� ��������� ��������
  *
  * @param  buf: ��������� �� ����� � �������, ������� ���� ���������
  *			size: ������ ������ ������� ���� ���������
  * @retval ��������� ������������ ������
  */
uint32_t rs485_2_direct_tx_data(void* buf, uint32_t size)
{

	if (size==0) return(0);

	if (rs485_2_get_tx_status()) return(0); //���� �������� ��� ���, �� ������� 0

#ifndef RS485_2_HFC_EN
	HAL_GPIO_WritePin(RS485_2_RE_PPORT, RS485_2_RE_PIN, GPIO_PIN_SET);
#endif
	HAL_UART_Transmit_DMA(&RS485_2_UART, (uint8_t*)buf, size);

	return(size);
}

/**
  * @brief  ���������� ��������� ����������
  *
  * @retval ���������� ���������� ��� �������� ������
  */
uint32_t rs485_2_get_tx_status(void)
{uint32_t dt;

	if (__HAL_UART_GET_FLAG(&RS485_2_UART, UART_FLAG_FE)||__HAL_UART_GET_FLAG(&RS485_2_UART, UART_FLAG_NE)||__HAL_UART_GET_FLAG(&RS485_2_UART, UART_FLAG_ORE))
		{
#ifndef RS485_2_HFC_EN
		HAL_GPIO_WritePin(RS485_2_RE_PPORT, RS485_2_RE_PIN, GPIO_PIN_RESET);
#endif
		__HAL_UART_FLUSH_DRREGISTER(&RS485_2_UART);
		rs485_2_clear_buf();
		return(1);
		}
#ifndef RS485_1_HFC_EN
	if (HAL_GPIO_ReadPin(RS485_2_RE_PPORT, RS485_2_RE_PIN)==GPIO_PIN_SET)
		{
#endif
		dt=__HAL_DMA_GET_COUNTER(&RS485_2_DMA_TX);
		if (dt==0)
			{
			if (__HAL_UART_GET_FLAG(&RS485_2_UART, UART_FLAG_TC)==RESET) return(1); //���� dt==0 ������ ��������� ���� ������� �� DMA � UART � � ������ ������ ��������� � �����, �� �������� ��� �� ���������
			else return(0);
			}
		else return(dt);
#ifndef RS485_2_HFC_EN
		}
	else
		return(0);
#endif
}

#ifndef RS485_2_HFC_EN
/**
  * @brief  ���������� ���������� UART: ������������� ���� ���������� ������� �� ����
  *
  *  @param  *huart: ��������� �� ��������� � ��������� UART
  *
  */
void rs485_2_irq(UART_HandleTypeDef *huart)
{uint32_t tmp1 = 0, tmp2 = 0;

	if (huart==&RS485_2_UART)
		{
		tmp1 = __HAL_UART_GET_FLAG(huart, UART_FLAG_TC);
		tmp2 = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TC);
		if((tmp1 != RESET) && (tmp2 != RESET))
			{
			HAL_GPIO_WritePin(RS485_2_RE_PPORT, RS485_2_RE_PIN, GPIO_PIN_RESET);
			}
		}
}
#endif