/*
 * atv61.h
 *
 *  Created on: 4 февр. 2016 г.
 *      Author: Перчиц А.Н.
 */
/*
 * Частотный преобразователь Altivar 61/71
 */
#ifndef APP_DEVICES_ATV61_H_
#define APP_DEVICES_ATV61_H_
#include "types.h"

#define ATV61_CONNECT_TIME 2000	//Макисмальное допустимое врем ожидания пакетов от ATV61, при превышении считается связь с ATV61 потерянной
#define ATV61_DATA_TX_TIME 200	//Таймаут между отправляемыми пакетами к ATV61
#define ATV61_DATA_TX_INIT 200	//Таймаут между отправляемыми пакетами инициализации
#define ATV61_MAX_ERR_SEND	2	// Максимально допустимое количество запросов без ответа

#define ATV61_STOP 0
#define ATV61_TURN_LEFT 1
#define ATV61_TURN_RIGHT 2

#pragma pack(1)
typedef union
	{
	struct
		{
		uint8_t switch_on:1;	//Switch On
		uint8_t en_volage:1;	//Блокировка напряжения
		uint8_t quick_stop:1;	//Быстрая остановка
		uint8_t enable_opr:1;	//Enable operation
		uint8_t reserv1:3;		//
		uint8_t fault_reset:1;	//Cброс ошибки
		uint8_t halt:1;			//
		uint8_t reserv2:2;		//
		uint8_t revers:1;		//Движение в обратную сторону
		uint8_t reserv3:4;		//
		}cmd; //командное слово
	struct
		{
		uint8_t rdy_switch:1;	//Готовность привода
		uint8_t switch_on:1;	//Готовность привода к запуску
		uint8_t opr_en:1;		//Привод работает
		uint8_t fault:1;		//Сбой привода активен
		uint8_t voltage_en:1;	//Блокировка напряжения
		uint8_t quick_stop:1;	//Быстрая остановка
		uint8_t switch_on_dsb:1;//Запрещение включения активно
		uint8_t warning:1;		//Предупреждение привода активно
		uint8_t reserv1:1;		//
		uint8_t remote:1;		//Управление по сети
		uint8_t target_reached:1;//Частота достигнута
		uint8_t internal_limit:1;//Задание вне заданных пределов
		uint8_t reserv2:2;		//
		uint8_t stop_key:1;		//Остановка от клавиши СТОП
		uint8_t revers:1;		//Направление вращения двигателя назад
		}eta; //слово состояния
	uint8_t byte[2];
	uint16_t word;
	}atv_cmd_st_t;

typedef union
	{
	struct
		{
		atv_cmd_st_t cmd_st;
		uint16_t frequency;
		}f;
	uint8_t byte[4];
	uint16_t word[2];
	}atv_pdo1_t; //структура отправляемого пакета
#pragma pack()

void atv61_init(uint8_t ch, uint8_t node_id); //Инициализация устйроства ATV61
static void atv61_cmd_prepare(void);		//Подготавливает командное слово для ATV61 в зависимости от команды пользователя и состояния привода
void atv61_step(void);					//Шаг обработки данных устйроства ATV61
void atv61_set_frequency(int32_t data);		//Устанавливает частоту ATV61
int32_t atv61_get_frequency(void);		//Возвращает значение частоты ATV61
uint8_t atv61_error (void);			//Возвращает состояние ошибки ПЧ
uint8_t atv61_err_link (void);			//Возвращает состояние связи с ATV61
#ifdef ATV61_MB
void atv61_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function);
#else
void atv61_update_data (char *data, uint8_t len, uint32_t adr);	//Обработчик пакетов принятых по CanOpen от ATV61
#endif

/*
 * Адреса регистров MODBUS
 */

#define MB_Status_word1		3201 // R-O
#define MB_Output_frequency	3202 // R-O
#define MB_Output_torque	3205 // R-O
#define MB_Control_word1	8501 // R/W
#define MB_Frequency_ref	8502 // R/W ( 0.1 Hz INT 16 signed bits)
#define MB_Control_wordExt	8504 // R/W
#define MB_Control_word2	8601 // R/W
#define MB_Speed_ref		8602 // R/W (rpm INT 16 signed bits)
#define MB_Status_word2		8603 // R-O
#define MB_Output_speed		8604 // R-O

#endif /* APP_DEVICES_ATV61_H_ */
