#ifndef LA10P_H_
#define LA10P_H_
#include "arm_math.h"
#include <stdbool.h>
#include "_signals.h"

/*typedef enum {
	LA10P_POWERED = 0,
	LA10P_STOP_ERR = 1,
	LA10P_NOT_INIT = 2,
	LA10P_INIT_RUN,
	LA10P_READY
} la10p_st;*/

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
#if MODEL_NO_SERVO
	#define STATE_SENS()	StateVal
#else
	#define STATE_SENS()	(float32_t)st(AI_T_OIL_OUT)
#endif
#define FORWARD_MOV			DO_OIL_HEATER
#define REVERS_MOV			DO_COOLANT_HEATER
#define LA10P_FULL_TIME		(ST_LENGT * 1000 / ST_MOVE + 2000) // ms
#define LA10P_ERR_TIME		LA10P_FULL_TIME + 1000
#define LA10P_MIN_TIME		(LA10P_FULL_TIME - LA10P_FULL_TIME / 4)
#define LA10P_MUL			5.00f
#define STEP_TIME			100 // время шага опроса привода, мс
#define DEAD_TIME			50 // время мертвого хода, мс
#define RELE_TIME			200 // время шага управления реле, мс
#define SENS_MAX_VAL		3000.0f // максимальное выходное значение, мВ
#define SENS_ERR_VAL		3300.0f // максимальное выходное значение, мВ
#define SENS_MIN_VAL		0.0f // минимальное выходное значение, мВ
#define SENS_I_MUL			0.6666666666666667f // входной делитель
#define SENS_I_mV_A			185.0f // Sensitivity (mV/A)
#define CURR_SENS_VAL		(float32_t)st(AI_P_MANIFOLD)
#define CURR_SENS_mV		((CURR_SENS_VAL - CurrNull) / SENS_I_MUL)
#define CURR_SENS_A			fabs(CURR_SENS_mV / SENS_I_mV_A)
#define SENS_I_MAX			4.2f
#define SENS_I_OFF			0.2f
#define FLOAT_MIN			0.0001

#define FORW_RUN()			set(FORWARD_MOV, ON)
#define FORW_STOP()			set(FORWARD_MOV, OFF)
#define REVR_RUN()			set(REVERS_MOV, ON)
#define REVR_STOP()			set(REVERS_MOV, OFF)
#define FORW_MOV			st(FORWARD_MOV)
#define REVR_MOV			st(REVERS_MOV)

void la10p_init(void);
void la10p_step(void);
float32_t la10p_get_pos(void);
void la10p_set_out(float32_t pid_out);
la10p_st la10p_state(void);

#endif /* LA10P_H_ */
