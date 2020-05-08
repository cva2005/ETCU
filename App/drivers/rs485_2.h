/*
 * rs485_2.h
 *
 *  Created on: 17 дек. 2015 г.
 *      Author: Перчиц А.Н.
 *      Драйвер RS-485 на UART-е
 *      v.2.0 изменения:
 *      - добавлена возможность работы с аппаратным контролем направления линии
 *      - DMA на приём теперь должен быть настроен как "cyclic"
 *      - устранена проблема с зависанием RS-485 при обноружениях ошибок на линии
 *      - добавлено отключение прерывания DMA при заполнении половины буфера RX
 */

#ifndef APP_DRIVERS_RS485_2_H_
#define APP_DRIVERS_RS485_2_H_
#include "stm32f4xx_hal.h"
//#include "main.h"
#include "mxconstants.h"

#define RS485_2_UART_NUMBER 1 //номер канала UART на котором работает RS485

//#define RS485_2_MODE_IRQ //если определена, то приём работает в режиме IRQ, иначе в режиме DMA
//#define RS485_2_HFC_EN		//если определена, то включён аппаратный контроль управления передачей "Hardware Flow Control"

#define RS485_2_BUF_SIZE_RX 4096		//размер буфера на приём
#define RS485_2_BUF_SIZE_TX 256			//размер буфера на передачу

#if RS485_2_UART_NUMBER==1
#define RS485_2_UART huart1				//UART на котором висит RS485
#define RS485_2_DMA_RX hdma_usart1_rx	//канал DMA который принимает данные
#define RS485_2_DMA_TX hdma_usart1_tx	//канал DMA который передаёт данные
#define DMA_RX_USART1_IRQ_POST 			rs485_2_reinit_dma()
#ifndef RS485_2_HFC_EN
#define UART1_IRQ_PRE					rs485_2_irq(&RS485_2_UART)
#endif
#ifdef RS485_2_MODE_IRQ
#define UART1_IRQ_POST					if (RS485_2_UART.RxXferCount==0) rs485_2_reinit_irq()
#endif
#endif
#if RS485_2_UART_NUMBER==2
#define RS485_2_UART huart2				//UART на котором висит RS485
#define RS485_2_DMA_RX hdma_usart2_rx	//канал DMA который принимает данные
#define RS485_2_DMA_TX hdma_usart2_tx	//канал DMA который передаёт данные
#define DMA_RX_USART2_IRQ_POST 				rs485_2_reinit_dma()//if(__HAL_DMA_GET_COUNTER(&RS485_2_DMA_RX)==0)	rs485_2_reinit_dma()
#ifndef RS485_2_HFC_EN
#define UART2_IRQ_PRE					rs485_2_irq(&RS485_2_UART)
#endif
#ifdef RS485_2_MODE_IRQ
#define UART2_IRQ_POST					if (RS485_2_UART.RxXferCount==0) rs485_2_reinit_irq()
#endif
#endif
#if RS485_2_UART_NUMBER==3
#define RS485_2_UART huart3				//UART на котором висит RS485
#define RS485_2_DMA_RX hdma_usart3_rx	//канал DMA который принимает данные
#define RS485_2_DMA_TX hdma_usart3_tx	//канал DMA который передаёт данные
#define DMA_RX_USART3_IRQ_POST 			rs485_2_reinit_dma()
#ifndef RS485_2_HFC_EN
#define UART3_IRQ_PRE					rs485_2_irq(&RS485_2_UART)
#endif
#ifdef RS485_2_MODE_IRQ
#define UART3_IRQ_POST					if (RS485_2_UART.RxXferCount==0) rs485_2_reinit_irq()
#endif
#endif

#ifndef RS485_2_HFC_EN
#define RS485_2_RE_PIN RS485_RE2_Pin	//номер ножки на порту где висит сигнал управления потоком
#define RS485_2_RE_PPORT RS485_RE2_GPIO_Port //порт на котором висит ножка управления потоком
#endif

void rs485_2_init (uint32_t speed);		//Инициализация интерфейса RS482 канал 2
void rs485_2_clear_buf(void);			//Очищает буфер приёма;
void* rs485_2_get_buf_pointer(void);	//Возвращает адрес буфера в памяти
uint32_t rs485_2_get_buf_shift(void);	//Возвращает смещение относительно начала буфера, куда в буфере будет записан следующий байт
void rs485_2_reinit_dma(void);			//Переинициализия буфера (установка указателя приёма на начало буфера) в режиме DMA
uint32_t rs485_2_get_buf_overflow(void);//Возвращает количество переполнений буфера
uint32_t rs485_2_read_rx_data(void* buf, uint32_t size);//Копирует из буфера принятые, но не обработанные внешней функцией данные
uint32_t rs485_2_get_rx_size(void); 	//Возвращает количество принятых, но не обработанных внешней функцией данных
uint32_t rs485_2_write_tx_data(void* buf, uint32_t size); //Копирует в буфер передачи данные и отправляет в RS485_2
uint32_t rs485_2_direct_tx_data(void* buf, uint32_t size);//передаёт данные из указанной области памяти: после начала передачи необходимо заблокировать запись в память до окончания передачи
uint32_t rs485_2_get_tx_status(void);	//Определяет состояние передачика: возвращает количество ещё не переданных байт
#ifdef RS485_2_MODE_IRQ
void rs485_2_reinit_irq(void);			//Переинициализия буфера (установка указателя приёма на начало буфера) в режиме IRQ
#endif
#ifndef RS485_2_HFC_EN
void rs485_2_irq(UART_HandleTypeDef *huart); //Обработчик прерывания UART
#endif

#endif /* APP_DRIVERS_RS485_2_H_ */
