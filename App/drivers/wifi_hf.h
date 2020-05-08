/*
 * wifi_hf.h
 *
 *  Created on: 18 дек. 2015 г.
 *      Author: Руденко Р.Л. Перчиц А.Н.
 *      rev 2.0
 */

#ifndef APP_DRIVERS_WIFI_HF_H_
#define APP_DRIVERS_WIFI_HF_H_
#include "stm32f4xx_hal.h"
#include "mxconstants.h"

#define WIFI_HF_UART_NUMBER 3 //номер канала UART на котором работает WiFi модуль

//#define WIFI_DEBUG

//#define WIFI_HF_MODE_IRQ //если определена, то приём работает в режиме IRQ, иначе в режиме DMA

#define WIFI_HF_BUF_SIZE_RX 8192		//размер буфера на приём
#define WIFI_HF_BUF_SIZE_TX 2048		//размер буфера на передачу

#if WIFI_HF_UART_NUMBER==1
#define WIFI_HF_UART huart1				//UART на котором висит WIFI
#define WIFI_HF_DMA_RX hdma_usart1_rx	//канал DMA который принимает данные
#define WIFI_HF_DMA_TX hdma_usart1_tx	//канал DMA который передаёт данные
#define DMA_RX_USART1_IRQ 				if(__HAL_DMA_GET_COUNTER(&WIFI_HF_DMA_RX)==0) wifi_hf_reinit_dma() //имя обработчика прерывания на DMA_RX
#define UART1_IRQ_POST					if (WIFI_HF_UART.RxXferCount==0) wifi_hf_reinit_irq()
#endif
#if WIFI_HF_UART_NUMBER==2
#define WIFI_HF_UART huart2				//UART на котором висит WIFI
#define WIFI_HF_DMA_RX hdma_usart2_rx	//канал DMA который принимает данные
#define WIFI_HF_DMA_TX hdma_usart2_tx	//канал DMA который передаёт данные
#define DMA_RX_USART2_IRQ 				if(__HAL_DMA_GET_COUNTER(&WIFI_HF_DMA_RX)==0) wifi_hf_reinit_dma() //имя обработчика прерывания на DMA_RX
#define UART2_IRQ_POST					if (WIFI_HF_UART.RxXferCount==0) wifi_hf_reinit_irq()
#endif
#if WIFI_HF_UART_NUMBER==3
#define WIFI_HF_UART huart3				//UART на котором висит WIFI
#define WIFI_HF_DMA_RX hdma_usart3_rx	//канал DMA который принимает данные
#define WIFI_HF_DMA_TX hdma_usart3_tx	//канал DMA который передаёт данные
#define DMA_RX_USART3_IRQ 				if(__HAL_DMA_GET_COUNTER(&WIFI_HF_DMA_RX)==0) wifi_hf_reinit_dma() //имя обработчика прерывания на DMA_RX
#define UART3_IRQ_POST					if (WIFI_HF_UART.RxXferCount==0) wifi_hf_reinit_irq()
#endif

#define WIFI_HF_RST WIFI_RST_GPIO_Port, WIFI_RST_Pin //порт и ножка управления сборосом wifi модуля
#define WIFI_HF_BOOT_EN WF_BOOT_EN_GPIO_Port, WF_BOOT_EN_Pin //порт и ножка выбора режима конфигурации wifi модуля


void wifi_hf_init (uint32_t speed);		//Инициализация интерфейса WIFI
void wifi_hf_clear_buf(void);			//Очищает буфер приёма;
void* wifi_hf_get_buf_pointer(void);	//Возвращает адрес буфера в памяти
uint32_t wifi_hf_get_buf_shift(void);	//Возвращает смещение относительно начала буфера, куда в буфере будет записан следующий байт
void wifi_hf_reinit_dma(void);			//Переинициализия буфера (установка указателя приёма на начало буфера) в режиме DMA
void wifi_hf_reinit_irq(void);			//Переинициализия буфера (установка указателя приёма на начало буфера) в режиме IRQ
uint32_t rs485_1_get_buf_overflow(void);//Возвращает количество переполнений буфера
uint32_t wifi_hf_read_rx_data(void* buf, uint32_t size);//Копирует из буфера принятые, но не обработанные внешней функцией данные
uint32_t wifi_hf_get_rx_size(void); 	//Возвращает количество принятых, но не обработанных внешней функцией данных
uint32_t wifi_hf_write_tx_data(void* buf, uint32_t size); //Копирует в буфер передачи данные и отправляет в WiFi
uint32_t wifi_hf_direct_tx_data(void* buf, uint32_t size);//Передаёт данные из указанной области памяти: после начала передачи необходимо заблокировать запись в память до окончания передачи
uint32_t wifi_hf_get_tx_status(void);	//Определяет состояние передачика: возвращает количество ещё не переданных байт

uint8_t wifi_hf_str_len(unsigned char* str);		//Возвращает длину строки, конец строки ищет по наличию символа '\0'
uint8_t wifi_hf_send_cmd(unsigned char* cmd);		//Отправляет команду модулю Wi-Fi
void wifi_hf_step(void); 							//Выполнение шага конфигурации модуля wifi

//void SaveInRxBuf_WF(uint8_t byte);															//Функция обработки полученных байт. Указатель на нее надо передать модулю USART.
//uint8_t wifiReadyTx();																		//Функция определения готовности модуля передать данные
//void transmitDataWiFi(unsigned char* data, int len);										//Функция передачи данных
//uint8_t wifiInDataMode();																	//1- модуль в режиме обмена, 0- в режиме конфигурирования

#define MAX_CMD_LEN 50			//максимальный размер команды при конфигурировании

//Параметры точки доступа
#define API_NAME "KRON_ETCU"
#define API_PASS "Kron2015"
#define API_IP "192.168.1.1"
#define API_TCP_PORT "5000"


// Перечень режимов драйвера
enum {
		SPW_WAIT_INIT,				//Ожидаем инициализации wi-fi модуля
		SPW_DATA_MODE,				//WiFi в режиме передачи данных
		SPW_WAIT_AT_RESPONSE,		//Ожидание ответа на АТ-команду
		SPW_WAIT_AT_MODE			//Ожидание готовности к переходу в АТ-режим
	};

//Ошибка драйвера
enum {
		ERR_NONE,
		ERR_AT_TIMEOUT,				//Таймаут ожидания ответа на АТ-команду
		ERR_AT_SEND,				//Ошибка отправки АТ-команды
		ERR_AT_RESPONSE,			//В ответ на АТ-команду модуль вернул ошибку
	};

#endif /* APP_DRIVERS_WIFI_HF_H_ */
