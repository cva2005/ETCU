/*
 * CanOpen.c
 *
 *  Created on: 26 авг. 2014 г.
 *      Author: Перчиц А.Н.
 */

#include "canopen.h"
#include "can_1.h"
#include "can_2.h"
#include "timers.h"

CanOpen_rx_object_t CanOpen_rx_object[MAX_DEV_CANOPEN]; //указатели на функции обработчики пакетов от устйроств CanOpen
static uint8_t canopen_channel=0; //рабочий канал CAN на котором инициализирована надстройка CanOpen

/**
  * @brief  Инициализация интерфейса CanOpen
  *
  * @param  chan: канал CAN на котором необходимло инициализировать надсткройку CanOpen: 1 или 2
  */
void CanOpen_init(uint8_t chan)
{uint8_t cnt;

	for (cnt=0; cnt<MAX_DEV_CANOPEN; cnt++) CanOpen_rx_object[cnt]=NULL;

	#ifdef CAN_1
	if (chan==1)
		{
		canopen_channel=1;
		can_1_set_filter32(0x000,0, 0x080,0x00, CAN_FILTERMODE_IDLIST); //принимать только пакеты NMT и пакеты SYNC
		can_1_set_filter32(0x380,0, 0x480,0x03, CAN_FILTERMODE_IDMASK); //пакеты PDO1(0x180) PDO2(0x280) PDO3(0x380), проверять на совпадение поля: ID (0x480), IDE, RTR
		can_1_set_filter32(0x580,0, 0x780,0x03, CAN_FILTERMODE_IDMASK); //пакеты SDO (0x580), проверять на совпадение поля: ID (0x780), IDE, RTR
		can_1_set_filter32(0x480,0, 0x780,0x03, CAN_FILTERMODE_IDMASK); //пакеты PDO4(0x480), проверять на совпадение поля: ID (0x780), IDE, RTR
		//can_1_set_filter32(0x580,0, 0x500,0x03, CAN_FILTERMODE_IDMASK); //пакеты SDO (0x580) и START (0x700), проверять на совпадение поля: ID (0x780), IDE, RTR
		can_1_set_filter32(0x700,0, 0x780,0x03, CAN_FILTERMODE_IDMASK); //NODE START (0x700), проверять на совпадение поля: ID (0x780), IDE, RTR
		}
	#endif
	#ifdef CAN_2
	if (chan==2)
		{
		canopen_channel=2;
		can_2_set_filter32(0x000,0, 0x080,0x00, CAN_FILTERMODE_IDLIST); //принимать только пакеты NMT и пакеты SYNC
		can_2_set_filter32(0x380,0, 0x480,0x03, CAN_FILTERMODE_IDMASK); //пакеты PDO1(0x180) PDO2(0x280) PDO3(0x380), проверять на совпадение поля: ID (0x480), IDE, RTR
		can_2_set_filter32(0x580,0, 0x780,0x03, CAN_FILTERMODE_IDMASK); //пакеты SDO (0x580), проверять на совпадение поля: ID (0x780), IDE, RTR
		can_2_set_filter32(0x480,0, 0x780,0x03, CAN_FILTERMODE_IDMASK); //пакеты PDO4(0x480), проверять на совпадение поля: ID (0x780), IDE, RTR
		can_2_set_filter32(0x700,0, 0x780,0x03, CAN_FILTERMODE_IDMASK); //NODE START (0x700), проверять на совпадение поля: ID (0x780), IDE, RTR
		}
	#endif
	//CanOpen_tx_nmt(NODE_STOP, 0);
}

/**
  * @brief  Изменяет канал CAN через который работает CanOpen
  *
  * @param  channel: задаваемый канал: 1 или 2, 0 - отключает модуль CanOpen
  *
  * @retval номер текущего канала
  */
uint8_t CanOpen_set_channel(uint8_t channel)
{
	if ((channel==1)||(channel==2))
		canopen_channel=channel;
	else
		canopen_channel=0;

	return(canopen_channel);
}

/**
  * @brief  шаг обмена CanOpen: необходимо добавить в основной цикл
  *
  */
void CanOpen_step(void)
{uint8_t cnt, read_st, can_msg[8], can_length;
uint32_t can_id;

	do
		{
		read_st=0;
		#ifdef CAN_1
		if ((canopen_channel==1) && can_1_get_rx_size()) //если CanOpen использует первый канал и там есть пакеты
 			{
			read_st=can_1_read_rx_data(&can_id, &can_length, can_msg);
 			}
		else
		#endif
 			{
		#ifdef CAN_2
			if ((canopen_channel==2) && can_2_get_rx_size())  //если CanOpen использует второй канал и там есть пакеты
				read_st=can_2_read_rx_data(&can_id, &can_length, can_msg);
		#endif
 			}

		if (read_st)	//если были прочитаны пакеты
 			{
			for (cnt=0; cnt<MAX_DEV_CANOPEN; cnt++)
				{
				if (CanOpen_rx_object[cnt]!=NULL)
					CanOpen_rx_object[cnt]((char*)can_msg, can_length, can_id);
				}
 			}
		}
	while (read_st);
}

/**
  * @brief  Отправить пакет NMT (протокол CanOpen)
  *
  * @param  type: тип пакета (CAN OPEN NMT COMMANDS):  NODE_START, NODE_STOP, NODE_PREOPERATIONAL, NODE_RESET
  * 		node_id: адрес устйроства CanOpen (от 1 до 32)
  * @retval 0:не отправлен 1:отправлен
  */
