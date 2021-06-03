#ifndef APP__CONTROL_H_
#define APP__CONTROL_H_
#include <stdbool.h>
#include "_signals.h"

typedef enum {
	TSC1Control	 = 0,
	EaccControl	 = 1,
	ServoControl = 2
} SpeedCntrl_t;

extern bool pid_init;
#define PID_REINIT()				pid_init = false
#define ERROR_CODE					0x7FFFFFFF
#define DEF_KEY_DELAY				500		//Антидребезг на кнопки
#define DEF_MAX_I_SERVO				5000	//максимальный ток сревопривода, мА
#define DEF_MAX_SERVO_POSITION		5000000	//максимальное положение сревопривода,
#define DEF_MAX_ENGINE_SPEED		(735 * 1000) //максимальные обороты электромотора
#define DEF_U_AKB_NORMAL			20000	//Минимальное занпряжение АКБ для запуска двигателя
#define DEF_STARTER_ON_TIME			10000	//время включения стартера, если двигатель не запускается
#define DEF_MIN_ROTATE				400000	//Минимальные обороты двигателя для определения его запуска
#define DEF_ERR_ROTATE				200000	//Минимальные обороты двигателя для определения ошибки
#define DEF_MIN_I_STARTER			10000	//Минимальный ток стартера
#define DEF_FUEL_PUMP_TIMEOUT		2000	//Время работы ТНВД перед запуском стартера
#define DEF_TIME_START_FC			5000	//Таймаут на запуск ПЧ
#define DEF_HYDROSTATION_TETS		10000	//Процент нажатия торможа при проверки гидростанции
#define DEF_SPEED_TIME_HYDRO		10000	//Время выхода на полную мощность гидростанции
#define DEF_TIMOUT_ENGINE_ROTATE	20000 	//Времяза которое должен выйти на заданные обороты двигатель
#define DEF_TIMOUT_SET_TORQUE		20000 	//Время установки заданного крутящего моента

#define OPR_START_TEST		1
#define OPR_KEY_ON			11

typedef struct {
	uint8_t opr;
} cmd_t;

typedef struct {
	uint8_t mode;
	uint8_t opr;
	uint32_t step;
} state_t;

typedef struct {
	stime_t key_delay;
	stime_t alg;
	stime_t all;
	stime_t hydro;
} timeout_t;

/* адреса датчиков в системе */
#define NODE_ID_BCU 1	//Адрес CanOpen модуля управления гидротормозом
#define NODE_ID_FC	2	//Адрес CanOpen/ModBus преобразователя частоты
#define SPSH20_ADR	1	//Адрес сервопривода
#define ADR_NL_3DPAS 1	//Адрес ModBus датчика параметров атмосферы NL-3DPAS
#define ADR_T46 	1	//Адрес ModBus датчика крутящего момента
#define ADR_CDU		1	//Адрес CDUlink зарядно-разрядного устйроства
#define ADR_MU6U 	2	//Адрес ModBus эмулятора педали газа

/* расположение аналоговых сигналов ETCU */
#define ETCU_AI_FUEL1		0
#define ETCU_AI_FUEL2 		1
#define ETCU_AI_ROTATE		2
#define ETCU_AI_P_EXHAUST	3
#define ETCU_AI_I_P			4
#define ETCU_AI_I_N			5
#define ETCU_AI_U			6
#define ETCU_AI_FUEL_LEVEL	7
#define ETCU_AI_T1			8
#define ETCU_AI_T2			9
#define ETCU_AI_T3			10
#define ETCU_AI_T4			11
#define ETCU_AI_T5			12
#define ETCU_AI_P_OIL		13
#define ETCU_AI_T_EXHAUST 	14
#define ETCU_AI_P_BOOST 	15
#define ETCU_AI_T_BOOST 	16
#define ETCU_AI_TIME		17
#define ETCU_AI_DATE		18
#define ETCU_AI_VBAT		19
#define ETCU_AI_5V			20
#define ETCU_AI_TEMP1		21
#define ETCU_AI_TEMP2		22
#define ETCU_AI_TEMP3		23
#define ETCU_AI_TEMP4		24
#define ETCU_AI_TEMP5		25
#define ETCU_AI_TEMP6		26
#define ETCU_AI_TEMP7		27
#define ETCU_AI_TEMP8		28

#ifndef NO_SAFE
#define SAFE	safe < SAFE_MAX
#define SFREQ	sreq < SFREQ_MAX
#else
#define SAFE	1
#define SFREQ	1
#endif

