/*
 * servotech_link.h
 *
 *  Created on: 26 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_INTERFACES_SERVOTECH_LINK_H_
#define APP_INTERFACES_SERVOTECH_LINK_H_
#include "types.h"

#define SERVOTECH_LINK_MAX_DEV 1 //максимальное возможное количество устйроств
#define SERVOTECH_LINK_ADR 0 	 //адрес устройства мастера (этого контрпроллера)
#define SERVOTECH_LINK_MODE 0	 //режим работы линии: мастер, слейв, контурного управления, синхрнизация

#define SYNC_ID 0x87	//ID пакета синхронизации


typedef enum
	{
	CMD_SET_POS = 8,	//команда установки позиции, скорости и сигнала разрешения портов
	CMD_SET_SPEED = 9,	//команда установки скорости
	CMD_SET_I = 10,		//команда установки тока
	CMD_ST_POS = 12,	//команда текущего положения
	CMD_TX_USB = 13,	//команда отправки пакета по технологическому протоколу
	CMD_RX_USB = 14,	//приём пакет по технологическому протоколу
	}servotech_link_cmd_t; //команды обмена по CAN
typedef enum
	{
	TECH_RQ = 5,	//Запрос значения параметра: Команда передается внешним контроллером при запросе значения параметра
	TECH_RSP = 6,	//Передача текущего значения параметра: Команда передается приводом в ответ на команду 5
	TECH_SET = 7,	//Установка значения параметра: Команда передается внешним контроллером при установке значения параметра
	TECH_START = 17	//Запустить программу ПЛК
	}servotech_link_tech_t; //команды технологического протокола

#pragma pack(1)
typedef union
	{
	struct
		{
		uint8_t sink_addr: 3;		//адрес назначения
		uint8_t source_addr: 3;		//адрес источника
		uint8_t cmd: 5;	//команда
		uint32_t notuse: 21;
		}f;
	uint8_t byte[4];
	uint16_t word[2];
	uint32_t dword;
	}servotech_link_id_t; //формат идентификатора
#pragma pack()

typedef void (*servotech_link_rx_t) (char *data, uint8_t len, uint32_t adr); //описание процедуры - обработчика принятых пакетов

void servotech_link_init(uint8_t chanal);//Инициализация интерфейса
uint8_t servotech_link_set_channel(uint8_t channel);//Изменяет канал CAN через который работает протокол
void servotech_link_step(void);//шаг обмена: необходимо добавить в основной цикл
uint8_t servotech_link_tx_tech(uint8_t adr, servotech_link_tech_t cmd, uint16_t parameter, uint32_t data);// Отправить пакет в технологический протокол
uint8_t servotech_link_tx_cmd(servotech_link_cmd_t cmd, uint8_t adr, int8_t *data, uint8_t ln);//Отправить пакет команды
uint8_t servotech_link_get_src_adr(uint32_t adr);//Выделяет из CAN адреса: адрес источника
uint8_t servotech_link_get_dst_adr(uint32_t adr);//Выделяет из CAN адреса: адрес назначения
uint8_t servotech_link_get_cmd(uint32_t adr);//Выделяет из CAN адреса: команду
uint16_t servotech_link_get_pr(uint8_t *data);//Выделяет из данных значение параметра для технологичесткого протокола
uint8_t servotech_link_get_tech_cmd(uint8_t *data);//Выделяет команду технологического протокола


#endif /* APP_INTERFACES_SERVOTECH_LINK_H_ */
