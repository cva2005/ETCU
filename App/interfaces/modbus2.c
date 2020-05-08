/*
 * modbus2.c
 *
 *  Created on: 29 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "modbus2.h"
#include "rs485_1.h"
#include "rs485_2.h"
#include "timers.h"
#include "crc.h"
#include <string.h>


modbus2_rx_t modbus2_rx[MODBUS2_MAX_DEV]; //указатели на функции обработчики пакетов от устйроств CanOpen
static uint8_t modbus2_channel=0;//рабочий канал RS-485 на котором инициализирована надстройка ModBus
static stime_t modbus2_tx_time; //время когда разрешен следующий приём
static stime_t modbus2_rx_time; //врея когда должен быть принят следующий байт
static uint8_t modbus2_busy;		//состояние интерфейса ModBus: 0-свободен, 1-ожидание пакетв ответа на запрос
static uint16_t modbus2_rx_size; //размер принятых данных
static uint8_t modbus2_buf[256];
static uint8_t modbus2_rq_tx[MODBUS2_MAX_DEV]; //таблица запросов отправки на случай если линия занята ожиданием ответа или передачей
static uint8_t modbus2_rq_point = 0; //указатель в таблице запросов на устйроство с максимальным приоритетом отправки: этот мехнизм нужен чтобы гарантировать всем устйроства на шине модбас равноценный доступ к процедуре отправки
static uint8_t modbus2_tx(uint8_t *data, uint16_t len); //Отправляет пакет по интерфейсу ModBus

/**
  * @brief  Инициализация интерфейса ModBus
  *
  * @param  chan: канал RS485 на котором необходимло инициализировать надсткройку ModBus: 1 или 2
  */
void modbus2_init(uint8_t chan)
{uint8_t cnt;
	for (cnt=0; cnt<MODBUS2_MAX_DEV; cnt++)
		{
		modbus2_rx[cnt]=NULL;
		modbus2_rq_tx[cnt]=0;
		}
	modbus2_busy=0;
	modbus2_tx_time=timers_get_finish_time(0);
	modbus2_rx_time=timers_get_finish_time(0);
	modbus2_rx_size=0;
	modbus2_rq_point=0;


#ifdef RS485_1_UART_NUMBER
	if (chan==1)
		{
		modbus2_channel=1;
		}
#endif
#ifdef RS485_2_UART_NUMBER
	if (chan==2)
		{
		modbus2_channel=2;
		}
#endif
}

/**
  * @brief  шаг обмена ModBus: необходимо добавить в основной цикл
  *
  */
void modbus2_step(void)
{uint32_t rx_size;
 uint16_t crc_rx, crc_calc;
 uint8_t cnt;

 	 if (timers_get_time_left(modbus2_tx_time)==0)
 		 modbus2_busy=0; //если не дождались ответа, то освободить  интерфейс

#ifdef RS485_1_UART_NUMBER
	if (modbus2_channel==1)	//если ModBus на канале 1
		rx_size=rs485_1_get_rx_size();	//прочитать кол-во принятых байт
#endif
#ifdef RS485_2_UART_NUMBER
	if (modbus2_channel==2)	//если ModBus на канале 2
		rx_size=rs485_2_get_rx_size();	//прочитать кол-во принятых байт
#endif
	if (rx_size!=modbus2_rx_size) //если количество принятых байт увеличилось
		{
		modbus2_rx_size=rx_size;	//обновить кол-во принятых байт
		modbus2_rx_time=timers_get_finish_time(MODBUS2_BYTE_RX_PAUSE);	//установить максимально допустимое время приёма следующего байта
		return;
		}

	if (rx_size!=0)	//если есть принятые байты
		{
		if (timers_get_time_left(modbus2_rx_time)==0)	//если истекло время ожидания следующего байта: приём закончен
			{
#ifdef RS485_1_UART_NUMBER
			if (modbus2_channel==1)	//если ModBus на канале 1
				rs485_1_read_rx_data(modbus2_buf, rx_size);	//прочитать все принятые байты из буфера
#endif
#ifdef RS485_2_UART_NUMBER
			if (modbus2_channel==2)	//если ModBus на канале 1
				rs485_2_read_rx_data(modbus2_buf, rx_size);	//прочитать все принятые байты из буфера
#endif
			crc_rx=((uint16_t)modbus2_buf[rx_size-2]<<8)|modbus2_buf[rx_size-1];		//прочитать CRC принятого пакета
			crc_calc=modbus_crc_rtu(modbus2_buf, rx_size-2);	//расчитать CRC принятого пакета
			if (crc_rx==crc_calc)							//если CRC cовпадают
				{
				modbus2_busy=0; //если пришёл ответ, то освободить  интерфейс
				for(cnt=0; cnt<MODBUS2_MAX_DEV; cnt++)		//найти обработчики принятых пакетов
					if (modbus2_rx[cnt]!=NULL)
						modbus2_rx[cnt](&modbus2_buf[3], modbus2_buf[2], modbus2_buf[0], modbus2_buf[1]); //вызвать обработчик принятых пакетов
				}
			}
		}
}


