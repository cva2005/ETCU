/*
 * signals.h
 *
 *  Created on: 29 авг. 2014 г.
 *      Author: Перчиц А.Н.
 */

#ifndef SIGNALS_H_
#define SIGNALS_H_

#ifdef QT_CORE_LIB
#include <QtCore>
#include <stdint.h>
#else
#include "types.h"
#endif

#define REV 1011 //версия протокола
//------------------типы пакетов--------------------------------------------------------
#define PC_LINK_LOSS 0  //нет связи
#define PC_SIG 1	//пакет с таблицей сигналов
#define PC_DATA 2  //пакет данных
#define PC_DEBUG 3 //пакет системных данных
#define PC_RESET 5 //сброс к заводским установкам
#define PC_RQ_CONNECT 0x10		//запрос соединения
#define PC_RSP_CONNECT 0x11		//ответ на запрос соединения

#define ECU_CONTROL (ECU_PED_CONTROL | ECU_TSC1_CONTROL)
//=========================================СИГНАЛЫ==============================================
#pragma pack(1)
typedef union
	{
	struct
		{
		uint8_t activ:1;
		uint8_t type:3;
		uint8_t deivice:4;
		uint8_t number;
		}fld;
	uint8_t byte[2];
	uint16_t word;
	}sig_cfg_t; //структура описания сигнала

//-------------------описание типов сигналов: type-------------------------------------------
#define CFG 0 //конфигурация
#define DO 1 //дискретный выход
#define DI 2 //дискретный вход
#define AO 3 //аналоговый выход
#define AI 4 //аналоговый вход
/* ToDo: сигналы управления! */
//-------------------описание устройств: device----------------------------------------------
#define ETCU 1	//контроллер
#define PC 2	//управляющий ПК
#define BCU 3	//контроллер гидротормоза
#define FC 4	//частотный преобразователь (frequency converter)
#define TA 5	//сервопривод дроссельной заслонки (throttle actuator) СПШ-20
#define CDU 6	//зарядно-разрядное устройство
#define PPU 7	//переносной пуль управления
#define APS 8	//датчик параметров атмосферы (atmosphere parameters sensor)
//-------------------описание сигналов-------------------------------------------------------
enum {
//сигналы конфигурации
	CFG_KEY_DELAY, 	//пауза между опросами кнопок
	CFG_PULSE1,		//импульсов на единицу измерения для имп входа№1
	CFG_PULSE2,		//импульсов на единицу измерения для имп входа№2
	CFG_PULSE3,		//импульсов на единицу измерения для имп входа№3
	CFG_SERVO_MAX_I,//максимальный допустимый ток сервопривода
	CFG_MAX_ENGINE_SPEED, //максимальные обороты электрического мотора
#if REV>1010
	CFG_MAX_SERVO_POSITION,	//максимальное положение сревопривода,
	CFG_U_AKB_NORMAL,		//Минимальное занпряжение АКБ для запуска двигателя
	CFG_STARTER_ON_TIME,	//время включения стартера, если двигатель не запускается
	CFG_MIN_ROTATE,			//Минимальные обороты двигателя для определения его запуска
	CFG_MIN_I_STARTER,		//Минимальный ток стартера
	CFG_FUEL_PUMP_TIMEOUT,	//Время работы ТНВД перед запуском стартера
	CFG_TIME_START_FC,		//Таймаут на запуск ПЧ
	CFG_HYDROSTATION_TETS,	//Процент нажатия торможа при проверки гидростанции
	CFG_SPEED_TIME_HYDRO,	//Время выхода на полную мощность гидростанции
	CFG_TIMOUT_ENGINE_ROTATE, //Времяза которое должен выйти на заданные обороты двигатель
	CFG_TIMOUT_SET_TORQUE,	//Время установки заданного крутящего моента
#endif
//сигналы ETCU
#if ECU_CONTROL
	DO_STARTER,				//Сигнал: Включено зажигание
	DO_COOLANT_FAN,			//Сигнал: Вентилятор ОЖ