uint8_t CanOpen_tx_nmt(uint8_t type, uint8_t node_id)
{uint8_t can_msg[8], can_length;
uint32_t can_id;
//формирование пакета
	if ((type==NODE_START)||(type==NODE_STOP)||(type==NODE_PREOPERATIONAL)||(type==NODE_RESET)||(type==NODE_RESET_COMMUNICATION))
		{
		can_id = 0;
		can_length = 2;
		can_msg[0] = type;
		can_msg[1] = node_id;
		}
	else return(0);

	#ifdef CAN_1
	if (canopen_channel==1)
		{if (can_1_write_tx_data(can_id, can_length, can_msg)) return(1);}
	#endif

	#ifdef CAN_2
	if (canopen_channel==2)
		{if (can_2_write_tx_data(can_id, can_length, can_msg)) return(1);}
	#endif

	return(0);
}

/**
  * @brief  Отправить пакет SDO (протокол CanOpen)
  *
  * @param  node_id: адрес устйроства CanOpen (от 1 до 32)
  * 		index: индекс объекта
  * 		subindex: подиндекс объекта
  * 		data: данные SDO
  * 		cmd: команда (CAN OPEN SDO COMMANDS): WR, WR_4BYTE, WR_2BYTE, WR_1BYTE, RD, RD_4BYTE, RD_2BYTE, RD_1BYTE
  * @retval 0:не отправлен 1:отправлен
  */
uint8_t CanOpen_tx_sdo(uint8_t node_id, uint16_t index, uint8_t subindex, uint32_t data, uint8_t cmd)
{uint8_t can_msg[8], can_length;
uint32_t can_id;

	can_id = SDO_RX_SLAVE|node_id;
	can_msg[0] = cmd;
	can_msg[1] = (uint8_t) index;
	can_msg[2] = (uint8_t) (index>>8);
	can_msg[3] = (uint8_t) subindex;
	can_msg[4] = (uint8_t) (data);
	can_msg[5] = (uint8_t) (data>>8);
	can_msg[6] = (uint8_t) (data>>16);
	can_msg[7] = (uint8_t) (data>>24);

	if ((cmd==WR)||(cmd==WR_4BYTE)) can_length = 8;
	if (cmd==WR_2BYTE) can_length = 6;
	if (cmd==WR_1BYTE) can_length = 5;

	if ((cmd==RD)||(cmd==RD_4BYTE)) can_length = 8;
	if (cmd==RD_2BYTE) can_length = 6;
	if (cmd==RD_1BYTE) can_length = 5;

	if ((cmd==RQ)||(cmd==ABORT_MSG)||(cmd==WR_RESPONSE)) can_length = 8;

	#ifdef CAN_1
	if (canopen_channel==1)
		{if (can_1_write_tx_data(can_id, can_length, can_msg)) return(1);}
	#endif

	#ifdef CAN_2
	if (canopen_channel==2)
		{if (can_2_write_tx_data(can_id, can_length, can_msg)) return(1);}
	#endif

	return(0);

}

/**
  * @brief  Отправить пакет PDO (протокол CanOpen)
  *
  * @param  pdo_n: номер PDO: 1, 2, 3, 4
  * 		node_id: адрес устйроства CanOpen (от 1 до 32)
  * 		*data: указатель на буфер с данными (от 0 до 8 байт)
  * 		ln: размер данных
  * @retval 0:не отправлен 1:отправлен
  */
uint8_t CanOpen_tx_pdo(uint8_t pdo_n, uint8_t node_id, int8_t *data, uint8_t ln)
{uint8_t can_msg[8], can_length, cnt;
uint32_t can_id;
//формирование пакета
	if (pdo_n>4)  pdo_n=4;//PDO бавают только: 1,2,3,4
	if (pdo_n==0) pdo_n=1;
	if (ln>8) ln=8;

	can_id = ((uint16_t)pdo_n+1)<<8; //адреса пакетов PDO1=0x200;PDO2=0x300;PDO3=0x400;PDO4=0x500
	can_id+=node_id;
	can_length = ln;
	for (cnt=0; cnt<ln; cnt++) can_msg[cnt] = data[cnt];

	#ifdef CAN_1
	if (canopen_channel==1)
		{if (can_1_write_tx_data(can_id, can_length, can_msg)) return(1);}
	#endif

	#ifdef CAN_2
	if (canopen_channel==2)
		{if (can_2_write_tx_data(can_id, can_length, can_msg)) return(1);}
	#endif

	return(0);
}

/**
  * @brief  Выделяет из CAN адреса (COB_ID), адрес модуля CanOpen (NODE ID)
  *
  * @param  adr: COB ID (адрес CAN)
  *
  * @retval NOD ID (адрес модуля CanOpen)
  */
uint8_t CanOpen_get_nodeid(uint32_t adr)
{
	return(adr&0x0000003F);
}

/**
  * @brief  Выделяет из CAN адреса (COB_ID), номер объекта (OBJECT)
  *
  * @param  adr: COB ID (адрес CAN)
  *
  * @retval номер объекта: NMT, SYNC, TIME STAMP, EMERGENCY, PDO_TX, PDO_RX, SDO_TX, SDO_RX
  */
uint16_t CanOpen_get_object(uint32_t adr)
{
	return(adr&0x000007C0);
}

/**
  * @brief  Выделяет индекс из объекта SDO
  *
  * @param  *data: указатель на данные CAN
  *
  * @retval индекс SDO
  */
uint16_t CanOpen_get_index(uint8_t *data)
{uint16_t index;
	index=((uint16_t)data[2]<<8)|data[1];
	return(index);
}

/**
  * @brief  Выделяет подиндекс из объекта SDO
  *
  * @param  *data: указатель на данные CAN
  *
  * @retval подиндекс SDO
  */
uint8_t CanOpen_get_subindex(uint8_t *data)
{
	return(data[3]);
}