/**
  * @brief  Функция: Получение текущего значения одного или нескольких входных регистров
  *
  * @param  adr: адрес устйроства
  * 		reg: номер запрашиваемого регистра
  * 		number: количество запрашиваемых регисторв
  *
  * @retval результат запроса: 1 - успешно, 0 - запрос не отправлен
  */
uint8_t modbus2_rd_in_reg(uint8_t adr, uint16_t reg, uint16_t number)
{uint8_t pack[8], st;
udata16_t crc;

	//if (modbus_busy) return(0);
	if (modbus2_get_busy(adr)) return(0);

	pack[0]=adr;
	pack[1]=MODBUS2_READ_INPUTS_REGISTERS;
	pack[2]=reg>>8;
	pack[3]=reg;
	pack[4]=number>>8;
	pack[5]=number;

	crc.word=modbus_crc_rtu(pack, 6);
	pack[6]=crc.byte[1];
	pack[7]=crc.byte[0];

	if (modbus2_tx(pack, sizeof(pack)))
		{
		if (adr>0) adr--;
		modbus2_rq_tx[adr]=0;
		modbus2_rq_point++;
		if (modbus2_rq_point>=MODBUS2_MAX_DEV) modbus2_rq_point=0;
		return(1);
		}
	return(0);
}

/**
  * @brief  Функция: Получение текущего значения одного или нескольких регистров хранения
  *
  * @param  adr: адрес устйроства
  * 		reg: номер запрашиваемого регистра
  * 		number: количество запрашиваемых регисторв
  *
  * @retval результат запроса: 1 - успешно, 0 - запрос не отправлен
  */
uint8_t modbus2_rd_hold_reg(uint8_t adr, uint16_t reg, uint16_t number)
{uint8_t pack[8];
udata16_t crc;

	//if (modbus_busy) return(0);
	if (modbus2_get_busy(adr)) return(0);

	pack[0]=adr;
	pack[1]=MODBUS2_READ_HOLDING_REGISTERS;
	pack[2]=reg>>8;
	pack[3]=reg;
	pack[4]=number>>8;
	pack[5]=number;

	crc.word=modbus_crc_rtu(pack, 6);
	pack[6]=crc.byte[1];
	pack[7]=crc.byte[0];

	if (modbus2_tx(pack, sizeof(pack)))
		{
		if (adr>0) adr--;
		modbus2_rq_tx[adr]=0;
		modbus2_rq_point++;
		if (modbus2_rq_point>=MODBUS2_MAX_DEV) modbus2_rq_point=0;
		return(1);
		}
	return(0);
}

/**
  * @brief  Функция: Запись нового значения в регистр хранения
  *
  * @param  adr: адрес устйроства
  * 		reg: номер  регистра
  * 		val: записываемое значение
  *
  * @retval результат запроса: 1 - успешно, 0 - запрос не отправлен
  */
uint8_t modbus2_wr_1reg(uint8_t adr, uint16_t reg, uint16_t val)
{uint8_t pack[8];
udata16_t crc;

	//if (modbus_busy) return(0);
	if (modbus2_get_busy(adr)) return(0);

	pack[0]=adr;

	pack[1]=MODBUS2_FORCE_SINGLE_REGISTER;
	pack[2]=reg>>8;
	pack[3]=reg;
	pack[4]=val>>8;
	pack[5]=val;

	crc.word=modbus_crc_rtu(pack, 6);
	pack[6]=crc.byte[1];
	pack[7]=crc.byte[0];

	if (modbus2_tx(pack, sizeof(pack)))
		{
		if (adr>0) adr--;
		modbus2_rq_tx[adr]=0;
		modbus2_rq_point++;
		if (modbus2_rq_point>=MODBUS2_MAX_DEV) modbus2_rq_point=0;
		return(1);
		}
	return(0);
}


/**
  * @brief  Функция: Пользовательская функция
  *
  * @param  adr: адрес устйроства
  * 		func: номер функции
  * 		ln: длина данных
  * 		*data: указатель данных функции
  *
  * @retval результат запроса: 1 - успешно, 0 - запрос не отправлен
  */
