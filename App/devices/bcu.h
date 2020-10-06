/*
 * bcu.h
 *
 *  Created on: 10 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
/* Модуль управления гидротормозом
 * Датчик давления: DMP 330L-4003-1-100-500-01R-ГП
 * Датчик температуры: 421.3828
 */
#ifndef APP_DEVICES_BCU_H_
#define APP_DEVICES_BCU_H_
#include "types.h"

#define BCU_CONNECT_TIME 200	//Макисмальное допустимое врем ожидания пакетов от BCU, при превышении считается связь с BCU потерянной
#define BCU_DATA_TX_TIME 50		//Таймаут между отправляемыми пакетами к BCU
#define BCU_DATA_TX_INIT 50		//Таймаут между отправляемыми пакетами инициализации

#define	BCU_MAX_PRESSURE 400000  //максимальное давление датчика DMP330L, мБар
#define BCU_I_MIN_PRESSURE 4000	 //минимальное значение тока в мкА при 0 датчика давления DMP330L
#define BCU_I_MAX_PRESSURE 20000 //минимальное значение тока в мкА при максимуме датчика давления DMP330L
#define BCU_FAN_MASK		0x01 // RELE 1 Вентилятор охлаждения масла
#define BCU_PUMP_MASK		0x02 // RELE 2 упр. насосом гидропривода
#define BCU_LIGHT_MASK		0x04 // RELE 3 световая индикация
#define BCU_SOUND_MASK		0x08 // RELE 4 звуковая индикация
#define BCU_I_MAX_PRESSURE 20000	//минимальное значение тока в мкА при максимуме датчика давления DMP330L
#define BCU_I_MAX_PRESSURE 20000	//минимальное значение тока в мкА при максимуме датчика давления DMP330L
#define Q_TAU				0.5f
#define Q_INT_SEC			2
#define Q_INTEGRAL_TIME		(Q_INT_SEC * 1000)
#define Q_T_MUL				(60000 / Q_INT_SEC)
#pragma pack(1)
typedef union {
	struct {
		uint16_t pwm1;		//ШИМ1
		uint16_t pwm2;		//ШИМ2
		uint16_t position;	//Положение сервопривода
		uint8_t reserv;		//
		uint8_t out;		//Значение дискретных выходов
	} fld;
	uint8_t byte[8];
	uint16_t word[4];
	uint32_t dword[2];
} bcu_tx_data_t; //структура отправляемого пакета

#pragma pack()

void bcu_init (uint8_t node_id);			//Инициализация устйроства управления гидротормозом (BCU)
void bcu_step (void);					//Шаг обработки данных устйроства управления гидротормозом (BCU)
void bcu_set_pwm1 (int32_t data);		//Устанавливает значение ШИМ1
void bcu_set_pwm2 (int32_t data);		//Устанавливает значение ШИМ2
void bcu_set_position (int32_t data);	//Устанавливает положение сервопривода
void bcu_set_out (uint8_t data);			//Устанавливает значение дискретных выходов
uint8_t bcu_get_in (void);				//Возвращает состояние дискретных входов
int32_t bcu_get_t (void);				//Возвращает значение датчика температуры
int32_t bcu_get_p (void);				//Возвращает значение датчика давления
int32_t bcu_get_position (void);		//Возвращает положение сервопривода
int32_t bcu_get_torque (void);			//Возвращает значение крутящего момента с датчика момента
int32_t bcu_get_frequency(void);		//Возвращает значение частоты с датчика момента
int32_t bcu_get_power(void);			//Возвращает значение мощности с датчика момента
uint8_t bcu_err_link (void);			//Возвращает состояние связи с BCU
void bcu_update_data (char *data, uint8_t len, uint32_t adr);	//Обработчик пакетов принятых по CanOpen от BCU
uint32_t bcu_get_Q (void); // расход л/мин

#endif /* APP_DEVICES_BCU_H_ */
