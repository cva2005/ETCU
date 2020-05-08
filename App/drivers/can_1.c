/*
 * can_1.c
 *
 *  Created on: 5 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "can_1.h"
#include <string.h>

extern CAN_HandleTypeDef CAN_1;

static CanTxMsgTypeDef can_1_tx;
static CanRxMsgTypeDef can_1_buf[CAN_1_BUF_SIZE_RX];

static uint8_t can_1_buf_overflow;		  //счётчик переполнений буфера
static uint32_t can_1_buf_read_pointer; //указатель в буфере на данные которые ещё не обработаны
static uint32_t can_1_buf_write_pointer; //указатель в буфере на данные куда будет записан следующий пакет

static uint8_t can_1_fnumber; //номер фильтра Дл инициализации
static uint8_t can_1_ffifo; //номер FIFO на котором надо инициализировать фильтр

/**
  * @brief  Инициализация интерфейса CAN
  *
  * @param prescaler: прешкаллер частоты обмена: CAN_1_SPEED_
  *
  */
void can_1_init(uint8_t prescaler)
{
	CAN_1.Init.Prescaler = prescaler;
	HAL_CAN_Init(&CAN_1);

	can_1_clear_buf(); //очистить приёмный буфер
	CAN_1.pRxMsg=&can_1_buf[can_1_buf_write_pointer]; //установить указатель на приём пакета в первый элемент буфера
	CAN_1.pTxMsg=&can_1_tx;							  //инициализировать переменную для формирования отправляемого пакета
	can_1_ffifo=CAN_FIFO0; can_1_fnumber=0;						  //установить указатель на инизиализацию первого фильтра FIFO0
	can_1_set_filter32(0,0, 0,0, CAN_FILTERMODE_IDMASK); //фильтр установить на приём всех пакетов
	can_1_ffifo=CAN_FIFO0; can_1_fnumber=0;						 //вернуть указаетль на фильтр 0, т.к. при вызове функции внешним модулем данный фильтр должен быть переинициализирован

}

/**
  * @brief  Установка 32-битного фильтра на приём (устанавливается сразу 2 фильтра или маску)
  *
  * @param  id: id для сравнения в фильтре 1
  * 		ide_rtr1: для фильтра1: бит0: удалённые запросы; бит1:пакеты с ID=29бит (0-не принимать; 1-принимать)
  *			id_mask: id для сравнения в фильтре 2 (режим CAN_FILTERMODE_IDLIST) или маска для режима CAN_FILTERMODE_IDMASK
  *			ide_rtr2: для фильтра2: бит0: удалённые запросы; бит1:пакеты с ID=29бит
  *			type: режим: 2 фильтра или маска. CAN_FILTERMODE_IDLIST - 2 фильтра;  CAN_FILTERMODE_IDMASK - маска
  * @retval 0:фильтр не установлен 1:фильтр установлен
  */