	DO_COOLANT_PUMP,		//Сигнал:
	DO_OIL_PUMP,			//Сигнал:
	DO_COOLANT_HEATER,		//Сигнал: Запуск Двигателя
	DO_OIL_HEATER,			//Сигнал:
	DO_FUEL_PUMP,			//Сигнал: Возбуждение Генератора
#else
	DO_STARTER,				//Сигнал: Стратер
	DO_COOLANT_FAN,			//Сигнал: Вентилятор ОЖ

	DO_COOLANT_PUMP,		//Сигнал: Насос ОЖ
	DO_OIL_PUMP,			//Сигнал: Насос масла
	DO_COOLANT_HEATER,		//Сигнал: Нагреватель ОЖ
	DO_OIL_HEATER,			//Сигнал: Нагреватель масла
	DO_FUEL_PUMP,			//Сигнал: Включить ТНВД
#endif
	AI_T_EXHAUST,			//Аналоговый: Температура выхлопных газов
	AI_T_COOLANT_IN,		//Аналоговый: Температура ОЖ на входе
	AI_T_COOLANT_OUT,		//Аналоговый: Температура ОЖ на выходе
	AI_T_OIL_IN,			//Аналоговый: Температура масла на входе
	AI_T_OIL_OUT,			//Аналоговый: Температура масла на выходе
	AI_T_FUEL,				//Аналоговый: Температура топлива
	AP_FUEL_IN,				//Импульсный: подаваемое топливо
	AP_FUEL_OUT,			//Импульсный: возвращаемое топливо
	AI_FUEL_LEVEL,			//Аналоговый: Датчик уровня топлива
	AI_T_EXT1,				//Аналоговый: Датчик темературы локального нагрева 1
	AI_T_EXT2,				//Аналоговый: Датчик темературы локального нагрева 2
	AI_T_EXT3,				//Аналоговый: Датчик темературы локального нагрева 3
	AI_T_EXT4,				//Аналоговый: Датчик темературы локального нагрева 4
	AI_T_EXT5,				//Аналоговый: Датчик темературы локального нагрева 5
	AI_T_EXT6,				//Аналоговый: Датчик темературы локального нагрева 6
	AI_T_EXT7,				//Аналоговый: Датчик темературы локального нагрева 7
	AI_T_EXT8,				//Аналоговый: Датчик темературы локального нагрева 8
	AI_P_EXHAUST,			//Аналоговый: Давление выхлопных газов
	AI_P_OIL,				//Аналоговый: Давление масла
	AI_P_CHARGE,			//Аналоговый: Давление наддувочного воздуха
	AI_T_CHARGE,			//Аналоговый: Температура наддувочного воздуха
	AI_P_MANIFOLD,			//Аналоговый: Давление впускного коллектора
	AI_VBAT,				//Аналоговый: Напряжение батарейки
	AI_5V,					//Аналоговый: Напряжение питания 5В
	AI_TIME,				//Аналоговый: Время
	AI_DATE,				//Аналоговый: Дата
	AI_ROTATION_ETCU,		//Импульсный: количество оборотов в минуту
	AI_I_AKB_P,				//Аналоговый: Ток АКБ втекающий
	AI_I_AKB_N,				//Аналоговый: Ток АКБ вытекающий
	AI_U_AKB,				//Аналоговый: Напряжение АКБ

//Сигналы датчика параметров атмосферы
	AI_T_AIR,				//Аналоговый: Температура окружающего воздуха
	AI_P_AIR,				//Аналоговый: Атмосферное давление
	AI_H_AIR,				//Аналоговый: Влажность окружающего воздуха

//сигналы BCU
	DO_EMERGANCY,			//Сигнал: Авария
	DO_SIREN,				//Сигнал: Сирена
	DO_OIL_FAN,				//Сигнал: Вентилятор охлаждения масла

