/*
 * rs485_1.h
 *
 *  Created on: 18 дек. 2015 г.
 *      Author: Перчиц А.Н.
 *      Драйвер RS-485 на UART-е
 *      v.2.0 изменения:
 *      - добавлена возможность работы с аппаратным контролем направления линии
 *      - DMA на приём теперь должен быть настроен как "cyclic"
 *      - устранена проблема с зависанием RS-485 при обноружениях ошибок на линии
 *      - добавлено отключение прерывания DMA при заполнении половины буфера RX
 */

#ifndef APP_DRIVERS_RS485_1_H_
#define APP_DRIVERS_RS485_1_H_
#include "stm32f4xx_hal.h"
//#include "main.h"
#include "mxconstants.h"

#define RS485_1_UART_NUMBER 2 //номер канала UART на котором работает RS485

//#define RS485_1_MODE_IRQ //если определена, то приём работает в режиме IRQ, иначе в режиме DMA
//#define RS485_1_HFC_EN		//если определена, то включён аппаратный контроль управления передачей "Hardware Flow Control"

#define RS485_1_BUF_SIZE_RX 4096		//размер буфера на приём
#define RS485_1_BUF_SIZE_TX 256			//размер буфера на передачу

#if RS485_1_UART_NUMBER==1
#define RS485_1_UART huart1				//UART на котором висит RS485
#define RS485_1_DMA_RX hdma_usart1_rx	//канал DMA который принимает данные
#define RS485_1_DMA_TX hdma_usart1_tx	//канал DMA который передаёт данные
#define DMA_RX_USART1_IRQ_POST 			rs485_1_reinit_dma()
#ifndef RS485_1_HFC_EN
#define UART1_IRQ_PRE					rs485_1_irq(&RS485_1_UART)
#endif
#ifdef RS485_1_MODE_IRQ
#define UART1_IRQ_POST					if (RS485_1_UART.RxXferCount==0) rs485_1_reinit_irq()
#endif
#endif
#if RS485_1_UART_NUMBER==2
#define RS485_1_UART huart2				//UART на котором висит RS485
#define RS485_1_DMA_RX hdma_usart2_rx	//канал DMA который принимает данные
#define RS485_1_DMA_TX hdma_usart2_tx	//канал DMA который передаёт данные
#define DMA_RX_USART2_IRQ_POST 				rs485_1_reinit_dma()//if(__HAL_DMA_GET_COUNTER(&RS485_1_DMA_RX)==0)	rs485_1_reinit_dma()
#ifndef RS485_1_HFC_EN
#define UART2_IRQ_PRE					rs485_1_irq(&RS485_1_UART)
#endif
#ifdef RS485_1_MODE_IRQ
#define UART2_IRQ_POST					if (RS485_1_UART.RxXferCount==0) rs485_1_reinit_irq()
#endif
#endif
#if RS485_1_UART_NUMBER==3
#define RS485_1_UART huart3				//UART на котором висит RS485
#define RS485_1_DMA_RX hdma_usart3_rx	//канал DMA который принимает данные
#define RS485_1_DMA_TX hdma_usart3_tx	//канал DMA который передаёт данные
#define DMA_RX_USART3_IRQ_POST 			rs485_1_reinit_dma()
#ifndef RS485_1_HFC_EN
#define UART3_IRQ_PRE					rs485_1_irq(&RS485_1_UART)
#endif
#ifdef RS485_1_MODE_IRQ
#define UART3_IRQ_POST					if (RS485_1_UART.RxXferCount==0) rs485_1_reinit_irq()
#endif
#endif

#ifndef RS485_1_HFC_EN
#define RS485_1_RE_PIN RS485_RE1_Pin	//номер ножки на порту где висит сигнал управления потоком
#define RS485_1_RE_PPORT RS485_RE1_GPIO_Port //порт на котором висит ножка управления потоком
#endif

void rs485_1_reinit (uint32_t speed);
void rs485_1_init (uint32_t speed);		//Инициализация интерфейса RS482 канал 1
void rs485_1_clear_buf(void);			//Очищает буфер приёма;
void* rs485_1_get_buf_pointer(void);	//Возвращает адрес буфера в памяти
uint32_t rs485_1_get_buf_shift(void);	//Возвращает смещение относительно начала буфера, куда в буфере будет записан следующий байт
void rs485_1_reinit_dma(void);			//Переинициализия буфера (установка указателя приёма на начало буфера) в DMA режиме
uint32_t rs485_1_get_buf_overflow(void);//Возвращает количество переполнений буфера
uint32_t rs485_1_read_rx_data(void* buf, uint32_t size);//Копирует из буфера принятые, но не обработанные внешней функцией данные
uint32_t rs485_1_get_rx_size(void); 	//Возвращает количество принятых, но не обработанных внешней функцией данных
uint32_t rs485_1_write_tx_data(void* buf, uint32_t size); //Копирует в буфер передачи данные и отправляет в RS485_1
uint32_t rs485_1_direct_tx_data(void* buf, uint32_t size);//передаёт данные из указанной области памяти: после начала передачи необходимо заблокировать запись в память до окончания передачи
uint32_t rs485_1_get_tx_status(void);	//Определяет состояние передачика: возвращает количество ещё не переданных байт
#ifdef RS485_1_MODE_IRQ
void rs485_1_reinit_irq(void);			//Переинициализия буфера (установка указателя приёма на начало буфера) в IRQ режиме
#endif
#ifndef RS485_1_HFC_EN
void rs485_1_irq(UART_HandleTypeDef *huart); //Обработчик прерывания UART
#endif


/**
  * @brief  Returns the number of remaining data units in the current DMAy Streamx transfer.
  * @param  __HANDLE__: DMA handle
  *
  * @retval The number of remaining data units in the current DMA Stream transfer.
  */
//#define __HAL_DMA_GET_COUNTER(__HANDLE__) ((__HANDLE__)->Instance->CNDTR)

#endif /* APP_DRIVERS_RS485_1_H_ */
