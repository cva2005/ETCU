#ifndef LA10P_H_
#define LA10P_H_
#include "arm_math.h"
#include <stdbool.h>
#include "_signals.h"

typedef enum {
	LA10P_READY = 0,
	LA10P_NOT_INIT,
	LA10P_STOP_ERR
} la10p_st;

/*
 * Линейный привод: 	LA10P 24В
 * Датчик положения: 	10кОм
 * Working duty: 		25%
 * Непрерывный Ток: 	4A
 * Выходная мощность: 	48W
 * Скорость линейная: 	20[mm/s]
 * Нагрузка: 			450N = 45кг
 * 150 мм 6-дюймовый ход
 * Рабочая температура -26 ~ + 65 градусов
 * IP Max IP 65
 */
#define ST_LENGT			150 // длина штока
#define ST_MOVE				20  // скорость перемещения штока [мм/сек]
#define STATE_SENS()		st(AI_T_FUEL)
#define FORWARD_MOV			DO_OIL_HEATER
#define REVERS_MOV			DO_COOLANT_HEATER
#define LA10P_FULL_TIME		(ST_LENGT * 1000 / ST_MOVE) // ms
#define LA10P_ERR_TIME		LA10P_FULL_TIME
#define LA10P_INIT_TIME		LA10P_FULL_TIME * 2
#define STEP_TIME			100 // время шага управления приводом, мс

void la10p_init(void);
void la10p_step(void);
float32_t la10p_get_pos(void);
void la10p_set_out(float32_t pid_out);

#endif /* LA10P_H_ */