	AI_P_OIL_BRAKE,			//Аналоговый: Давление масла в гидротормозе
	AI_T_OIL_BRAKE,			//Аналоговый: Температура масла в гидротормозе
	AI_ROTATION_SPEED,		//Аналоговый: Скорость вращения
	AI_TORQUE,				//Аналоговый: Момент вращения
	AI_POWER,				//Аналоговый: Мощность
	AI_VALVE_POSITION,		//Позиция клапана

	AO_HYDROSTATION,		//Производительность гидростанции
	AO_VALVE_POSITION,		//Задаваемая позиция клапана
	AO_VALVE_ENABLE,		//Сигнал вкл/выкл клапана
//сигналы FC
	AO_FC_FREQ,				//заданная частота вращения
	AI_FC_FREQ,				//текущая частота вращения
//сигналы TA
	AI_SERVO_POSITION,		//Аналоговый: Текущее положение сервопривода
	AI_SERVO_ST,			//Аналоговый: Состояние привода
	AI_SERVO_I,				//Аналоговый: Ток обмоток сервопривода

	AO_SERVO_POSITION,		//Задаваемое положение сервопривода
//сигналы PPU

//сигналы CDU
	AI_CDU_U,
	AI_CDU_I,
	AI_CDU_ST,
	AI_CDU_ERR,
//сигналы PC
#if ECU_CONTROL
	DI_PC_TEST_START,			//Сигнал:
	DI_PC_TEST_STOP,			//Сигнал:
	DI_PC_FIRE_ALARM,			//Сигнал: Включить зажигание
	DI_PC_HOT_TEST,				//Сигнал:
	DI_PC_SET_TIME,				//Сигнал:
	DI_PC_BRAKE_TEST,			//Сигнал:

	DO_PC_STARTER,				//Сигнал: Включено зажигание
	DO_PC_COOLANT_FAN,			//Сигнал:
	DO_PC_COOLANT_PUMP,			//Сигнал:
	DO_PC_OIL_PUMP,				//Сигнал:
	DO_PC_COOLANT_HEATER,		//Сигнал:
	DO_PC_OIL_HEATER,			//Сигнал: Лампа "АВАРИЯ" ЭБУ
    DO_PC_FUEL_PUMP,			//Сигнал: Возбуждение Генератора
	DO_PC_BRAKE_FAIL,			//Сигнал:
	DO_PC_OIL_FAN,              //Сигнал:
#else
	DI_PC_TEST_START,			//Сигнал: Запустить испытание
	DI_PC_TEST_STOP,			//Сигнал: Остановить испытание
	DI_PC_FIRE_ALARM,			//Сигнал: Возгорание
	DI_PC_HOT_TEST,				//Сигнал: Горячая обкатка
	DI_PC_SET_TIME,				//Сигнал: Обновить системное время
	DI_PC_BRAKE_TEST,			//Сигнал: Тест гидротормоза
	
	DO_PC_STARTER,				//Сигнал: Стратер
	DO_PC_COOLANT_FAN,			//Сигнал: Вентилятор ОЖ
	DO_PC_COOLANT_PUMP,			//Сигнал: Насос ОЖ
	DO_PC_OIL_PUMP,				//Сигнал: Насос масла
	DO_PC_COOLANT_HEATER,		//Сигнал: Нагреватель ОЖ
	DO_PC_OIL_HEATER,			//Сигнал: Нагреватель масла
    DO_PC_FUEL_PUMP,			//Сигнал: Включить ТНВД
	DO_PC_BRAKE_FAIL,			//Сигнал: авария гидротормоза
	DO_PC_OIL_FAN,              //Сигнал: Вентилятор масла
#endif
	
	AI_PC_TORQUE,			//Аналоговый: Заданая нагрузка (момент на валу)
	AI_PC_ROTATE,			//Аналоговый: Заданая скорость вращения
    AI_PC_DURATION,			//Аналоговый: Время испытания
	AI_PC_DATE,
	AI_PC_TIME,
	AI_PC_GA_TASK,		//Аналоговый: Регистры задания параметров газоанализатора