uint8_t modbus2_user_function(uint8_t adr, uint8_t func, uint8_t ln, uint8_t *data)
{uint8_t pack[255];
udata16_t crc;

	//if (modbus_busy) return(0);
	if (modbus2_get_busy(adr)) return(0);
	if (ln>(sizeof(pack)-sizeof(adr)-sizeof(func)-sizeof(ln))) return(0);

	pack[0]=adr;
	pack[1]=func;
	pack[2]=ln;
	memcpy(&pack[3],data,ln);

	crc.word=modbus_crc_rtu(pack, ln+sizeof(adr)+sizeof(func)+sizeof(ln));
	pack[3+ln]=crc.byte[1];
	pack[4+ln]=crc.byte[0];

	if (modbus2_tx(pack, ln+sizeof(adr)+sizeof(func)+sizeof(ln)+2))
		{
		if (adr>0) adr--;
		modbus2_rq_tx[adr]=0;
		modbus2_rq_point++;
		if (modbus2_rq_point>=MODBUS2_MAX_DEV) modbus2_rq_point=0;
		return(1);
		}
	return(0);
}


/**
  * @brief  Возвращает готовность ModBus к передаче данных
  *
  * @param  adr: адрес устйроство которе запрашивает доступ к шине.
  * Если 0 - то проверяется готовность линии без учёта приоритетов
  * Если >0 - проверяется очереь запросов, и результат возвращается с учётом приорететов на отправку
  * в случае если линяя занята, то заданное устйроство ставится в очередь запросов на отправку
  *
  * @retval Состояние интерфейса: 1 - ModBus занят, 0 - ModBus свободен
  */
uint8_t modbus2_get_busy(uint8_t adr)
{uint8_t cnt;
	if (adr>0) //если проверка доступности линии с учётом приоритетов
		{
		adr--;
		if (modbus2_busy==0) //шина доступна
			{
			cnt=modbus2_rq_point;		//начинаем ппроверку проиоритетов запроса с устйроства, которое на данном шаге имеет максимальный приоритет
			while ((cnt!=adr)&&(modbus2_rq_tx[cnt]==0)) //проерить есть ли в очереди устйроства с большим приоритетом отправки
				{
				cnt++;
				if (cnt>=MODBUS2_MAX_DEV) cnt=0;
				}
			if (cnt==adr) //если нет устройств с большим приоритетом
				return(0); //вернуть что канал свободен
			else		//если есть устйроства с большим приоритетом отправки
				{
				modbus2_rq_tx[adr]=1; //установить флаг запроса на отправку
				return(1); //вернуть что шина занята (ожидает запроса отправки более приоритетного устйроства)
				}
			}
		else return(1);
		}
	else return(modbus2_busy);
}

/**
  * @brief  Отправляет пакет по интерфейсу ModBus
  *
  * @param  *data: указатель на пакет
  * 		len: размер данных
  *
  * @retval результат отправки: 1 - успешно, 0 - паакет не отправлен
  */
static uint8_t modbus2_tx(uint8_t *data, uint16_t len)
{
	if (modbus2_busy) return(0);

#ifdef RS485_1_UART_NUMBER
	if (modbus2_channel==1)
		if (rs485_1_write_tx_data(data, len))
			{
			modbus2_busy=1;
			modbus2_tx_time=timers_get_finish_time(MODBUS2_MAX_WAIT_TIME);
			return(1);
			}
#endif
#ifdef RS485_2_UART_NUMBER
	if (modbus2_channel==2)
		if (rs485_2_write_tx_data(data, len))
			{
			modbus2_busy=1;
			modbus2_tx_time=timers_get_finish_time(MODBUS2_MAX_WAIT_TIME);
			return(1);
			}

#endif
	return(0);
}

/**
  * @brief  Функция: Запись нового значения в регистры хранения
  *
  * @param  adr: адрес устйроства
  * 		reg: номер  1-го регистра
  * 		num: количество регистров
  * 		val: записываемое значение
  *
  * @retval результат запроса: 1 - успешно, 0 - запрос не отправлен
  */
uint8_t modbus2_wr_mreg(uint8_t adr, uint16_t reg, uint16_t num, uint8_t* data)
{
	uint8_t pack[MB2_TX_BUFF], i, j; udata16_t crc;

	if (modbus2_get_busy(adr)) return(0);
	pack[0] = adr;
	pack[1] = MODBUS2_FORCE_MULTIPLE_REGISTERS;
	pack[2] = (uint8_t)(reg >> 8);
	pack[3] = (uint8_t)reg;
	pack[4] = (uint8_t)(num >> 8);
	pack[5] = (uint8_t)num;
	num *= 2;
	pack[6] = (uint8_t)num;
	for (i = 0, j = 7; i < num; i++, j++)
		pack[j] = data[i];
	crc.word = modbus_crc_rtu(pack, j);
	pack[j++] = crc.byte[1];
	pack[j++] = crc.byte[0];
	if (modbus2_tx(pack, j)) {
		if (adr > 0) adr--;
		modbus2_rq_tx[adr] = 0;
		modbus2_rq_point++;
		if (modbus2_rq_point >= MODBUS2_MAX_DEV)
			modbus2_rq_point = 0;
		return 1;
	}
	return 0;
}


