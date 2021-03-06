/*
 * wifi_hf.c
 *
 *  Created on: 18 ���. 2015 �.
 *      Author: ������� �.�. ������ �.�.
 */
#include "wifi_hf.h"
#include <string.h>
#include "timers.h"
#ifdef WIFI_DEBUG
#include "rs485_1.h"
#endif

extern UART_HandleTypeDef WIFI_HF_UART;
#ifndef  WIFI_HF_MODE_IRQ
extern DMA_HandleTypeDef WIFI_HF_DMA_RX;
#endif
extern DMA_HandleTypeDef WIFI_HF_DMA_TX;

#define ATCOUNT 17		//���������� ������, ������������ �� wifi ������ ��� ������������
//���� ������������
const char* cfgCmd[ATCOUNT] = { "AT+\r\0","AT+\r\0",									//��������: �� ��������� �� ������ � ������ ��
								"+++\0",												//������ �� ���������� � �������� � ��-�����
								"a\0",													//������� � ��-�����
								"AT+E=off\r\0",											//���������� ��� �� ������
								"AT+TXPWR=0\r\0",										//���������� �������� ����������� �� 100% �� ������������ (Power = 16 - <value>* 0.5 dBm)
								"AT+WADMN=Config.en\r\0",								//���������� ����� WEB-���������� ��������� (http://Config.en)
								"AT+WADHCP=on,10,50\r\0",								//�������� DHCP � ���������� ��������� ���������� ������� (*.*.*.10-50)
								"AT+PLANG=EN\r\0",										//���������� ���� WEB-���������� ��������� (English)
								"AT+WMODE=AP\r\0",										//��������� ������ ����� �������
								//"AT+NDBGL=0\r\0",
								"AT+WALKIND=ON\r\0",									//������������ ��������� nLink
								"AT+MAXSK=2\r\0",										//������������ ���������� ����������� � TCP ������� (�� � ����� �������!)
								"AT+WAP=11BGN,"API_NAME",AUTO\r\0",						//��������� ����� �������: ��������� IEEE 802, ��� ����� �������, ����� ������
								"AT+WAKEY=WPA2PSK,AES,"API_PASS"\r\0",					//��������� ����� �������: ��� ������, ����� ����������, ������
								"AT+LANN="API_IP",255.255.255.0\r\0",					//��������� ����� �������: IP-����� ������� (WEB ��� ���������), ����� �������
								"AT+NETP=TCP,Server,"API_TCP_PORT","API_IP"\r\0",		//��������� �������: ��������, �����, ����, IP
								"AT+Z\r\0"												//������������ ������ ��� ���������� ��������
								};
//�������� ������������
const uint16_t cfgTo[ATCOUNT] = {500,500,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};

//���������� ������ �� ��-�������
const char* cfgAns[ATCOUNT] = {"+ok\0","+ok\0","a\0","+ok\0","AT+E=off\0","+ok\0","+ok\0","+ok\0","+ok\0","+ok\0","+ok\0","+ok\0","+ok\0","+ok\0","+ok\0","+ok\0","+ok\0"};

static uint8_t SPW_ST = 0; 	 				//��������� wi-fi ������
static uint8_t init_step_WF = 0; 				//��� ���������� �������� wi-fi
static stime_t timeout_WF;				//������� ��-�������
static uint8_t msg_rx_WF = 0;	 				//����: ���� ��������� �� wi-fi ������
static uint8_t expect_mes_len = 3;				//��������� ������ �������� �� wi-fi ������
static uint8_t error_WF = 0; 					//������ ��� ������ � wi-fi
static stime_t timeout_rx;				//������� �������� ���������� �������
static unsigned char cmd_WF[MAX_CMD_LEN];		//������ ������ ��������� �� ������
static uint8_t point_cmd_WF = 0;


static uint8_t wifi_hf_buf[WIFI_HF_BUF_SIZE_RX]; //����� ��� ����� �����
static uint8_t wifi_hf_buf_tx[WIFI_HF_BUF_SIZE_TX]; //����� ��� �������� �����
static uint8_t wifi_hf_buf_overflow;		  //������� ������������ ������
static uint32_t wifi_hf_buf_read_pointer; //��������� � ������ �� ������ ������� ��� �� ����������

/**
  * @brief  ���������� ����� ������, ����� ������ ���� �� ������� ������� '\0'
  *
  * @param  *str: ��������� �� ������
  *
  * @retval ����� ������
  */
