/*
 * can_2.h
 *
 *  Created on: 5 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DRIVERS_CAN_2_H_
#define APP_DRIVERS_CAN_2_H_
#include "stm32f4xx_hal.h"

#define CAN_2_BUF_SIZE_RX 50		//размер буфера на приём (в пакетах)
#define CAN_2_START_FILTER 14		//Начальный номер фильтра для CAN2, номер этого фильтра всегда должен быть больше на 1 чем номер начального фильтра для CAN2 (CAN_2_START_FILTER>CAN_1_END_FILTER)
#define CAN_ALL_FILTERS 28			//Общее число фильтров CAN для данного микроконтроллера, всегда больше чем CAN_2_START_FILTER (CAN_2_START_FILTER<CAN_ALL_FILTERS)
#define CAN2_TX_TIMEOUT 10		//максимально-допустимое время для отправки пакета

#define CAN_2 hcan2				//Физический канал на котором висит CAN
#define CAN_2_RX0_IRQ 		can_2_rx0_irq(&CAN_2); //обработчик прерывания CAN FIFO0
#define CAN_2_RX1_IRQ 		can_2_rx1_irq(&CAN_2); //обработчик прерывания CAN FIFO1


void can_2_init(uint8_t prescaler);					//Инициализация интерфейса CAN канал 1
uint8_t can_2_set_filter32(uint32_t id, uint8_t ide_rtr1, uint32_t id_mask, uint8_t ide_rtr2, uint8_t type);//Установка 32-битного фильтра на приём (устанавливается сразу 2 фильтра или маску)
void can_2_clear_buf(void);				//Очищает буфер приёма
void* can_2_get_buf_pointer(void);		//Возвращает адрес буфера в памяти
uint32_t can_2_get_buf_shift(void);		//возвращает  номер элемента буфера куда будет записан следующий байт
uint32_t can_2_get_buf_overflow(void);	//Возвращает количество переполнений буфера
uint8_t can_2_read_rx_data(uint32_t* id, uint8_t* length, uint8_t* data); //Копирует из буфера принятые, но не обработанные данные
uint32_t can_2_get_rx_size(void);		//Возвращает количество принятых, но не обработанных внешней функцией пакетов
uint8_t can_2_write_tx_data(uint32_t id, uint8_t length, uint8_t* data);//Копирует в буфер передачи данные и отправляет в CAN
void can_2_buf_point_move(void);	//Перемещение указателя в приёмноом буфере на следующее свободное место

void can_2_rx0_irq(CAN_HandleTypeDef *hcan);//Обработчик прерывания CAN1 очереди FIFO0
void can_2_rx1_irq(CAN_HandleTypeDef *hcan);//Обработчик прерывания CAN1 очереди FIFO1

//-------------------------CAN BUS SPEED-------------------------------------------
#define CAN_2_SPEED_20K 300
#define CAN_2_SPEED_125K 48
#define CAN_2_SPEED_250K 24
#define CAN_2_SPEED_500K 12
#define CAN_2_SPEED_1000K 6

#endif /* APP_DRIVERS_CAN_1_H_ */
