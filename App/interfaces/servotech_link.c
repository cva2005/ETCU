/*
 * servotech_link.c
 *
 *  Created on: 26 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "servotech_link.h"
#include "can_1.h"
#include "can_2.h"
#include "timers.h"

servotech_link_rx_t servotech_link_rx[SERVOTECH_LINK_MAX_DEV]; //указатели на функции обработчики пакетов от устйроств CanOpen
static uint8_t servotech_link_channel=0; //рабочий канал CAN на котором инициализирована надстройка servotech_link

/**
  * @brief  Инициализация интерфейса
  *
  * @param  chanal: канал CAN на котором необходимо инициализировать надсткройку servotech_link: 1 или 2
  */
void servotech_link_init(uint8_t chanal)
{uint8_t cnt;

	for (cnt=0; cnt<SERVOTECH_LINK_MAX_DEV; cnt++) servotech_link_rx[cnt]=NULL;

	#ifdef CAN_1
	if (chanal==1)
		{
		servotech_link_channel=1;
		can_1_set_filter32(SERVOTECH_LINK_ADR,0, 0x007,0x03, CAN_FILTERMODE_IDMASK); //пакеты только с адресом контроллера
		}
	#endif
	#ifdef CAN_2
	if (chanal==2)
		{
		servotech_link_channel=2;
		can_2_set_filter32(SERVOTECH_LINK_ADR,0, 0x007,0x03, CAN_FILTERMODE_IDMASK); //пакеты только с адресом контроллера
		}
	#endif
}

/**
  * @brief  Изменяет канал CAN через который работает интерфейс
  *
  * @param  channel: задаваемый канал: 1 или 2, 0 - отключает модуль servotech_link
  *
  * @retval номер текущего канала
  */
uint8_t servotech_link_set_channel(uint8_t channel)
{
	if ((channel==1)||(channel==2))
		servotech_link_channel=channel;
	else
		servotech_link_channel=0;

	return(servotech_link_channel);
}

/**
  * @brief  шаг обмена CanOpen: необходимо добавить в основной цикл
  *
  */
void servotech_link_step(void) // ToDo: ИМ контрура оборотов ("медленный сервопривод")
{uint8_t cnt, read_st, can_msg[8], can_length;
uint32_t can_id;

	do
		{
		read_st=0;
		#ifdef CAN_1
		if ((servotech_link_channel==1) && can_1_get_rx_size()) //если servotech_link использует первый канал и там есть пакеты
 			{
			read_st=can_1_read_rx_data(&can_id, &can_length, can_msg);
 			}
		else
		#endif
 			{
		#ifdef CAN_2
			if ((servotech_link_channel==2) && can_2_get_rx_size())  //если servotech_link использует второй канал и там есть пакеты
				read_st=can_2_read_rx_data(&can_id, &can_length, can_msg);
		#endif
 			}

		if (read_st)	//если были прочитаны пакеты
 			{
			for (cnt=0; cnt<SERVOTECH_LINK_MAX_DEV; cnt++)
				{
				if (servotech_link_rx[cnt]!=NULL)
					servotech_link_rx[cnt]((char*)can_msg, can_length, can_id);
				}
 			}
		}
	while (read_st);
}

/**
  * @brief  Отправить технологический пакет
  *
  * @param  adr: адрес устйроства servotech_link (адрес привода от 1 до 6)
  * 		cmd: команда технологического протокола servotech_link_tech_t
  * 		parameter: адрес параметра
  * 		data: значение параметра
  *
  * @retval 0:не отправлен 1:отправлен
  */
