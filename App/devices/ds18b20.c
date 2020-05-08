/*
 * ds18b20.c
 *
 *  Created on: 21 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#ifdef LOCAL_TEMP
#include "ds18b20.h"
#include "ds2482.h"
#include "timers.h"
#include "crc.h"
//#include <string.h>

static ds18b20_step_st_t ds18b20_step_st=DS18B20_SELECT_CHAN;	//состояние автомата чтения датчика
static uint8_t	ds18b20_init_chan=0; //инициализированные каналы
static uint8_t ds18b20_chan=0;		//текущий номер канала
static uint8_t ds18b20_fault=0;	//счётчик ошибок передачи команды
static uint8_t ds18b20_wait=0;		//флаг: необходимо ожидание выполнения команды
static uint8_t ds18b20_buf[9]={0,0,0,0,0,0,0,0,0};	//буфер для чтения памяти DS18B20
static uint8_t ds18b20_byte_cnt=0;	//указатель на свободный элемент буфера чтения памяти DS18B20
static stime_t ds18b20_step_timeout;	//таймаут между выполнением шагов имзерения температуры
static int32_t ds18b20_temperature[DS18B20_MAX_CHANNELS]; //значения температуры
static uint8_t ds18b20_error[DS18B20_MAX_CHANNELS];		//счётчики ошибок для каждого канала

/**
  * @brief  Инициализировать датчик DS18B20
  *
  * @param  chan: номер датчика от 1 до DS18B20_MAX_CHANNELS
  */
void ds18b20_init(uint8_t chan)
{
	if (chan>DS18B20_MAX_CHANNELS) chan=DS18B20_MAX_CHANNELS;
	if (chan>0) chan--;

	ds18b20_error[chan]=0;
	if (ds18b20_init_chan==0) ds18b20_chan=chan;
	ds18b20_init_chan|=(1<<chan);
	ds18b20_temperature[chan]=0;
}

/**
  * @brief  Получить данные температуры
  *
  * @param  chan: номер датчика от 1 до DS18B20_MAX_CHANNELS
  *
  * @retval температура*1000 (m°C)
  */
int32_t ds18b20_get_temp(uint8_t chan)
{
	if (chan>DS18B20_MAX_CHANNELS) chan=DS18B20_MAX_CHANNELS;
	if (chan>0) chan--;

	return(ds18b20_temperature[chan]);
}

/**
  * @brief  Получить данные об ошибках датчика
  *
  * @param  chan: номер датчика от 1 до DS18B20_MAX_CHANNELS
  *
  * @retval количество ошибок чтения
  */
int32_t ds18b20_get_error(uint8_t chan)
{
	if (chan>DS18B20_MAX_CHANNELS) chan=DS18B20_MAX_CHANNELS;
	if (chan>0) chan--;

	return(ds18b20_error[chan]);
}

/**
  * @brief  Шаг обработки данных датчиков температуры DS18B20: необходимо добавить в основной цик программы
  */