uint8_t wifi_hf_str_len(unsigned char* str)
{
	uint8_t len = 0;
	while ((str[len] != '\0') && (len < MAX_CMD_LEN)) len++;
	return len;
}

/**
  * @brief  ���������� ������� ������ Wi-Fi
  *
  * @param  *cmd: ��������� �� �������
  *
  * @retval ���������� ������������ ������
  */
uint8_t wifi_hf_send_cmd(unsigned char* cmd)
{
	//if (!transmitStr) return 0;
	uint8_t len = wifi_hf_str_len(cmd);

#ifdef WIFI_DEBUG
	while (rs485_1_get_tx_status()!=0);
	rs485_1_write_tx_data(cmd, len);
#endif

	//transmitStr(cmd, len);
	return (wifi_hf_write_tx_data(cmd, len));
}

/**
  * @brief  ���������� ���� ������������ ������ wifi
  */
void wifi_hf_step(void)
{uint8_t byte;

	if (SPW_ST==SPW_DATA_MODE) //���� � ������ �������� ��������, �� � �������� "��������"
		{
		return;
		}
	else
		{
		if (wifi_hf_get_rx_size())
			{
			wifi_hf_read_rx_data(&byte, 1);
			if ((byte == 0x00) || (byte == 0x0a || byte == 0x0d)) return; //�������� �������� �������
			else
				{
				cmd_WF[point_cmd_WF++]=byte; //��������� ���� � ������
				if (point_cmd_WF >= MAX_CMD_LEN)  point_cmd_WF = 0;
				msg_rx_WF = (point_cmd_WF >= expect_mes_len)?1:0;
				timeout_rx = timers_get_finish_time(10); //�������� ���������� �����. ���� �� ��������� - ����� �������.
				}
			}
		}

	switch (SPW_ST)
		{
//------------------------- ������������� ������
		case SPW_WAIT_INIT:
			{
			if ((error_WF == ERR_NONE) || ((init_step_WF < 2) && (error_WF == ERR_AT_TIMEOUT))) init_step_WF++;
				else
					{
					if ((init_step_WF == 3))init_step_WF--; //���� �� ��������� � �� �����, ����������� ������� ��� ���
					}

				wifi_hf_clear_buf();
				if (!wifi_hf_send_cmd((uint8_t*) cfgCmd[init_step_WF])) error_WF = ERR_AT_SEND;
				else
					{
					if (init_step_WF >= (ATCOUNT-1)) SPW_ST = SPW_DATA_MODE; //��������� ��� ������������
					else
						{
						timeout_WF = timers_get_finish_time(cfgTo[init_step_WF]);//GetCntVal(cfgTo[init_step_WF]);
						expect_mes_len = wifi_hf_str_len((uint8_t*) cfgAns[init_step_WF]);
						SPW_ST = SPW_WAIT_AT_RESPONSE;
						}
					}
			break;
			}

//------------------------- ������� ������ �� ��-�������
		case SPW_WAIT_AT_RESPONSE:
			{

			if (timers_get_time_left(timeout_WF)==0) //���� ������� ����� �������� ����������� � ����
				{
				error_WF = ERR_AT_TIMEOUT;
				SPW_ST = SPW_WAIT_INIT;
				point_cmd_WF = 0;
				msg_rx_WF=0;
				}

			if ((timers_get_time_left(timeout_rx) == 0) && ((point_cmd_WF >= expect_mes_len)))
				{
#ifdef WIFI_DEBUG
				uint8_t mess[255];
				memset(mess,0,255);
				strcpy(mess,"Rx: ");
				strcat(mess,cmd_WF);
				strcat(mess,"\r\n");
				while (rs485_1_get_tx_status()!=0);
				rs485_1_write_tx_data(mess,strlen(mess));
#endif
				error_WF = ((strncasecmp(cmd_WF, cfgAns[init_step_WF], expect_mes_len) == 0)?ERR_NONE:ERR_AT_RESPONSE);
				SPW_ST = SPW_WAIT_INIT;
				point_cmd_WF = 0;
				msg_rx_WF=0;
				memset(cmd_WF,0,MAX_CMD_LEN);
				}

			break;
			}
		}
}

/**
  * @brief  ������������� ���������� RS482 ����� 2
  *
  * @param  speed: �������� ������ � ���/�
  */
