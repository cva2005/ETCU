/*
 * spsh20.h
 *
 *  Created on: 26 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DEVICES_SPSH20_H_
#define APP_DEVICES_SPSH20_H_
#include "types.h"

//#define SPSH20_LOOP_CONTROL			//: если поределено, то режим контурного управлени


#define SPSH20_CONNECT_TIME 200		//Макисмальное допустимое время ожидания пакетов, при превышении считается связь с BCU потерянной
#define SPSH20_DATA_TX_TIME 20		//Таймаут между отправляемыми пакетами команды
#define SPSH20_DATA_TX_RQ_TIME 2	//Таймаут между отправляемыми пакетами запроса

#define SPSH20_MAX_POSITION 	 0
#define SPSH20_MIN_POSITION 	-420000

#define MAX_SERVO_POSITION 	SPSH20_MAX_POSITION
#define MIN_SERVO_POSITION	SPSH20_MIN_POSITION

typedef enum { //параметры : байты поменяны местами
	PR_CT3_POS = 0x0402,	// заданная позиция 0x0204 (Сигнал управления при замкнутом контуре положения -2147483647;2147483647)
	PR_DD8_POS = 0x0C04,	// текущая позиция 0x040C
	PR_UP16_I = 0x1007,		// текущий ток 0x0710
	PR_DD11_ST = 0x0F04,	// Сосотяние привода 0x040F
	PR_CT2_SP = 0x0202,		// Задание скорости об/мин 0x0202 (Сигнал управления при разомкнутом контуре положения -12000;12000)
	PR_DD4_SP = 0x0804,		// Текущая скорость вращения об/мин 0x0408 (Текущая скорость вращения -5000;5000)
	PR_DD5_SP = 0x0904,		// Заданная скорость вращения об/мин 0x0409 (Заданная скорость вращения -5000;5000)
} spsh20_pr_t; //параметры привода СПШ-20

typedef enum {
	STATE_GET = 0,
	POSITION_GET,
	POSITION_SET
} spsh20_tx_t; // тип запроса СПШ-20


#pragma pack(1)
typedef union
	{
	struct
		{
		int32_t pos;		//положение/скорость
		struct
			{
			uint8_t out0 :1;	//значение выхода 1
			uint8_t out1 :1;	//значение выхода 2
			uint8_t en :1;		//сигнал разрешения работы ШИМ
			uint32_t speed :29;	//значение скорости
			}s;
		}f;
	uint8_t byte[8];
	uint16_t word[4];
	uint32_t dword[2];
	}spsh20_tx_data_t; //структура отправляемого пакета

typedef union
	{
	struct
		{
		int32_t pos;		//положение
		struct
			{
			uint8_t din :4;	//значение дискретных входов
			uint8_t st :2;		//состояние привода
			uint16_t ain0 :12;	//значение аналогового входа 1
			uint16_t ain1 :12;	//значение аналогового входа 2
			}s;
		}f;
	uint8_t byte[8];
	uint16_t word[4];
	uint32_t dword[2];
	}spsh20_rx_data_t; //структура отправляемого пакета
#pragma pack()

void spsh20_init(uint8_t id);			//Инициализация устйроства управления гидротормозом (BCU)
void spsh20_step(void);					//Шаг обработки данных устйроства управления гидротормозом (BCU)
void spsh20_set_pos (int32_t data);		//Устанавливает значение позиции
int32_t spsh20_get_pos (void);				//Возвращает положение привода
uint8_t spsh20_get_status (void);			//Возвращает статус привода
uint8_t spsh20_err_link (void);				//Возвращает состояние связи
void spsh20_update_data (char *data, uint8_t len, uint32_t adr);	//Обработчик пакетов принятых от сервопривода


#endif /* APP_DEVICES_SPSH20_H_ */