void ds18b20_step(void)
{
	if (ds18b20_init_chan==0) return; //если нет инициализированных каналов, то не обрабатывать

	if (ds18b20_fault>10)			//если большое число ошибок на текущем канале
		{
		ds18b20_next_chan(); //взять следующий канал
		ds18b20_step_st=DS18B20_SELECT_CHAN; //перейти к шагу выбора канала
		ds18b20_step_timeout=timers_get_finish_time(10);//установить задержку перед переходом на следующий канал
		ds18b20_fault=0;	//сбросить счётчик ошибок
		return;
		}

	if (timers_get_time_left(ds18b20_step_timeout)!=0) return; //если не истекла задержка, то выйти
	if (ds18b20_wait)		//если необходимо дождаться окончания операции
		{
		if (ds2482_get_busy())						//проверить закончена операция
			{
			ds18b20_step_timeout=timers_get_finish_time(1); //если не закончена проверить через 1 мс.
			return;
			}
		else ds18b20_wait=0;	//если операция закончина, то сбросить флаг ожидания готовности
		}

	switch (ds18b20_step_st)
		{
		case DS18B20_SELECT_CHAN:
			{
			if (ds2482_set_chanel(ds18b20_chan+1)==(ds18b20_chan+1)) //отправить команду выбора канала и проверить нужный канал выбран или нет?
				{
				ds18b20_step_st=DS18B20_SEND_CONVERT_ST;	 	//если выбран нужный канал перейти к следующему шагу
				ds18b20_step_timeout=timers_get_finish_time(0);
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_SEND_CONVERT_ST:
			{
			if (ds2482_rd_bit_cmd_1wire())			//отправить команду чтения 1 бита
				{
				ds18b20_step_st=DS18B20_READ_CONVERT_ST; //перейти к проверке результата чтения 1 бита
				ds18b20_step_timeout=timers_get_finish_time(1);	//через 1 мс.
				ds18b20_wait=1; //установить флаг необходимости ожидания завершения операции
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_READ_CONVERT_ST:
			{
			if (ds2482_rd_bit_result_1wire()==1)   //если измерение температуры закончено
				{
				ds18b20_step_st=DS18B20_SEND_PRESET_RD;	//перейти к отправке present импульса
				ds18b20_step_timeout=timers_get_finish_time(1); //чере 1 мс.
				}
			else								//если нет, то ожидать окончания
				{
				ds18b20_step_st=DS18B20_SEND_CONVERT_ST;
				ds18b20_cmd_retry();
				}
			break;
			}
		case DS18B20_SEND_PRESET_RD:
			{
			if (ds2482_send_present_1wire())
				{
				ds18b20_step_st=DS18B20_RESULT_PRESET_RD;
				ds18b20_step_timeout=timers_get_finish_time(1); //чере 1 мс.
				ds18b20_wait=1; //установить флаг необходимости ожидания завершения операции
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_RESULT_PRESET_RD:
			{
			if (ds2482_get_present_1wire())			//если есть ответ на present pulse
				{
				ds18b20_step_st=DS18B20_SEND_SKIP_ROM_RD;
				ds18b20_step_timeout=timers_get_finish_time(1); //чере 1 мс.
				}
			else
				{
				if (ds18b20_error[ds18b20_chan]<0xFF) ds18b20_error[ds18b20_chan]++;
				ds18b20_next_chan(); //взять следующий канал
				ds18b20_step_st=DS18B20_SELECT_CHAN; //перейти к чтению следующего канала
				}
			break;
			}
		case DS18B20_SEND_SKIP_ROM_RD:
			{
			if (ds2482_wr_1wire(DS18B20_SKIP_ROM))
				{
				ds18b20_step_st=DS18B20_SEND_MEM_RD;
				ds18b20_step_timeout=timers_get_finish_time(1); //чере 1 мс.
				ds18b20_wait=1; //установить флаг необходимости ожидания завершения операции
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_SEND_MEM_RD:
			{
			if (ds2482_wr_1wire(DS18B20_MEM_RD))
				{
				ds18b20_step_st=DS18B20_SEND_READ; //перейти к отправке команды чтения байта
				ds18b20_step_timeout=timers_get_finish_time(1); //чере 1 мс.
				ds18b20_byte_cnt=0; //указатель на начало буфера для чтения
				ds18b20_wait=1; //установить флаг необходимости ожидания завершения операции
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_SEND_READ:
			{
			if (ds2482_rd_byte_cmd_1wire())
				{
				ds18b20_step_st=DS18B20_GET_READ;
				ds18b20_step_timeout=timers_get_finish_time(1); //чере 1 мс.
				ds18b20_wait=1; //установить флаг необходимости ожидания завершения операции
				}
			break;
			}
		case DS18B20_GET_READ:
			{
			ds18b20_buf[ds18b20_byte_cnt]=ds2482_rd_byte_result_1wire();
			ds18b20_byte_cnt++;
			if (ds18b20_byte_cnt<sizeof(ds18b20_buf)) //если не все байты прочитаны
				ds18b20_step_st=DS18B20_SEND_READ; //перейти к отправке команды чтения байта
			else
				{
				if (crc8_1wire(ds18b20_buf, 8, 1)==ds18b20_buf[8]) //если CRC правильный
					{
					ds18b20_temperature[ds18b20_chan]=ds18b20_calc_temp(ds18b20_buf);
					ds18b20_error[ds18b20_chan]=0;
					}
				else
					{if (ds18b20_error[ds18b20_chan]<0xFF) ds18b20_error[ds18b20_chan]++;}

				ds18b20_step_st=DS18B20_SEND_PRESET_CNV;
				}
			break;
			}
		case DS18B20_SEND_PRESET_CNV:
			{
			if (ds2482_send_present_1wire())
				{
				ds18b20_step_st=DS18B20_RESULT_PRESET_CNV;
				ds18b20_step_timeout=timers_get_finish_time(1); //чере 1 мс.
				ds18b20_wait=1; //установить флаг необходимости ожидания завершения операции
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_RESULT_PRESET_CNV:
			{
			if (ds2482_get_present_1wire())			//если есть ответ на present pulse
				{
				ds18b20_step_st=DS18B20_SEND_SKIP_ROM_CNV;
				ds18b20_step_timeout=timers_get_finish_time(1); //чере 1 мс.
				}
			else
				{
				ds18b20_next_chan(); //взять следующий канал
				ds18b20_step_st=DS18B20_SELECT_CHAN; //перейти к чтению следующего канала
				}
			break;
			}
		case DS18B20_SEND_SKIP_ROM_CNV:
			{
			if (ds2482_wr_1wire(DS18B20_SKIP_ROM))
				{
				ds18b20_step_st=DS18B20_SEND_START_CNV;
				ds18b20_step_timeout=timers_get_finish_time(1); //чере 1 мс.
				ds18b20_wait=1; //установить флаг необходимости ожидания завершения операции
				}
			else ds18b20_cmd_retry();
			break;
			}
		case DS18B20_SEND_START_CNV:
			{
			if (ds2482_wr_1wire(DS18B20_START_CNV))
				{
				ds18b20_next_chan(); //взять следующий канал
				ds18b20_step_st=DS18B20_SELECT_CHAN;
				ds18b20_step_timeout=timers_get_finish_time(1); //чере 10 мс.
				ds18b20_wait=1; //установить флаг необходимости ожидания завершения операции
				}
			else ds18b20_cmd_retry();
			break;
			}
		}
}

/**
  * @brief  Персчитывает значение температуры из формата DS18B20 в м°C
  *
  * @param  *buf: указатель на буфер с сырыми данными, прочитанными из датчика
  *
  * @retval температура*1000 (m°C)
  */
static uint32_t ds18b20_calc_temp(uint8_t *buf)
{//uint16_t deg_val[16]={0,100,100,200,300,300,400,400,500,600,600,700,800,800,900,900};
uint16_t deg_val[16]={0,062,125,188,250,313,375,438,500,563,625,688,750,813,875,938};
int8_t dg, fl;
uint32_t temp;

	dg=(int8_t)((buf[1]<<4)|(buf[0]>>4));
	fl=buf[0]&0x0F;

	temp=(dg*1000)+deg_val[fl];

	return(temp);
}

/**
  * @brief  Обработчик неудачной попытки отправки команды датчику: увеличивает счётчик неудачных попыток и устанавливает таймаутт до следующей попытки
  */
static void ds18b20_cmd_retry(void)
{
	ds18b20_fault++;
	ds18b20_step_timeout=timers_get_finish_time(10); //если команда не прошла, то попробовать через 10 мс
}

/**
  * @brief  Взять следующий датчик (следующий канал): ищет следующий инициализированный датчик и устанавливает на него указатель ds18b20_chan
  */
static void ds18b20_next_chan(void)
{uint8_t cnt;
	ds18b20_fault=0;
	for(cnt=(ds18b20_chan+1); cnt<DS18B20_MAX_CHANNELS; cnt++)
		{
		if (ds18b20_init_chan&(1<<cnt))
			{
			ds18b20_chan=cnt;
			return;
			}
		}
	for(cnt=0; cnt<ds18b20_chan; cnt++)
		{
		if (ds18b20_init_chan&(1<<cnt))
			{
			ds18b20_chan=cnt;
			return;
			}
		}
	return;
}
#endif
