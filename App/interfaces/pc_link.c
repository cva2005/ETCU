/*
 * pc_link.c
 *
 *  Created on: 11 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "pc_link.h"
#include "wifi_hf.h"
#include "timers.h"
#include "crc.h"
#include <string.h>
//------------------------переменные для обммена с ПК------------------------------
static pc_link_pack_t pc_link_pack_rx; //буфер хрнения принимаемого пакета
static pc_link_pack_t pc_link_pack_tx; //буфер для формирования передаваемого пакета
static uint8_t pc_link_buf[PC_LINK_BUF_SIZE-sizeof(header_t)]; //буфер для хранения полностью правильно принятого пакета
static uint16_t pc_link_rx_size=0; //размер принятых даных
static uint16_t pc_link_point_rx=0; //указатель на текущий принимаемый байт
static stime_t pc_link_tx_time;	//время когда разрешена отправка следующего пакета (минимальный интервал между отправками)
static stime_t pc_link_rx_time;	//время до которого должен прийти пакет данных, чтобы связь не считалась оборванной (максимальный интервал между приёмами пакетов)
static uint8_t pc_link_tx_busy=0; 	//флаг "передатчик занят"
static stime_t pc_link_rx_pack_time; //время до которого должен быть завершён приём пакета (максимально допустимое время приёма пакета)
static uint8_t pc_link_tx_mode=PC_LINK_LOSS; 	//режим отправки данных (тип отправляемых пакетов)
static uint8_t pc_link_rx_mode=PC_LINK_LOSS; 	//режим приёма данных (тип принимаемых данных)
static uint16_t pc_link_rx_session=0; //номер соединения (устройства) от которго принят пакет
static uint16_t pc_link_tx_session=0;//номер соединения (устройства) для передачи
static uint16_t pc_link_rev=PC_LINK_REV; //версия протокола и версия данных, для исключения несовместимости

/**
  * @brief  Инициализация модуля обмена с ПК
  */
void pc_link_init(void)
{
	pc_link_reset();
}

/**
  * @brief	Сброс модуля обмена с ПК (переинициализация)
  */
void pc_link_reset(void)
{
	pc_link_tx_time=timers_get_finish_time(500);
	pc_link_pack_tx.fld.header.size=0;
	pc_link_rx_size=0; //размер принятых даных
	pc_link_point_rx=0; //указатель на текущий принимаемый байт
	pc_link_tx_time = timers_get_finish_time(0);	//интервал между отправками пакета
	pc_link_rx_time=timers_get_finish_time(PC_LINK_MAX_RX_TIMEOUT);	//максимально допустимый интервал между приёмами пакетов
	pc_link_tx_mode=PC_LINK_LOSS; 	//режим отправки данных (тип отправляемых пакетов)
	pc_link_rx_mode=PC_LINK_LOSS; 	//режим приёма данных (тип принимаемых данных)
	wifi_hf_clear_buf();
}

/**
  * @brief  Возвращает состояние режима обмена для последних принятых данных
  *
  * @retval режим обмена: обрыв, обмен данными, обмен конфигурацией и т.д.
  */
uint8_t pc_link_get_rx_mode(void)
{
	pc_link_step();
	if (pc_link_rx_time.del==0) return(PC_LINK_LOSS);//if ((pc_rx_time==0)&&(pc_tx_version==V_CE3)) return(PC_LINK_LOSS);
	else return(pc_link_rx_mode);
}

/**
  * @brief  Возвращает номер сессии для последних принятых данных
  *
  * @retval уникальный номер сессии
  */
uint16_t pc_link_get_pc_session(void)
{
	pc_link_step();
	return(pc_link_rx_session);
}

/**
  * @brief  Определяет состояние готовности модуля отправки данных на ПК
  *
  * @retval сосотяние: 1-передатчик готов к отправке; 0-передатчик занят отправкой предыдущего пакета
  */
uint8_t pc_link_tx_ready(void)
{
	if ((wifi_hf_get_tx_status()==0)&&(pc_link_tx_busy==0)) return(1); //if ((wifiReadyTx())&&(pc_tx_busy==0)) return(1);
	else return(0);
}