void wifi_hf_init (uint32_t speed)
{stime_t wifi_init_timeout;
//������������� UART
	WIFI_HF_UART.Init.BaudRate = speed;
	HAL_UART_Init(&WIFI_HF_UART);
//��������� ����� �����
	wifi_hf_clear_buf();


	HAL_GPIO_WritePin(WIFI_HF_RST, GPIO_PIN_SET);


	if (HAL_GPIO_ReadPin(WIFI_HF_BOOT_EN)==GPIO_PIN_SET)
		{
		SPW_ST = SPW_WAIT_INIT;
		error_WF = ERR_AT_RESPONSE; //��� ������ � ������������. ����� ��������� ������ ��� � �������� ����� �� �������.
		init_step_WF = 0;
		}
	else
		SPW_ST = SPW_DATA_MODE;

	timeout_WF=timers_get_finish_time(0);
	timeout_rx=timers_get_finish_time(0);

	wifi_init_timeout=timers_get_finish_time(5000);

	while ((SPW_ST!=SPW_DATA_MODE)&&(timers_get_time_left(wifi_init_timeout)>0))
		{
		wifi_hf_step();
		}
return;
}

/**
  * @brief  ������� ����� �����
  */

void wifi_hf_clear_buf(void)
{
#ifdef WIFI_HF_MODE_IRQ
	wifi_hf_reinit_irq();
#else
	HAL_UART_DMAStop(&WIFI_HF_UART);
	wifi_hf_reinit_dma();
#endif
	wifi_hf_buf_overflow=0;
	wifi_hf_buf_read_pointer=wifi_hf_get_buf_shift();
}

/**
  * @brief  ���������� ����� ������ � ������
  *
  * @retval �����
  */
void* wifi_hf_get_buf_pointer(void)
{
	return(wifi_hf_buf);
}

/**
  * @brief  ���������� �������� ������������ ������������ ������ ������ ���� � ������ ����� ������� ��������� ����
  *
  * @retval �������� �� 0 �� RS485_2_BUF_SIZE-1
  */
uint32_t wifi_hf_get_buf_shift(void)
{
#ifdef WIFI_HF_MODE_IRQ
	return(sizeof(wifi_hf_buf)-WIFI_HF_UART.RxXferCount);
#else
	return(sizeof(wifi_hf_buf)-__HAL_DMA_GET_COUNTER(&WIFI_HF_DMA_RX));
#endif
}

/**
  * @brief  ��������������� ������ (��������� ��������� ����� �� ������ ������) ��� ������ � DMA ������
  *
  */
void wifi_hf_reinit_dma(void)
{
#ifndef WIFI_HF_MODE_IRQ
	HAL_UART_Receive_DMA(&WIFI_HF_UART, wifi_hf_buf, sizeof(wifi_hf_buf));
	wifi_hf_buf_overflow++;
#endif
}

/**
  * @brief  ��������������� ������ (��������� ��������� ����� �� ������ ������) ��� ������ � IRQ ������
  *
  */
void wifi_hf_reinit_irq(void)
{
#ifdef WIFI_HF_MODE_IRQ
	HAL_UART_Receive_IT(&WIFI_HF_UART, wifi_hf_buf, sizeof(wifi_hf_buf));
	wifi_hf_buf_overflow++;
#endif
}

/**
  * @brief  ���������� ���������� ������������ ������
  *
  * @retval ���������� ������������
  */
uint32_t wifi_hf_get_buf_overflow(void)
{
	return(wifi_hf_buf_overflow);
}

/**
  * @brief  �������� �� ������ ��������, �� �� ������������ ������
  *
  * @param  buf: ��������� �� �����, ���� ���� ����������� ������
  *			size: ������ ������ ������� ���� �����������
  * @retval ��������� ������������� ������
  */