uint8_t can_1_set_filter32(uint32_t id, uint8_t ide_rtr1, uint32_t id_mask, uint8_t ide_rtr2, uint8_t type)
{CAN_FilterConfTypeDef  CAN_FilterInitStructure;

	if (can_1_fnumber>CAN_1_END_FILTER) return(0); //если использованы все свободные фильтры

	CAN_FilterInitStructure.FilterNumber = can_1_fnumber; //Номер фильтра

	if (type==CAN_FILTERMODE_IDMASK)
		CAN_FilterInitStructure.FilterMode = CAN_FILTERMODE_IDMASK;//CAN_FilterInitStructure.FilterMode = CAN_FILTERMODE_IDMASK;
	else
		CAN_FilterInitStructure.FilterMode = CAN_FILTERMODE_IDLIST;

	CAN_FilterInitStructure.FilterScale = CAN_FILTERSCALE_32BIT;
	CAN_FilterInitStructure.FilterIdHigh = ((id&0x07FF)<<5)|((id&0x3E000)>>13); //запись фильтра для битов 0-10 STID и для битов 13-17 EXID
	CAN_FilterInitStructure.FilterIdLow = ((id&0x1FFF)<<3)|((ide_rtr1&0x03)<<1); //запись фильтра IDE, RTR и битов 0-12 EXID
	CAN_FilterInitStructure.FilterMaskIdHigh = ((id_mask&0x07FF)<<5)|((id_mask&0x3E000)>>13); //запись фильтра для битов 0-10 STID и для битов 13-17 EXID
	CAN_FilterInitStructure.FilterMaskIdLow = ((id_mask&0x1FFF)<<3)|((ide_rtr2&0x03)<<1); //запись фильтра IDE, RTR и битов 0-12 EXID;
	if (can_1_ffifo==CAN_FIFO0)	CAN_FilterInitStructure.FilterFIFOAssignment = CAN_FILTER_FIFO0;//фильтр для FIFO0
	else					CAN_FilterInitStructure.FilterFIFOAssignment = CAN_FILTER_FIFO1;//фильтр для FIFO1
	CAN_FilterInitStructure.FilterActivation = ENABLE;
	#ifdef CAN2
	CAN_FilterInitStructure.BankNumber=CAN_1_END_FILTER+1;
	#else
	CAN_FilterInitStructure.BankNumber=CAN_ALL_FILTERS;
	#endif

	HAL_CAN_ConfigFilter(&CAN_1, &CAN_FilterInitStructure);

	if (can_1_ffifo==CAN_FIFO0) //если инициализация фильтра на FIFO0
		{
		HAL_CAN_Receive_IT(&CAN_1, CAN_FIFO0);	//включить приём пакетов на FIFO0
		CAN_1.State = HAL_CAN_STATE_READY;
		can_1_ffifo=CAN_FIFO1;					//следующий фильтр будет установлен на FIFO1
		}
	else
		{
		HAL_CAN_Receive_IT(&CAN_1, CAN_FIFO1);	//включить приём пакетов на FIFO1
		CAN_1.State = HAL_CAN_STATE_READY;
		can_1_ffifo=CAN_FIFO0;					//следующий фильтр будет установлен на FIFO0
		}
	can_1_fnumber++;

	return(1);

}

/**
  * @brief  Очищает буфер приёма
  */
void can_1_clear_buf(void)
{
	memset((uint8_t*)can_1_buf, 0, sizeof(can_1_buf));
	can_1_buf_read_pointer=0;
	can_1_buf_write_pointer=0;
	can_1_buf_overflow=0;
}

/**
  * @brief  Возвращает адрес буфера в памяти
  *
  * @retval адрес
  */
void* can_1_get_buf_pointer(void)
{
	return(can_1_buf);
}

/**
  * @brief  Возвращает  номер элемента буфера куда в будет записан следующий байт
  *
  * @retval смещение от 0 до CAN_1_BUF_SIZE-1
  */
uint32_t can_1_get_buf_shift(void)
{
	return(can_1_buf_write_pointer);
}

/**
  * @brief  Возвращает количество переполнений буфера
  *
  * @retval количество переполнений
  */
uint32_t can_1_get_buf_overflow(void)
{
	return(can_1_buf_overflow);
}

/**
  * @brief  Копирует из буфера принятые, но не обработанные данные
  *
  * @param  *id: указатель куда записать идентификатор принятого пакета
  *			*length: указательна переменную куда записать длинну принятого пакета
  *			*data: указатель на буфер, куда надо скопировать данные (буфер должен быть 8 байт)
  * @retval 0: нет необработанных данных 1:данные скопированы
  */
uint8_t can_1_read_rx_data(uint32_t* id, uint8_t* length, uint8_t* data)
{uint32_t cpy_size=0, rx_point, buf_point=0;

	rx_point=can_1_get_buf_shift();
	if (rx_point==can_1_buf_read_pointer) return(0);

	if (can_1_buf[can_1_buf_read_pointer].IDE==CAN_ID_STD)
		*id=can_1_buf[can_1_buf_read_pointer].StdId;
	else
		*id=can_1_buf[can_1_buf_read_pointer].ExtId;

	*length=can_1_buf[can_1_buf_read_pointer].DLC;
	memset(data,0,8);
	memcpy(data, can_1_buf[can_1_buf_read_pointer].Data, can_1_buf[can_1_buf_read_pointer].DLC);

	can_1_buf_read_pointer++;
	if (can_1_buf_read_pointer>=CAN_1_BUF_SIZE_RX)  can_1_buf_read_pointer=0;

	return(1);
}