/**
  * @brief  Записать данные для отправки в очередь ожидания
  *
  * @param  mode: режим отправки данных (тип пакета)
  * 		session: сессия в рамках которой надо топравить данные
  * 		p*: указатель на буфер с данными для отправки
  * 		size: размер отправляемых данных (размер поля данных)
  *
  * @retval количество отправленных (поставленных в очередь на отправку) данных
  */
uint16_t pc_link_write_data(uint8_t mode, uint16_t session, uint8_t *p, uint16_t size)
{uint16_t tx_size=0;

	if ((wifi_hf_get_tx_status()==0)&&(pc_link_tx_busy==0))
		{
		if (size<=sizeof(pc_link_pack_tx.fld.data))
			{
			pc_link_tx_mode=mode;
			pc_link_tx_session=session;
			memcpy(pc_link_pack_tx.fld.data, p, size); //скопировать необходимые для отправки данные
			pc_link_pack_tx.fld.header.size=size;//+sizeof(pc_pack_rx.fld.number)+sizeof(pc_pack_rx.fld.type); //заполнить размер данных
			tx_size=size;
			pc_link_tx_busy=1;
			}
		else tx_size=0;
		}
	pc_link_step();
	return (tx_size);
}

/**
  * @brief  Устанавливает занчение поля "версия протокола", версия протокола зависит от типа данных, поэтому устанавливается внешней функцией
  *
  * @param  rev: версия протокола
  */
void pc_link_set_rev(uint16_t rev)
{
	pc_link_rev=rev;
}

/**
  * @brief  Прочитать последний принятый пакет с верным CRC
  *
  * @param  *buf: указатель на буфер для копирования данных
  * 		size: размер предоставленного буфера
  *
  * @retval количество скопированных (принятых) данных
  */
uint16_t pc_link_read_data(void *buf, uint16_t size)
{uint16_t rx_size_tmp;
	if ((size>=pc_link_rx_size)&&(pc_link_rx_size>0))
		{
		pc_link_step();
		memcpy(buf, pc_link_buf, pc_link_rx_size); //скопировать данные в предоставленный буфер
		rx_size_tmp=pc_link_rx_size;
		pc_link_rx_size=0;
		pc_link_rx_mode=PC_LINK_LOSS;
		return(rx_size_tmp);
		}
	else
		return 0;
}

/**
  * @brief  Обработчик принятых пакетов: определяет все ли данные приняты, данные корректны и корректна ли CRC
  */
