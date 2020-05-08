/*
 * pc_device.h
 *
 *  Created on: 11 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DEVICES_PC_DEVICE_H_
#define APP_DEVICES_PC_DEVICE_H_
#include "types.h"

#define PC_DEVICE_MAX_SESSION 2 		//максимальное число подключений (устйроств)
#define PC_DEVICE_TIMEOUT_SESSION 5000	//время по истечении которого ссесия автоматически закрывается, если не было никаких данных
#define PC_DEVICE_TX_TIME 100			//интервал между отправками пакетов каждому из подключенных устйроств

//#define PC_DEVICE_DATA_SZIE 1000		//максимальный размер поля данных (должен быть не больше чем размер поля данных в модуле pc_link)

#define PC_DEVICE_RQ_CONNECT 0x10		//запрос соединения
#define PC_DEVICE_RSP_CONNECT 0x11		//ответ на запрос соединения
//#define PC_DEVICE_SIG 1					//таблица сигналов
//#define PC_DEVICE_DATA 2				//пакет данных
//#define PC_DEVICE_SYS 3					//пакет системных данных
//#define PC_DEVICE_RST 5					//сброс к заводским настройкам

#define FLD_MODE_CONNECTION 0
#define FLD_ERR_CONNECTION 1
#define FLD_SESSION 2
//#define FLD_RQ_PARAMETR 1
//#define FLD_WR_PARAMETR 1
//#define FLD_STATUS 0

#define DT_CONNECTION_ERR 0 //ошибка подключения устйроства (нет свободных сессий)
#define DT_CONNECTION_CTR 1	//подключение в режиме управления
#define DT_CONNECTION_MNT 2	//подключение в режиме мониторинга
#define DT_SESSION_BROADCAST 0	//запрос подключения (широковещательный пакет без номера сессии)

//#define RESULT_NO_PARAMETR  0x01
//#define RESULT_NOT_ALL_PR	0x02
//#define RESULT_NOT_WRITE	0x04

typedef struct
	{
	uint16_t session; //Идентификатор соединения (сессии)
	stime_t time; //время при достижении которого ссессия считается разорванной
	stime_t tx_time; //время когда надо отправить следующий пакет
	uint8_t mode;	//режим работы сессии (управлени или контроль)
	uint8_t type;	//тип пакетов, которые нобходимо отправить
	}pc_device_session_t;

void pc_device_init(void);	//Инициализация устройства ПК
void pc_device_step(void);	//Шаг обработки данных устройства ПК
void pc_device_rx_callback(uint8_t type, uint8_t size, void *pack); //Приём: предопределённая функция вызова обработчика верхнего упровня: обработчик приложения или обработчик алгоритма управления
static void pc_device_process_rx(void);	//Обабатывает данные полученные для ПК
static void pc_device_prepare_tx(void);	//Формирует данные для отправки на ПК

#endif /* APP_DEVICES_PC_DEVICE_H_ */