#ifdef MODEL_OBJ
	#define SPEED_KP_EA			03.00f
	#define SPEED_TI_EA			10.00f
#define SPEED_KP_SP			0.01f
#define SPEED_TI_SP			200.00f
#define SPEED_TD			50.50f
	//#define SPEED_KP_SP			0.0012f
	//#define SPEED_TI_SP			100.00f
	//#define SPEED_TD			10.00f
	#define SPEED_DF_TAU		30.0f
	#define TORQUE_KP			00.20f
	#define TORQUE_TI			10.00f
	#define TORQUE_TD			0.00f
	#define TORQUE_DF_TAU		100.0f
#else
	#define SPEED_KP_EA			00.30f
	#define SPEED_TI_EA			100.00f
	#define SPEED_KP_SP			10.00f
	#define SPEED_TI_SP			280.00f
	#define SPEED_TD			35.00f
	#define SPEED_DF_TAU		00.01f
	#define TORQUE_KP			00.20f
	#define TORQUE_TI			10.00f
	#define TORQUE_TD			1.00f
	#define TORQUE_DF_TAU		0.0050f
#endif

#if UNI_CONTROL
	#define ZONE_DEAD_EACC		05.0f
	#define ZONE_DEAD_LA10P		30.0f
	#define SPEED_MUL_EACC		0.08f
	#define SPEED_MUL_LA10P		0.10f
#elif SERVO_CONTROL
	#define ACCEL_SET			servo_set_out
	#define ACCEL_STATE			servo_get_pos()
	#define ZONE_DEAD_REF		80.0f
	#define SPEED_MUL			1.20f
#elif SPSH_CONTROL
	#define ACCEL_SET			spsh20_set_pos
	#define ACCEL_STATE			(float32_t)(spsh20_get_pos() / 1000)
	#define ZONE_DEAD_REF		20.0f
	#define SPEED_MUL			-40.00f
#else
	#error "Accelerator driver not defined"
#endif

#define XI_DIV				3.0f
//#define SPEED_LOOP_TIME		100 // дискретизация по времени контура регулирования оборотов, мс
#define TORQUE_MAX			400.0f // Нм
#define TORQUE_FACTOR		0.2f
#define SPEED_MAX			3000.0f
#if UNI_CONTROL
#define SPEED_FACT_EACC		0.04f
#define SPEED_FACT_LA10P	45.0f
#else
#define SPEED_FACT			45.0f
#endif

#define TORQUE_LOOP_TIME	200 // дискретизация по времени контура крутящего момента, мс
#define SPEED_DIFF			50
#define PWM_SCALE			100000U
#define PWM_FSCALE			100000.0f
#define PWM_PERCENT			(PWM_SCALE / 100000)
#define SCALE_DIV			4U // разбивка диапазона нагнетания между насосом и клапаном 1:4
#define VALVE_DIV			8U
#define VALVE_NULL			(PWM_SCALE / VALVE_DIV) // начальный угол закрытия клапана 20...30%
#define PERCENT_FACTOR		0.008f
#define TFILTER_TAU			0.2f
#define VALVE_MIN			0.10f
#define PWM_V_MIN			(VALVE_MIN * PWM_FSCALE)
#define VALVE_MAX			0.80f
#define PWM_V_MAX			(VALVE_MAX * PWM_FSCALE)
#define PUMP_MIN			0.20f
#define PWM_P_MIN			(PUMP_MIN * PWM_FSCALE)
#define PUMP_MAX			0.80f
#define PWM_P_MAX			(PUMP_MAX * PWM_FSCALE)

#ifdef MODEL_OBJ
	#define TORQUE_SCALE	153.846f
#else
	#define TORQUE_SCALE	153.846f
#endif // MODEL_OBJ

void control_init (void); //инициализация системы управления
void control_step (void); //1 шаг алгоритма управления
void signals_start_cfg (void);
void parametrs_start_cfg (void);
void sensors_config (void);
void read_devices (void);
void update_devices (void);
void read_keys (void);		//опросить значение кнопок
void set_indication (void);	//обновить состояние индикаторов
void work_step (void);		//выполнить шаг алгоритма управления

uint8_t chek_out_val(int32_t val1, int32_t val2, int32_t delta);
void work_stop(void);

#ifdef MODEL_OBJ
typedef struct {
	uint32_t st; // начало периода дискретизации
	float32_t out; // предыдущее значение выходного параметра звена 1
	float32_t tau; // постоянная времени апериодического звена 1, ms
} obj_t;
#endif

typedef enum {
	Percent = 0,
	Absolute
} torq_val_t;

#endif /* APP__CONTROL_H_ */