static void pc_link_rx_data(void)
{uint32_t rx_size;
udata32_t crc;

	if (pc_link_point_rx<sizeof(pc_link_pack_rx.fld.header)) //Если заголовок пакета ещё не прочитан
		{
		rx_size=wifi_hf_get_rx_size(); //проверить сколько байт заголовка принято
		if (rx_size>=(sizeof(pc_link_pack_rx.fld.header)-pc_link_point_rx)) //если заголовок уже принят, то прочитать его
			{
			pc_link_point_rx+=wifi_hf_read_rx_data(&pc_link_pack_rx.fld.header.start+pc_link_point_rx, (sizeof(pc_link_pack_rx.fld.header)-pc_link_point_rx));
			}
		}

	if (pc_link_point_rx>=sizeof(pc_link_pack_rx.fld.header)) //если заголовок пакета прочитан
		{
		rx_size=wifi_hf_get_rx_size(); //проверить сколько байт данных принято
		if (rx_size>=(pc_link_pack_rx.fld.header.size+4)) //если число принятых байт данных равно значению в поле длинны и принято CRC (+4)
			{
			pc_link_point_rx+=wifi_hf_read_rx_data(pc_link_pack_rx.fld.data, (pc_link_pack_rx.fld.header.size+4));
			}
		}

	if (pc_link_point_rx>=(sizeof(pc_link_pack_rx.fld.header)+pc_link_pack_rx.fld.header.size+4)) //если принят весь пакет
		{
		pc_link_point_rx=0;  //указатель установить на приём следующих пакетов
		crc.dword=crc32_ether(&pc_link_pack_rx.byte[0], (pc_link_pack_rx.fld.header.size+sizeof(pc_link_pack_rx.fld.header)), 1);
		if (memcmp(crc.byte, &pc_link_pack_rx.byte[pc_link_pack_rx.fld.header.size+sizeof(pc_link_pack_rx.fld.header)], 4)!=0)
			{
			pc_link_rx_size=0;
			pc_link_rx_mode=PC_LINK_LOSS;
			return;
			}
		pc_link_rx_time=timers_get_finish_time(PC_LINK_MAX_RX_TIMEOUT); //ожидать прихода следующего пакета в течении заданного времени
		if (pc_link_pack_rx.fld.header.version==pc_link_rev)
			{
			pc_link_rx_mode=pc_link_pack_rx.fld.header.type;
			pc_link_rx_session=pc_link_pack_rx.fld.header.session;
			pc_link_rx_size=pc_link_pack_rx.fld.header.size;//-sizeof(pc_pack_rx.fld.number)-sizeof(pc_pack_rx.fld.type);
			if (pc_link_pack_rx.fld.header.size<sizeof(pc_link_buf))
			memcpy(pc_link_buf, pc_link_pack_rx.fld.data, pc_link_pack_rx.fld.header.size);//(pc_pack_rx.fld.size-sizeof(pc_pack_rx.fld.number)-sizeof(pc_pack_rx.fld.type)));
			}
		else
			{
			pc_link_rx_size=0;
			pc_link_rx_mode=PC_LINK_LOSS;
			}
		}
}

/**
  * @brief  Формирует заголовок пакета и отправляет данные из буфера (записанные pc_link_write_data)
  */
static void pc_link_tx_data(void)
{uint16_t pack_size=0;
 udata32_t crc;

 	if (pc_link_tx_busy) //если буфер на отправку не пуст (есть пакет на отправку)
 		{
 		if (wifi_hf_get_tx_status()==0) //если передатчик свободен
			{
 			pc_link_pack_tx.fld.header.start=0x5A;
 			pc_link_pack_tx.fld.header.number++;
 			pc_link_pack_tx.fld.header.type=pc_link_tx_mode;
 			pc_link_pack_tx.fld.header.session=pc_link_tx_session;
 			pc_link_pack_tx.fld.header.version=pc_link_rev;

 			pack_size=pc_link_pack_tx.fld.header.size+sizeof(pc_link_pack_tx.fld.header); // + старт-байт + размер пакета
 			crc.dword=crc32_ether(&pc_link_pack_tx.byte[0], pack_size, 1);
 			memcpy(&pc_link_pack_tx.byte[pack_size], crc.byte, 4);

 			if (wifi_hf_write_tx_data(pc_link_pack_tx.byte, (pack_size+4))) //добавляем к разему 4 байта CRC32
 				{
 				pc_link_tx_time=timers_get_finish_time(PC_LINK_MIN_TX_TIME);
 				pc_link_tx_busy=0;
 				}
			}
 		}
}

/**
  * @brief  //шаг обмена pc_link: необходимо добавить в основной цикл
  */
void pc_link_step(void)
{uint16_t cnt;

	if (timers_get_time_left(pc_link_tx_time)==0) //истекло минимальное время между отправками пакетов
		{
		pc_link_tx_data();						  //отправить пакет (если есть пакет для отправки)
		}

	pc_link_rx_data();							 //обработать принятый пакет
	if (pc_link_point_rx==0) //если нет приёма пакета
		{
		pc_link_rx_pack_time=timers_get_finish_time(PC_LINK_MAX_RX_PACK_TIME); //установить новое время, когда должен закончится приём пакета
		}
	else					//если идёт приём пакета
		{
		if (timers_get_time_left(pc_link_rx_pack_time)==0)	//если время на приём пакета истекло, но при этом пакет принят не удалось, значит в буфере "мусор"
			wifi_hf_clear_buf();							//очистить буфер
		}

}