	AO_PC_TORQUE,			//Аналоговый: Момент на валу (нагрузка)
	AO_PC_ROTATE,			//Аналоговый: Скорость вращения
	AO_PC_T_EXHAUST,		//Аналоговый: Температура выхлопных газов
	AO_PC_T_COOLANT_IN,		//Аналоговый: Температура ОЖ на входе
	AO_PC_T_COOLANT_OUT,	//Аналоговый: Температура ОЖ на выходе
	AO_PC_T_OIL_IN,			//Аналоговый: Температура масла на входе
	AO_PC_T_OIL_OUT,		//Аналоговый: Температура масла на выходе
	AO_PC_T_FUEL,			//Аналоговый: Температура топлива
	AO_PC_FUEL_CONSUM,		//Аналоговый: расход топлива
	AO_PC_FUEL_LEVEL,		//Аналоговый: Датчик уровня топлива
#if !ECU_CONTROL // убрать в новой версии !!!
	AO_PC_T_EXT1,			//Аналоговый: Датчик темературы локального нагрева 1
	AO_PC_T_EXT2,			//Аналоговый: Датчик темературы локального нагрева 2
	AO_PC_T_EXT3,			//Аналоговый: Датчик темературы локального нагрева 3
	AO_PC_T_EXT4,			//Аналоговый: Датчик темературы локального нагрева 4
	AO_PC_T_EXT5,			//Аналоговый: Датчик темературы локального нагрева 5
	AO_PC_T_EXT6,			//Аналоговый: Датчик темературы локального нагрева 6
	AO_PC_T_EXT7,			//Аналоговый: Датчик темературы локального нагрева 7
	AO_PC_T_EXT8,			//Аналоговый: Датчик темературы локального нагрева 8
#else // добавить в новой версии !!!
	AO_PC_Q_BCU,			//Аналоговый: Расход воды на входе в гидравлический тормоз
#endif
	AO_PC_P_EXHAUST,		//Аналоговый: Давление выхлопных газов
	AO_PC_P_OIL,			//Аналоговый: Давление масла
	AO_PC_P_CHARGE,			//Аналоговый: Давление наддувочного воздуха
	AO_PC_T_CHARGE,			//Аналоговый: Температура наддувочного воздуха
	AO_PC_P_MANIFOLD,		//Аналоговый: Давление впускного коллектора
	AO_PC_T_ENV_AIR,		//Аналоговый: Температура окружающего воздуха
	AO_PC_P_ENV_AIR,		//Аналоговый: Атмосферное давление
	AO_PC_H_ENV_AIR,		//Аналоговый: Влажность воздуха	
	AO_PC_VOLTAGE,			//Аналоговый: напряжение АКБ
	AO_PC_CURRENT,			//Аналоговый: ток АКБ
	AO_PC_TROTTLE,			//Аналоговый: положение педали газа
	AO_PC_P_BRAKE,			//Аналоговый: давление в гидротормозе
	AO_PC_STATUS,			//Аналоговый: Состояние выполняемой операции
	AO_PC_ERR_MAIN,			//Аналоговый: Ошибки
	AO_PC_DATE,				//Аналоговый: Текущая дата: 0x00ГГММДД
	AO_PC_TIME,				//Аналоговый: текущее время: 0x00ЧЧММСС

