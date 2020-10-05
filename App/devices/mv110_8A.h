#ifndef APP_DEVICES_MV8A_H_
#define APP_DEVICES_MV8A_H_
#include "types.h"
#include "timers.h"
#include "_control.h"

#pragma pack(1)
typedef	struct {
	uint8_t status;
	uint8_t	time;
	uint8_t fdata[4];
} mv8a_rx_t;
#pragma pack()

#define MV8A_CONNECT_TIME	1000 // Время ответа при прев. связь считается потерянной
#define MV8A_DATA_TX_TIME	200	// Таймаут между отправляемыми пакетами
#define MV8A_MAX_ERR_SEND	3	// Допустимое количество запросов без ответа
#define MV8A_NUM 			2 	// количество модулей MV110-8A
#define CH_NUM				8 	// количество каналов в MV110-8A
#define INP_NUM				15 	// количество используемых каналов измерения
#define MV8A_INP			INP_NUM

void mv8a_init (uint8_t ch, uint8_t addr);	// Инициализация устйроства
int32_t mv8a_read_res (uint8_t ch);	// результат измерения
uint8_t mv8a_err_link (void);	// Возвращает состояние связи с MV8A
void mv8a_step (void);

/*
 * Адреса регистров MODBUS
 * Статус входа 1		Int16 		0x0002
 * Цикл. время изм.		Int16 		0x0003
 * Измерение входа 1	Float32 	0x0004, 0x0005
 * Статус входа 2		Int16 		0x0008
 * Цикл. время изм.		Int16 		0x0009
 * Измерение входа 2	Float32 	0x000A, 0x000B
 * Статус входа 3		Int16 		0x000E
 * Цикл. время изм.		Int16 		0x000F
 * Измерение входа 3	Float32 	0x0010, 0x0011
 * Статус входа 4		Int16 		0x0014
 * Цикл. время изм.		Int16 		0x0015
 * Измерение входа 4	Float32 	0x0016, 0x0017
 * Статус входа 5		Int16 		0x001A
 * Цикл. время изм.		Int16 		0x001B
 * Измерение входа 5	Float32 	0x001C, 0x001D
 * Статус входа 6		Int16 		0x0020
 * Цикл. время изм.		Int16 		0x0021
 * Измерение входа 6	Float32 	0x0022, 0x0023
 * Статус входа 7		Int16 		0x0026
 * Цикл. время изм.		Int16 		0x0027
 * Измерение входа 7	Float32 	0x0028, 0x0029
 * Статус входа 8		Int16 		0x002C
 * Цикл. время изм.		Int16 		0x002D
 * Измерение входа 8	Float32 	0x002E, 0x002F
 * 1. Все регистры только для чтения. Регистры считываются
 * командами 03 или 04 (прибор поддерживает обе команды).
 * 2. Во время передачи четырехбайтовых значений (тип Float32)
 * старшее слово передается в регистре с меньшим номером.
 */

#define	STATUS_REG	2
#define RESULT_REG	4
#define CH_REG_NUM	4 // регистров для чтения канала
#define CH_OFFSET	6 // смещение регистров каналов

#endif /* APP_DEVICES_MV8A_H_ */