// ToDo: ИМ контрура оборотов ("медленный сервопривод")
uint8_t servotech_link_tx_tech(uint8_t adr, servotech_link_tech_t cmd, uint16_t parameter, uint32_t data)
{uint8_t can_msg[8], can_length;
servotech_link_id_t can_id;

	can_id.dword=0;
	can_id.f.sink_addr = adr;
	can_id.f.source_addr = SERVOTECH_LINK_ADR;
	can_id.f.cmd = CMD_TX_USB;
	can_msg[0] = cmd;
	can_msg[1] = (uint8_t) parameter;
	can_msg[2] = (uint8_t) (parameter>>8);
	can_msg[3] = (uint8_t) (data);
	can_msg[4] = (uint8_t) (data>>8);
	can_msg[5] = (uint8_t) (data>>16);
	can_msg[6] = (uint8_t) (data>>24);


	if (cmd==TECH_RQ) can_length=3;
	else can_length=7;

	#ifdef CAN_1
	if (servotech_link_channel==1)
		{if (can_1_write_tx_data(can_id.dword, can_length, can_msg)) return(1);}
	#endif

	#ifdef CAN_2
	if (servotech_link_channel==2)
		{if (can_2_write_tx_data(can_id.dword, can_length, can_msg)) return(1);}
	#endif

	return(0);

}

/**
  * @brief  Отправить пакет с командой
  *
  * @param  pdo_n: номер PDO: 1, 2, 3, 4
  * 		node_id: адрес устйроства CanOpen (от 1 до 32)
  * 		*data: указатель на буфер с данными (от 0 до 8 байт)
  * 		ln: размер данных
  * @retval 0:не отправлен 1:отправлен
  */
uint8_t servotech_link_tx_cmd(servotech_link_cmd_t cmd, uint8_t adr, int8_t *data, uint8_t ln)
{uint8_t can_msg[8], can_length, cnt;
servotech_link_id_t can_id;
//формирование пакета
	if (ln>8) ln=8;

	can_id.dword=0;
	can_id.f.sink_addr = adr;
	can_id.f.source_addr = SERVOTECH_LINK_ADR;
	can_id.f.cmd = cmd;

	can_length = ln;

	for (cnt=0; cnt<ln; cnt++) can_msg[cnt] = data[cnt];

	#ifdef CAN_1
	if (servotech_link_channel==1)
		{if (can_1_write_tx_data(can_id.dword, can_length, can_msg)) return(1);}
	#endif

	#ifdef CAN_2
	if (servotech_link_channel==2)
		{if (can_2_write_tx_data(can_id.dword, can_length, can_msg)) return(1);}
	#endif

	return(0);
}

/**
  * @brief  Выделяет из CAN адреса: адрес источника
  *
  * @param  adr: адрес CAN
  *
  * @retval Адрес источника
  */
uint8_t servotech_link_get_src_adr(uint32_t adr)
{servotech_link_id_t can_id;

	can_id.dword=adr;
	return(can_id.f.source_addr);
}

/**
  * @brief  Выделяет из CAN адреса: адрес назначения
  *
  * @param  adr: адрес CAN
  *
  * @retval Адрес назначения
  */
uint8_t servotech_link_get_dst_adr(uint32_t adr)
{servotech_link_id_t can_id;

	can_id.dword=adr;
	return(can_id.f.sink_addr);
}

/**
  * @brief  Выделяет из CAN адреса: команду
  *
  * @param  adr: адрес CAN
  *
  * @retval Команда
  */
uint8_t servotech_link_get_cmd(uint32_t adr)
{servotech_link_id_t can_id;

	can_id.dword=adr;
	return(can_id.f.cmd);
}

/**
  * @brief  Выделяет из данных значение параметра для технологичесткого протокола
  *
  * @param  *data: указатель на пакет
  *
  * @retval Параметр
  */
uint16_t servotech_link_get_pr(uint8_t *data)
{uint16_t pr=0;
	pr=data[1]|((uint16_t)data[2]<<8);
	return(pr);
}

/**
  * @brief  Выделяет команду технологического протокола
  *
  * @param  *data: указатель на пакет
  *
  * @retval Команда технологического протокола
  */
uint8_t servotech_link_get_tech_cmd(uint8_t *data)
{
	return(data[0]);
}