	AO_PC_POWER,			//Аналоговый: Мощность на валу
	AO_PC_T_BRAKE,			//Аналоговый: Температура в гидротормозе
	AO_PC_SET_BRAKE,		//Аналоговый: Установленная мощность гидротормоза в м%
	//----------------------------
#ifdef ECU_CONTROL
	AO_PC_1MV8A1,			// Т input 1
	AO_PC_1MV8A2,			// Т input 2
	AO_PC_1MV8A3,			// Т input 3
	AO_PC_1MV8A4,			// Т input 4
	AO_PC_1MV8A5,			// Т input 5
	AO_PC_1MV8A6,			// Т input 6
	AO_PC_1MV8A7,			// Т input 7
	AO_PC_1MV8A8,			// Т input 8
	AO_PC_2MV8A1,			// Т input 9
	AO_PC_2MV8A2,			// Т input 10
	AO_PC_2MV8A3,			// Т input 11
	AO_PC_2MV8A4,			// Т input 12
	AO_PC_2MV8A5,			// Т input 13
	AO_PC_2MV8A6,			// Т input 14
	AO_PC_2MV8A7,			// Т input 15
	AO_PC_AGM_D01,			//Аналоговый: Регистр статуса
	AO_PC_AGM_D02,			//Аналоговый: Регистр ошибок
	AO_PC_AGM_D03,			//Аналоговый: Дата поверки
	AO_PC_AGM_D04,			//Аналоговый: Год поверки
	AO_PC_AGM_D05,			//Аналоговый: Наработка
	AO_PC_AGM_D06,			//Аналоговый: Ta, deg.C
	AO_PC_AGM_D07,			//Аналоговый: Тg 1-й канал, deg.C
	AO_PC_AGM_D08,			//Аналоговый: Тg 2-й канал, deg.C
	AO_PC_AGM_D09,			//Аналоговый: O2 1-й канал, % об.* 100
	AO_PC_AGM_D10,			//Аналоговый: O2 2-й канал, % об.* 100
	AO_PC_AGM_D11,			//Аналоговый: CO2 1-й канал, % об.* 100
	AO_PC_AGM_D12,			//Аналоговый: CO2 2-й канал, % об.* 100
	AO_PC_AGM_D13,			//Аналоговый: QA 1-й канал, % * 100
	AO_PC_AGM_D14,			//Аналоговый: QA 2-й канал, % * 100
	AO_PC_AGM_D15,			//Аналоговый: Alfa 1-й канал, * 1000
	AO_PC_AGM_D16,			//Аналоговый: Alfa 2-й канал, * 1000
	AO_PC_AGM_D17,			//Аналоговый: CO 1-й канал, ppm или mg/m3
	AO_PC_AGM_D18,			//Аналоговый: CO 2-й канал, ppm или mg/m3
	AO_PC_AGM_D19,			//Аналоговый: NO 1-й канал, ppm или mg/m3
	AO_PC_AGM_D20,			//Аналоговый: NO 2-й канал, ppm или mg/m3
	AO_PC_AGM_D21,			//Аналоговый: NO2 1-й канал, ppm или mg/m3
	AO_PC_AGM_D22,			//Аналоговый: NO2 2-й канал, ppm или mg/m3
	AO_PC_AGM_D23,			//Аналоговый: SO2 1-й канал, ppm или mg/m3
	AO_PC_AGM_D24,			//Аналоговый: SO2 2-й канал, ppm или mg/m3
	AO_PC_AGM_D25,			//Аналоговый: CH 1-й канал, ppm или mg/m3
	AO_PC_AGM_D26,			//Аналоговый: CH 2-й канал, ppm или mg/m3
	AO_PC_SMG_D01,			//Аналоговый: СМОГ 2-01 Дымность, N0,43
	AO_PC_SMG_D02,			//Аналоговый: СМОГ 2-01 Дымность, NН
	AO_PC_SMG_D03,			//Аналоговый: СМОГ 2-01 Коэффициент поглощения
	AO_PC_SMG_D04,			//Аналоговый: СМОГ 2-01 Температура
	AO_PC_SMG_D05,			//Аналоговый: СМОГ 2-01 Ошибки БОИ
	AO_PC_SMG_D06,			//Аналоговый: СМОГ 2-01 Статус БОИ
	AO_PC_ECU_01,			//65262/175: Температура масла
	AO_PC_ECU_02,			//65263/100: Давление масла
	AO_PC_ECU_03,			//65263/94: Давление топлива
	AO_PC_ECU_04,			//61450/132: Расход наддувочного воздуха
	AO_PC_ECU_05,			//65266/183: Расход топлива л/ч
	AO_PC_ECU_06,			//65270/106: Давление наддувочного воздуха
	AO_PC_ECU_07,			//65270/105: Температура воздуха после охладителя наддувочного воздуха
	AO_PC_ECU_08,			//расчетный: Удельный эффективный расход топлива (г/кВт*ч)
	AO_PC_ECU_09,			//65253/247: Engine Total Hours of Operation
	AO_PC_ECU_10,			//65262/110: Engine Coolant Temperature
#endif
	//----------------------------
	SIG_END,
};
#define SAVE_PID_VAL		DI_PC_SET_TIME // Сигнал: применить коэффициенты ПИД-регуляторов
#define AI_PC_SPEED_KP_KI	AI_PC_DATE
#define AI_PC_TORQUE_KP_KI	AI_PC_TIME
#define AO_PC_SPEED_KP_KI	AO_PC_DATE
#define AO_PC_TORQUE_KP_KI	AO_PC_TIME
#define AI_PC_SPEED_KI		(st(AI_PC_SPEED_KP_KI) & 0xFFFF)
#define AI_PC_SPEED_KP		(st(AI_PC_SPEED_KP_KI) >> 16)
#define AI_PC_TORQUE_KI		(st(AI_PC_TORQUE_KP_KI) & 0xFFFF)
#define AI_PC_TORQUE_KP		(st(AI_PC_TORQUE_KP_KI) >> 16)
#define ENGINE_KEY_TASK		DI_PC_FIRE_ALARM // Сигнал: Включить зажигание
#define ENGINE_ON_LED		DO_PC_STARTER // Сигнал: Включено зажигание
#define ECU_ERROR_LED		DO_PC_OIL_HEATER // Сигнал: Лампа "АВАРИЯ" ЭБУ
#define GEN_EXC_LED			DO_PC_FUEL_PUMP // Сигнал: Возбуждение Генератора
#define ENGINE_RELAY		DO_STARTER // Сигнал: Включено зажигание
#define GEN_EXC_RELAY		DO_FUEL_PUMP // Сигнал: Возбуждение Генератора
#define START_RELAY			DO_COOLANT_HEATER // Сигнал: Запустить Двигатель