/**
  * @brief  Возвращает количество принятых, но не обработанных внешней функцией пакетов
  *
  * @retval количество пакетов
  */
uint32_t can_1_get_rx_size(void)
{uint32_t sz=0, rx_point, pt;

	rx_point=can_1_get_buf_shift();
	pt=can_1_buf_read_pointer;

	if (rx_point<can_1_buf_read_pointer) //если указатель приёма переходил через конец буфера
		{
		sz=CAN_1_BUF_SIZE_RX-can_1_buf_read_pointer;
		pt=0;
		}
	sz+=(rx_point-pt);

	return(sz);
}

/**
  * @brief  Копирует в буфер передачи данные и отправляет в CAN
  *
  * @param  id: идентификатор
  *			length: размер данных которые надо отправить (не больше 8)
  *			data: указатель на буфер с данными, которые надо отправить
  * @retval 0: пакет не отправлен, 1: пакет отправлен
  */
uint8_t can_1_write_tx_data(uint32_t id, uint8_t length, uint8_t* data)
{
	if (length>8) length=8; //если попытка передать больше чем 8 байт, то ограничить пакет 8-ю байтами

	if (id>0x7FF)	CAN_1.pTxMsg->IDE = CAN_ID_EXT; //если размер дареса больше 11 бит, то расширенный ID = 29бит
	else			CAN_1.pTxMsg->IDE = CAN_ID_STD; //иначе ID 11бит

	CAN_1.pTxMsg->RTR=0;	//пакеты удалённых запросов не поддерживаются (не рекуомендуется поддержка удалённых запросов, т.к. они аппаратно по разному обрабатываются и поддерживаются на различных устйроствах)

	CAN_1.pTxMsg->DLC = length;
	CAN_1.pTxMsg->StdId = id;
	CAN_1.pTxMsg->ExtId = id;
	memcpy(CAN_1.pTxMsg->Data, data, length);

	if (HAL_CAN_Transmit(&CAN_1, CAN1_TX_TIMEOUT)==HAL_OK) return(1); //если пакет отправлен
	else return(0);
}

/**
  * @brief Перемещение указателя в приёмноом буфере на следующее свободное место
  *
  */
void can_1_buf_point_move(void)
{
	can_1_buf_write_pointer++;
	if (can_1_buf_write_pointer>=CAN_1_BUF_SIZE_RX)
		{
		can_1_buf_write_pointer=0;
		can_1_buf_overflow++;
		}
	CAN_1.pRxMsg=&can_1_buf[can_1_buf_write_pointer];

}

/**
  * @brief  Обработчик прерывания CAN1 очереди FIFO0
  *
  *  @param  *hcan: указатель на структуру с описанием CAN
  *
  */
void can_1_rx0_irq(CAN_HandleTypeDef *hcan)
{
	can_1_buf_point_move();
	//CAN_1.State = HAL_CAN_STATE_READY;
	//HAL_CAN_Receive_IT(&CAN_1, CAN_FIFO0);
	__HAL_CAN_ENABLE_IT(hcan, CAN_IT_FMP0);
}

/**
  * @brief  Обработчик прерывания CAN1 очереди FIFO1
  *
  *  @param  *hcan: указатель на структуру с описанием CAN
  *
  */
void can_1_rx1_irq(CAN_HandleTypeDef *hcan)
{
	can_1_buf_point_move();
	//CAN_1.State = HAL_CAN_STATE_READY;
	//HAL_CAN_Receive_IT(&CAN_1, CAN_FIFO1);
	__HAL_CAN_ENABLE_IT(hcan, CAN_IT_FMP1);
}
