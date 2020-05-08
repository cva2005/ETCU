/*
 * ds2482.h
 *
 *  Created on: 7 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DRIVERS_DS2482_H_
#define APP_DRIVERS_DS2482_H_
#include "stm32f4xx_hal.h"

#define DS2482_I2C hi2c1 //канал I2C к которому подключена микросхема DS2482

#define DS2482_ADRESS_AD 1 //Адрес микросхемы, задаваемый аппаратно ножками AD2, AD1, AD0

#define DS2482_RAPEAT_CNT 10	//количество попыток передачи команды DS2482 в случае ошибки I2C
#define DS2482_RAPEAT_DELAY 2	//таймаут между попытками предачи в I2C в случае ошибки
#define DS2482_MAX_ERR	15		//максимальное количетсво неудачных попыток записи/чтения, после чего микросхема считается неисправной

void ds2482_init(void);						//Инициализция микросхемы DS2482
uint8_t ds2482_set_chanel(uint8_t chan);	//Изменяет рабочий канал микросхемы DS2482
uint8_t ds2482_get_busy(void);				//Читает состояние регистра статуса DS18B20 и возвращает сосотяние бита 1WB
uint8_t ds2482_send_present_1wire(void);	//Формирует present импульс в линию 1-wire
uint8_t ds2482_get_present_1wire(void);		//Возвращает ответ на последний present импульс: данные дейтвительны только после ds2482_get_busy()==0
uint8_t ds2482_wr_1wire(uint8_t data);		//Передать 1 байт по 1-wire
uint8_t ds2482_rd_bit_cmd_1wire(void);		//Команда: начать чтение одного бита данных из 1-wire
uint8_t ds2482_rd_bit_result_1wire(void);	//Возвращает результат чтения одного бита данных из 1-wire: данные действительны только если ds2482_get_busy()==0
uint8_t ds2482_rd_byte_cmd_1wire(void);		//Команда: начать чтение байта данных из 1-wire
uint8_t ds2482_rd_byte_result_1wire(void);	//Возвращает результат чтения байта данных из 1-wire: данные действительны только если ds2482_get_busy()==0
static uint8_t ds2482_wr_repeat(uint8_t* data, uint8_t size, uint8_t repeat); //Отправляет микросхеме DS2482 заданное количество байт, в случае ошибки передачи, повторяет отправку заданное количество раз
static uint8_t ds2482_rd_repeat(uint8_t* data, uint8_t size, uint8_t repeat); //Читает из микросхемы DS2482 заданное количество байт, в случае ошибки чтения, повторяет чтение заданное количество раз

#define DS2482_ADRESS_BASE 0x18	//базовый адрес микросхемы DS2482
#define DS2482_ADRESS (DS2482_ADRESS_BASE+DS2482_ADRESS_AD) //Результирующий адрес: базовый + задаваемый линиями AD
#define DS2482_ADRESS_7BIT(__ADDRESS__) ((uint8_t)((__ADDRESS__) << 1)& (~I2C_OAR1_ADD0) ) //преобразование адреса в 7-ми битный

typedef union
	{
	struct
		{
		uint8_t WB: 1;	//1-wire занята
		uint8_t PPD: 1;	//ответ на present pulse
		uint8_t SD: 1;
		uint8_t LL: 1;
		uint8_t RST: 1;
		uint8_t SBR: 1;	//ответ на чтение одиночного бита
		uint8_t TSB: 1;
		uint8_t DIR: 1;
		}sr;
	uint8_t byte;
	}ds2482_resp_t; //сосотяние регистра статуса

//---------------Команды DS2482---------------
#define DS2482_DRST 0xF0	//Сброс DS2482: параметров нет
#define DS2482_SRP 0xE1		//Установить указатель на регистр: 1 параметр - номер регистра
#define DS2482_WCFG 0xD2	//Записать конфигурацию: 1 параметр - байт конфигурации
#define DS2482_CHSL 0xC3	//Выбрать канал: 1 параметр - код выбора канала
#define DS2482_1WRS 0xB4	//Сгенерировать Reset/Presence detect цикл на линию 1-Wire: параметров нет
#define DS2482_1WWB 0xA5	//Передать байт данных в линию 1-wire: 1 параметр - байт данных
#define DS2482_1WRB 0x96	//Прочитать байт данных по линии 1-wire: параметров нет
#define DS2482_1WSB 0x87	//1-wire одиночный бит: 1 параметр - битовый байт
#define DS2482_1WT 0x78		//Тройной 1-wire (2 чтения + 1 запись): 1 параметр - байт напрвления для записи

//--------------Пареметры команды SPR---------
#define DS2482_SRP_SR 0xF0		//Регистр статуса
#define DS2482_SRP_RDR 0xE1		//Регистр чтения данных по 1-wire
#define DS2482_SRP_CHSL 0xD2	//Регистр выбора канала
#define DS2482_SRP_CFG 0xC3		//Регистр конфигурации
//--------------Биты конфигурации для команды WCFG---------
#define DS2482_CFG_RESET 0xF0	//начальное состояние регистра конфигурации
#define DS2482_CFG_APU	0xE1	//Запись: Active Pullup (APU)
#define DS2482_CFG_PPM	0xD2	//Запись:Presence Pulse Masking (PPM)
#define DS2482_CFG_SPU	0xB4	//Запись:Strong Pullup (SPU)
#define DS2482_CFG_1WS	0x78	//Запись:1-Wire Speed (1WS)
//--------------Код выбора канала для команды CHSL---------
#define DS2482_CHSL_IO0_WR 0xF0 //Код записи выбора канала 0
#define DS2482_CHSL_IO1_WR 0xE1 //Код записи выбора канала 1
#define DS2482_CHSL_IO2_WR 0xD2 //Код записи выбора канала 2
#define DS2482_CHSL_IO3_WR 0xC3 //Код записи выбора канала 3
#define DS2482_CHSL_IO4_WR 0xB4 //Код записи выбора канала 4
#define DS2482_CHSL_IO5_WR 0xA5 //Код записи выбора канала 5
#define DS2482_CHSL_IO6_WR 0x96 //Код записи выбора канала 6
#define DS2482_CHSL_IO7_WR 0x87 //Код записи выбора канала 7

#define DS2482_CHSL_IO0_RD 0xB8	//Код ответа выбора канала 0
#define DS2482_CHSL_IO1_RD 0xB1	//Код ответа выбора канала 1
#define DS2482_CHSL_IO2_RD 0xAA	//Код ответа выбора канала 2
#define DS2482_CHSL_IO3_RD 0xA3	//Код ответа выбора канала 3
#define DS2482_CHSL_IO4_RD 0x9C	//Код ответа выбора канала 4
#define DS2482_CHSL_IO5_RD 0x95	//Код ответа выбора канала 5
#define DS2482_CHSL_IO6_RD 0x8E	//Код ответа выбора канала 6
#define DS2482_CHSL_IO7_RD 0x87	//Код ответа выбора канала 7
//--------------Битовый байт для команды 1WSB и 1WT-------------
#define DS2482_BITBYTE_0 0x00	//Бит = 0 для комнады записи бита
#define DS2482_BITBYTE_1 0x80	//Бит = 1 для команды записи бита

#endif /* APP_DRIVERS_DS2482_H_ */