//--------------------------состояние сигналов------------------------------------------------------------
typedef struct
	{
#ifdef OCU
	struct
		{
		struct
			{
			uint16_t d;
			}o;
		struct
			{
			uint8_t d;
			int32_t a;
			}i;
		}ocu;
#endif
#ifdef PM
	struct
		{
		struct
			{
			uint16_t d;
			int32_t a;
			}i;
		}pm;
#endif
#ifdef OCU_24V
	struct
		{
		struct
			{
			uint16_t d;
			}o;
		struct
			{
			uint8_t d;
			int32_t a[1];
			}i;
		}ocu;
#endif
#ifdef ETCU
	struct
		{
		struct
			{
			uint8_t d;
			}o;
		struct
			{
			int32_t a[29];
			}i;
		}etcu;
#endif
#ifdef BCU
	struct
		{
		struct
			{
			uint8_t d;
			int32_t a[3];
			}o;
		struct
			{
			uint8_t d;
			int32_t a[8];
			}i;
		}bcu;
#endif
#ifdef TA
	struct
		{
		struct
			{
			int32_t a[1];
			}o;
		struct
			{
			int32_t a[3];
			}i;
		}ta;
#endif
#ifdef FC
	struct
		{
		struct
			{
			int32_t a[1];
			}o;
		struct
			{
			int32_t a[1];
			}i;
		}fc;
#endif
#ifdef PC
	struct
		{
		struct
			{
			uint16_t d;
			int32_t a[SIG_END-AO_PC_TORQUE];
			}o;
		struct
			{
			uint8_t d;
			int32_t a[5];
			}i;
		struct
			{
			int32_t a[DO_STARTER];
			}cfg;
		}pc;
#endif
#ifdef INCL
	struct
		{
		struct
			{
			int32_t a[2];
			}i;
		}incl;
#endif
#ifdef PPU
	struct
		{
		struct
			{
			uint64_t d;
			}o;
		struct
			{
			uint16_t d;
			}i;
		}ppu;
#endif
#ifdef MP
	struct
		{
		struct
			{
			uint32_t a[4];
			}i;
		}mp;
#endif
#ifdef CDU
	struct
		{
		struct
			{
			uint32_t a[4];
			}i;
		}cdu;
#endif
#ifdef APS
	struct
		{
		struct
			{
			uint32_t a[3];
			}i;
		}aps;
#endif
#ifdef SSCU
	struct
		{
		struct
			{
			uint32_t d;
			}o;
		struct
			{
			uint16_t d;
			uint32_t a[2];
			}i;
		}sscu;
#endif
#ifdef PSU
	struct
		{
		struct
			{
			uint8_t d;
			}o;
		struct
			{
			uint32_t a[3];
			}i;
		}psu;
#endif
#ifdef HCU
	struct
		{
		struct
			{
			uint8_t d;
			uint32_t a[1];
			}o;
		struct
			{
			uint8_t d;
			uint32_t a[1];
			}i;
		}hcu;
#endif
	}sg_t;