uint32_t wifi_hf_read_rx_data(void* buf, uint32_t size)
{uint32_t cpy_size=0, rx_point, buf_point=0;

	rx_point=wifi_hf_get_buf_shift();

	if (rx_point==wifi_hf_buf_read_pointer) return(0);

	if (rx_point<wifi_hf_buf_read_pointer) //���� ��������� ����� ��������� ����� ����� ������
		{
		cpy_size=sizeof(wifi_hf_buf)-wifi_hf_buf_read_pointer; //��������� ���������� �� ����������� ������ �� ����� ������
		if (cpy_size>size) cpy_size=size;				//���� ���������� ������ ������ ��� ��������������� ��� ������ �����
		memcpy((uint8_t*)buf, &wifi_hf_buf[wifi_hf_buf_read_pointer], cpy_size);
		wifi_hf_buf_read_pointer+=cpy_size; //����������� ��������� ����������� ������
		if (wifi_hf_buf_read_pointer>=sizeof(wifi_hf_buf)) wifi_hf_buf_read_pointer=0; //���� �������� ��� ������ �� ����� �������� ������, �� ��������� ����������� ������ ����������� �� 0
		if (wifi_hf_buf_read_pointer!=0) return (cpy_size); //���� ������ ���������������� ������ �� �������� ����������� ��� ������ �� ����� �������� ������, �� ����� �.�. ������ ���������� ������
		buf_point=cpy_size;								//� ������ ��� ����������� ����������� ���������
		size-=cpy_size;									//��������� ������ ������ ��� ���������� �� ����� ������������� ������
		}

	cpy_size=rx_point-wifi_hf_buf_read_pointer; //��������� ���������� �� ����������� ������
	if (cpy_size>size) cpy_size=size;				//���� ���������� ������ ������ ��� ��������������� ��� ������ �����
	memcpy(&((uint8_t*)buf)[buf_point], &wifi_hf_buf[wifi_hf_buf_read_pointer], cpy_size);
	wifi_hf_buf_read_pointer+=cpy_size;

	return(cpy_size+buf_point);
}

/**
  * @brief  ���������� ���������� ��������, �� �� ������������ ������� �������� ������
  *
  * @retval ���������� ������
  */
uint32_t wifi_hf_get_rx_size(void)
{uint32_t sz=0, rx_point, pt;

	rx_point=wifi_hf_get_buf_shift();
	pt=wifi_hf_buf_read_pointer;

	if (rx_point<wifi_hf_buf_read_pointer) //���� ��������� ����� ��������� ����� ����� ������
		{
		sz=sizeof(wifi_hf_buf)-wifi_hf_buf_read_pointer;
		pt=0;
		}
	sz+=(rx_point-pt);

	return(sz);
}

/**
  * @brief  �������� � ����� �������� ������ � ���������� � WiFi
  *
  * @param  buf: ��������� �� ����� � �������, ������� ���� ���������
  *			size: ������ ������ ������� ���� ���������
  * @retval ��������� ������������ ������
  */
uint32_t wifi_hf_write_tx_data(void* buf, uint32_t size)
{
	if (size==0) return(0);

	if (wifi_hf_get_tx_status()) return(0); //���� �������� ��� ���, �� ������� 0

	if (size>sizeof(wifi_hf_buf_tx)) size=sizeof(wifi_hf_buf_tx);
	memcpy(wifi_hf_buf_tx, (uint8_t*)buf, size);

	HAL_UART_Transmit_DMA(&WIFI_HF_UART, wifi_hf_buf_tx, size);

	return(size);
}

/**
  * @brief  ������� ������ �� ��������� ������� ������: ����� ������ �������� ���������� ��������� ������ � ��������� ������� ������ �� ��������� ��������
  *
  * @param  buf: ��������� �� ����� � �������, ������� ���� ���������
  *			size: ������ ������ ������� ���� ���������
  * @retval ��������� ������������ ������
  */
uint32_t wifi_hf_direct_tx_data(void* buf, uint32_t size)
{
	if (size==0) return(0);

	if (wifi_hf_get_tx_status()) return(0); //���� �������� ��� ���, �� ������� 0

	HAL_UART_Transmit_DMA(&WIFI_HF_UART, (uint8_t*)buf, size);

	return(size);
}

/**
  * @brief  ���������� ��������� ����������
  *
  * @retval ���������� ���������� ��� �������� ������
  */
uint32_t wifi_hf_get_tx_status(void)
{uint32_t dt;
	dt=__HAL_DMA_GET_COUNTER(&WIFI_HF_DMA_TX);
	if (dt==0)
		{
		if (__HAL_UART_GET_FLAG(&WIFI_HF_UART, UART_FLAG_TC)==RESET) return(1);
		else return(0);
		}
	else
		return(dt);
	//return(__HAL_DMA_GET_COUNTER(&WIFI_HF_DMA_TX));
}
