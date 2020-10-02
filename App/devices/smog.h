#ifndef APP_DEVICES_SMOG_H_
#define APP_DEVICES_SMOG_H_
#include "types.h"
#include "timers.h"
#include "_control.h"


#define SMOG_CONNECT_TIME	1000 // Время ответа при прев. связь считается потерянной
#define SMOG_DATA_TX_TIME	1500 // Таймаут между отправляемыми пакетами
#define REINIT_DELAY		500
#define SMOG_MAX_ERR_SEND	3 // Допустимое количество запросов без ответа
#define CH_N				4 // каналов измерения
#define SMG_CH				6 // результаты измерения + status + error
#define N_BYTE				3 // байт в поле данных

#pragma pack(1)
typedef	struct {
	struct {
	    uint8_t Point:	3; // положение запятой
	    uint8_t Unused:	4;
	    uint8_t Sign:	1; // Бит ЗНАК
	    uint8_t	Data[N_BYTE]; // измеренное значение (цифры от 0 до 9 десятичные)
	} ch[CH_N];
    uint16_t Empty[2];
    uint8_t Error;
    uint8_t Status;
} smog_rx_t;
#pragma pack()

void smog_init (uint8_t ch, uint8_t addr); // Инициализация устйроства
int32_t smog_read_res (uint8_t ch);
uint8_t smog_err_link (void); // Возвращает состояние связи
void smog_step (void);

/*
 * Адреса регистров MODBUS:
 * Дымность, приведенная к эффективной фотометрической
 * длине дымомера L = 430 мм, N[0 ... 100%]
 * N0,43		Int16 		0x0000
 * Коэффициент поглощения светового потока, K	[0 ... +Inf]
 * K			Int16 		0x0002
 * Дымность, NН [0 ... 100%]
 * N			Int16 		0x0004
 * Ошибки БОИ	Int16 		0x000D
 * Статус БОИ	Int16 		0x000E
 */
#define N0_43_REG	0x0000 // Дымность N0,43
#define K_REG		0x0002 // Коэффициент поглощения
#define NH_REG		0x0004 // Дымность, NН
#define ERR_REG		0x000D // Ошибки БОИ
#define STAT_REG	0x000E // Статус БОИ
#define REG_NUM		(STAT_REG + 1) // количество регистров для чтения в SMOG

// Описание регистра «статус БОИ»:
#define MEAS_MODE	00 // режим измерения
#define WARMING		04 // прогрев
#define CORRECTION	01 // идет корректировка

// Описание регистра «ошибки БОИ»:
#define LAMP_FAULT	01 // неисправна лампа
#define T_SENS_ERR	02 // неисправен датчик температуры
#define P_SENS_ERR	03 // неисправен датчик давления
#define COMM_ERR	08 // ошибка приема

#endif /* APP_DEVICES_SMOG_H_ */