#pragma pack()
//----------------значения дискретных сигналов------------------------------------------------
	#define ON 1
	#define OFF 0
	#define INVERT 2

//---------------значения аналоговых сигналов-------------------------------------------------
	#ifdef PC //значения сигнала "AO_PC_STATUS"
	typedef enum
		{
		ST_STOP=0,	 //нет последней задачи
		ST_STOP_ERR=1, //остановка изза ошибки
		ST_STRAT_FC=2,	//Запуск электрического мотора
		ST_STARTER_ON=3, //включение стартера
		ST_WAIT_ENGINE_START=4,	//ожидание запуска двигателя
		ST_COLD=5,				//холодная обкатка, контроль датчиков локального нагрева
		ST_BREAK_TEST=6,		//запуск гидротормоза для теста
		ST_FUEL_PUMP=7,			//подача топлива
		ST_SET_ROTATION=8,		//выход двигателя на заданные обороты
		ST_SET_TORQUE=9,		//Установка заданного крутящего момента
		ST_HOT=10,				//Горячая обкатка
		ST_STOP_TIME=11,		//Остановка: истекло заданное время
		}ao_pc_status_t;
	#endif

	typedef union { //структура сигнала "AO_PC_ERR_MAIN"
		struct {
			uint8_t no_bcu :1; //нет связи с модулем управления гидротормозом
			uint8_t no_fc :1; //нет связи с преобразователем часоты
			uint8_t no_ta :1; //нет связи с сервоприводом педали газа
			uint8_t no_cdu :1; //нет свзяи с ЗРМ
			//uint8_t no_ppu :1; //нет связи с ППУ
			uint8_t no_trq :1; //нет связи с датчиком момента
			uint8_t err_fc :1; //ошибка в работе преобразователя частоты
			uint8_t err_akb:1; //низкое напряжение АКБ, запуск стартера невозможен
			uint8_t err_starter:1; //стартер не включился
			uint8_t err_brake:1; //ощибка гидротормогза
			uint8_t err_fc_speed:1; //Электропривод не может достичь заданных оборотов
			uint8_t engine_start:1; //ошибка запуска двигателя: двигатель не запускается
			uint8_t engine_rotate:1; //двигатель не выходит на заданные обороты
			uint8_t emergancy_stop:1; //нажать "Аварийный стоп"
			uint8_t servo_not_init:1; //калибровка сервопривода
			uint8_t servo_error:1; //ошибка сервопривода
		} bit;
		uint8_t byte[4];
		uint16_t word[2];
		uint32_t dword;
	} error_t;

	#ifdef CDU //значения сигнала "AI_CU_ERR",  "AI_MP_ERR", "AO_PC_ERR_MP", "AO_PC_ERR_CU"
	#define	ERR_OVERLOAD		1 //перегрузка тока или напряжения при заряде
	#define	ERR_DISCH_PWR		2 //перегрузка тока или няпряжения при разряде
	#define ERR_CONNECTION 		3 //неверное подключение АКБ
	#define ERR_NO_AKB			4 //АКБ не подключен
	#define ERR_OVERTEMP1		5 //перегрев входного каскада
	#define ERR_OVERTEMP2		6 //перегрев выходного каскада
	#define ERR_OVERTEMP3		7 //перегрев внешнего радиатора
	#define ERR_SET				8 //
	#define ERR_ADC				9 //Неисправность АЦП
	#define ERR_STAGE			10 //Неверные параметры этапа
	#define ERR_CDU				11 //нет связи с ЗРМ
	#define ERR_OUT				12 //КЗ выпрмителя ЗРМ
	#define ERR_CONNECTION1		13 //неверное подключение АКБ
	#define ERR_DM_LOSS			14 //обрыв разярдного модуля
		//значения сигнала "AI_CU_ST", "AI_MP_ST"
	#define	CU_STOP				0 //ЗРМ остановлен
	#define	CU_CHARGE			1 //ЗРМ заряд
	#define	CU_DISCHARGE		2 //ЗРМ разряд
	#endif

	typedef enum
		{
		SERVO_OK, //0:Исправен;
		SERVO_STOP_CMD, // 1:Останов по команде
		SERVO_HI_RESIST, //2:Останов - высокое противодействующее усилие
		SERVO_SHORT_CIRCUIT,//3:Останов - короткое замыкание
		SERVO_OVERTAMP,//4:Выход из температурного диапазона
		SERVO_HIGH_U_POWER,//5:Повышенное силовое напряжение
		SERVO_LOW_U_POWER,//6:Низкое силовое напряжение
		SERVO_HIGH_U_DIGITAL,//7:Повышенное цифровое напряжение
		SERVO_LOW_U_DIGITAL,//8:Низкое цифровое напряжение
		SERVO_HIGH_U_USER,//9:Cиловое напряжение превышает напряжение, установл. пользователем;
		SERVO_OVERLOAD,//10:Останов - превышение допустимой перегрузки
		SERVO_IGBT_ERR,//11:Останов - сработала защита IGBT модуля;
		SERVO_UVW_ERR,//12:Останов - Ошибка поиска фазы по UVW трекам;
		SERVO_CFG_ERR,//13:Останов - ошибка в настройках энкодера;
		SERVO_ENCODER_ERR,//14: Останов - сбой в работе энкодера;
		SERVO_LINK_ERR	//15: Нет связи с сервоприводом
		}ai_servo_st;

//===============функции работы с сигналами==============================================================
	//------------------------------------------------------------------------------
	/// @brief  Запись сигнала
	///
	/// Устанавливает заданный сигнал в заданное значение
	///
	/// @param sg - номер (имя) сигнала в таблице описаний сигналов
	/// @param st - значение сигнала
	//------------------------------------------------------------------------------
		void set(uint16_t sg, int32_t st); //установить сигнал
	//------------------------------------------------------------------------------
	/// @brief  Чтение состояния сигнала
	///
	/// Возвращает текущее значение сигнала
	///
	/// @param  sg - номер (имя) сигнала в таблице описаний сигналов
	/// @return - значение сигнала
	//------------------------------------------------------------------------------
		int32_t st(uint16_t sg);		//прочитать значение сигнала
//=======================================================================================================
#endif /* SIGNALS_H_ */
