/*
 * wifi_hf.h
 *
 *  Created on: 18 ���. 2015 �.
 *      Author: ������� �.�. ������ �.�.
 *      rev 2.0
 */

#ifndef APP_DRIVERS_WIFI_HF_H_
#define APP_DRIVERS_WIFI_HF_H_
#include "stm32f4xx_hal.h"
#include "mxconstants.h"

#define WIFI_HF_UART_NUMBER 3 //����� ������ UART �� ������� �������� WiFi ������

//#define WIFI_DEBUG

//#define WIFI_HF_MODE_IRQ //���� ����������, �� ���� �������� � ������ IRQ, ����� � ������ DMA

#define WIFI_HF_BUF_SIZE_RX 8192		//������ ������ �� ����
#define WIFI_HF_BUF_SIZE_TX 2048		//������ ������ �� ��������

#if WIFI_HF_UART_NUMBER==1
#define WIFI_HF_UART huart1				//UART �� ������� ����� WIFI
#define WIFI_HF_DMA_RX hdma_usart1_rx	//����� DMA ������� ��������� ������
#define WIFI_HF_DMA_TX hdma_usart1_tx	//����� DMA ������� ������� ������
#define DMA_RX_USART1_IRQ 				if(__HAL_DMA_GET_COUNTER(&WIFI_HF_DMA_RX)==0) wifi_hf_reinit_dma() //��� ����������� ���������� �� DMA_RX
#define UART1_IRQ_POST					if (WIFI_HF_UART.RxXferCount==0) wifi_hf_reinit_irq()
#endif
#if WIFI_HF_UART_NUMBER==2
#define WIFI_HF_UART huart2				//UART �� ������� ����� WIFI
#define WIFI_HF_DMA_RX hdma_usart2_rx	//����� DMA ������� ��������� ������
#define WIFI_HF_DMA_TX hdma_usart2_tx	//����� DMA ������� ������� ������
#define DMA_RX_USART2_IRQ 				if(__HAL_DMA_GET_COUNTER(&WIFI_HF_DMA_RX)==0) wifi_hf_reinit_dma() //��� ����������� ���������� �� DMA_RX
#define UART2_IRQ_POST					if (WIFI_HF_UART.RxXferCount==0) wifi_hf_reinit_irq()
#endif
#if WIFI_HF_UART_NUMBER==3
#define WIFI_HF_UART huart3				//UART �� ������� ����� WIFI
#define WIFI_HF_DMA_RX hdma_usart3_rx	//����� DMA ������� ��������� ������
#define WIFI_HF_DMA_TX hdma_usart3_tx	//����� DMA ������� ������� ������
#define DMA_RX_USART3_IRQ 				if(__HAL_DMA_GET_COUNTER(&WIFI_HF_DMA_RX)==0) wifi_hf_reinit_dma() //��� ����������� ���������� �� DMA_RX
#define UART3_IRQ_POST					if (WIFI_HF_UART.RxXferCount==0) wifi_hf_reinit_irq()
#endif

#define WIFI_HF_RST WIFI_RST_GPIO_Port, WIFI_RST_Pin //���� � ����� ���������� �������� wifi ������
#define WIFI_HF_BOOT_EN WF_BOOT_EN_GPIO_Port, WF_BOOT_EN_Pin //���� � ����� ������ ������ ������������ wifi ������


void wifi_hf_init (uint32_t speed);		//������������� ���������� WIFI
void wifi_hf_clear_buf(void);			//������� ����� �����;
void* wifi_hf_get_buf_pointer(void);	//���������� ����� ������ � ������
uint32_t wifi_hf_get_buf_shift(void);	//���������� �������� ������������ ������ ������, ���� � ������ ����� ������� ��������� ����
void wifi_hf_reinit_dma(void);			//��������������� ������ (��������� ��������� ����� �� ������ ������) � ������ DMA
void wifi_hf_reinit_irq(void);			//��������������� ������ (��������� ��������� ����� �� ������ ������) � ������ IRQ
uint32_t rs485_1_get_buf_overflow(void);//���������� ���������� ������������ ������
uint32_t wifi_hf_read_rx_data(void* buf, uint32_t size);//�������� �� ������ ��������, �� �� ������������ ������� �������� ������
uint32_t wifi_hf_get_rx_size(void); 	//���������� ���������� ��������, �� �� ������������ ������� �������� ������
uint32_t wifi_hf_write_tx_data(void* buf, uint32_t size); //�������� � ����� �������� ������ � ���������� � WiFi
uint32_t wifi_hf_direct_tx_data(void* buf, uint32_t size);//������� ������ �� ��������� ������� ������: ����� ������ �������� ���������� ������������� ������ � ������ �� ��������� ��������
uint32_t wifi_hf_get_tx_status(void);	//���������� ��������� ����������: ���������� ���������� ��� �� ���������� ����

uint8_t wifi_hf_str_len(unsigned char* str);		//���������� ����� ������, ����� ������ ���� �� ������� ������� '\0'
uint8_t wifi_hf_send_cmd(unsigned char* cmd);		//���������� ������� ������ Wi-Fi
void wifi_hf_step(void); 							//���������� ���� ������������ ������ wifi

//void SaveInRxBuf_WF(uint8_t byte);															//������� ��������� ���������� ����. ��������� �� ��� ���� �������� ������ USART.
//uint8_t wifiReadyTx();																		//������� ����������� ���������� ������ �������� ������
//void transmitDataWiFi(unsigned char* data, int len);										//������� �������� ������
//uint8_t wifiInDataMode();																	//1- ������ � ������ ������, 0- � ������ ����������������

#define MAX_CMD_LEN 50			//������������ ������ ������� ��� ����������������

//��������� ����� �������
#define API_NAME "KRON_ETCU"
#define API_PASS "Kron2015"
#define API_IP "192.168.1.1"
#define API_TCP_PORT "5000"


// �������� ������� ��������
enum {
		SPW_WAIT_INIT,				//������� ������������� wi-fi ������
		SPW_DATA_MODE,				//WiFi � ������ �������� ������
		SPW_WAIT_AT_RESPONSE,		//�������� ������ �� ��-�������
		SPW_WAIT_AT_MODE			//�������� ���������� � �������� � ��-�����
	};

//������ ��������
enum {
		ERR_NONE,
		ERR_AT_TIMEOUT,				//������� �������� ������ �� ��-�������
		ERR_AT_SEND,				//������ �������� ��-�������
		ERR_AT_RESPONSE,			//� ����� �� ��-������� ������ ������ ������
	};

#endif /* APP_DRIVERS_WIFI_HF_H_ */