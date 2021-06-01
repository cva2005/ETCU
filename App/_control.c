#include <string.h>
#include <stdlib.h>
#include "timers.h"
#include "ecu.h"
#include "_control.h"
#include "dio.h"
#include "flash.h"
#include "crc.h"
#include "adc.h"
#include "pc_device.h"
#include "bcu.h"
#include "atv61.h"
#include "spsh20.h"
#include "nl_3dpas.h"
#include "t46.h"
#include "ds18b20.h"
#include "power.h"
#include "rtc_sens.h"
#include "pulse_sens.h"
#include "u_sens.h"
#include "i_sens.h"
#include "t_auto.h"
#include "p_mm370.h"
#include "p_745_3829.h"
#include "mu110_6U.h"
#include "servo.h"
#include "la10pwm.h"
#include "pid_r.h"

extern sig_cfg_t sig_cfg[SIG_END];  //описание сигналов
extern sg_t sg_st;					//состояние сигналов

static state_t state;
static cmd_t cmd;
static timeout_t time;
static error_t error;
static stime_t cntrl_M_time; // таймер дискретизации регулятора момента
pid_r_instance Speed_PID;
pid_r_instance Torque_PID;
uint16_t safe = 0, sfreq = 0;
void init_PID (void);
void Speed_loop (void);
void Torque_loop (torq_val_t val);
#ifdef MODEL_OBJ
uint32_t ServoPos;
obj_t FrequeObj;
obj_t TorqueObj;
float32_t get_obj (obj_t * obj, float32_t inp);
void init_obj (void);
#endif
uint32_t Pwm1_Out = 0, Pwm2_Out = 0;
#define PID_RESET				1
#define PID_NO_RESET			0
bool CntrlDevSel = false;
static SpeedCntrl_t SpeedCntrl = TSC1Control;
static bool Ready;
bool pid_init = false;
float32_t SpeedKp, SpeedTi;
float32_t TorqueKp, TorqueTi;
static unsigned StopCount = 0;
static bool tune_start = false;

void signals_start_cfg (void) {
	uint16_t cnt, nmb;

	memset((uint8_t*) (&sig_cfg), 0, sizeof(sig_cfg));
//настройка сигналов конфигурации--------------------------
	for (cnt=0; cnt<DO_STARTER; cnt++) {
		sig_cfg[cnt].fld.deivice=PC;
		sig_cfg[cnt].fld.activ=1;
		sig_cfg[cnt].fld.type=CFG;
		sig_cfg[cnt].fld.number=cnt;
	}
//настройка сигналов ETCU----------------------------------
	for (cnt=DO_STARTER; cnt<AI_T_EXHAUST; cnt++) {
		sig_cfg[cnt].fld.deivice=ETCU;
		sig_cfg[cnt].fld.activ=1;
		sig_cfg[cnt].fld.type=DO;
	}
	for (cnt=AI_T_EXHAUST; cnt<AI_T_AIR; cnt++) {
		sig_cfg[cnt].fld.deivice=ETCU;
		sig_cfg[cnt].fld.activ=1;
		sig_cfg[cnt].fld.type=AI;
	}
	sig_cfg[AI_T_EXHAUST].fld.number=ETCU_AI_T_EXHAUST;	//Аналоговый: Температура выхлопных газов
	sig_cfg[AI_T_COOLANT_IN].fld.number=ETCU_AI_T1;		//Аналоговый: Температура ОЖ на входе
	sig_cfg[AI_T_COOLANT_OUT].fld.number=ETCU_AI_T2;	//Аналоговый: Температура ОЖ на выходе
	sig_cfg[AI_T_OIL_IN].fld.number=ETCU_AI_T3;			//Аналоговый: Температура масла на входе
	sig_cfg[AI_T_OIL_OUT].fld.number=ETCU_AI_T4;		//Аналоговый: Температура масла на выходе
	sig_cfg[AI_T_FUEL].fld.number=ETCU_AI_T5;			//Аналоговый: Температура топлива
	sig_cfg[AP_FUEL_IN].fld.number=ETCU_AI_FUEL1;		//Импульсный: подаваемое топливо
	sig_cfg[AP_FUEL_OUT].fld.number=ETCU_AI_FUEL2;		//Импульсный: возвращаемое топливо
	sig_cfg[AI_FUEL_LEVEL].fld.number=ETCU_AI_FUEL_LEVEL;//Аналоговый: Датчик уровня топлива
	sig_cfg[AI_T_EXT1].fld.number=ETCU_AI_TEMP1;		//Аналоговый: Датчик темературы локального нагрева 1
	sig_cfg[AI_T_EXT2].fld.number=ETCU_AI_TEMP2;		//Аналоговый: Датчик темературы локального нагрева 2
	sig_cfg[AI_T_EXT3].fld.number=ETCU_AI_TEMP3;		//Аналоговый: Датчик темературы локального нагрева 3
	sig_cfg[AI_T_EXT4].fld.number=ETCU_AI_TEMP4;		//Аналоговый: Датчик темературы локального нагрева 4
	sig_cfg[AI_T_EXT5].fld.number=ETCU_AI_TEMP5;		//Аналоговый: Датчик темературы локального нагрева 5
	sig_cfg[AI_T_EXT6].fld.number=ETCU_AI_TEMP6;		//Аналоговый: Датчик темературы локального нагрева 6
	sig_cfg[AI_T_EXT7].fld.number=ETCU_AI_TEMP7;		//Аналоговый: Датчик темературы локального нагрева 7
	sig_cfg[AI_T_EXT8].fld.number=ETCU_AI_TEMP8;		//Аналоговый: Датчик темературы локального нагрева 8
	sig_cfg[AI_P_EXHAUST].fld.number=ETCU_AI_P_EXHAUST;	//Аналоговый: Давление выхлопных газов
	sig_cfg[AI_P_OIL].fld.number=ETCU_AI_P_OIL;			//Аналоговый: Давление масла
	sig_cfg[AI_P_CHARGE].fld.number=ETCU_AI_P_BOOST;	//Аналоговый: Давление наддувочного воздуха
	sig_cfg[AI_T_CHARGE].fld.number=ETCU_AI_T_BOOST;	//Аналоговый: Температура наддувочного воздуха
	sig_cfg[AI_P_MANIFOLD].fld.number=ETCU_AI_FUEL_LEVEL;//Аналоговый: Давление впускного коллектора
	sig_cfg[AI_VBAT].fld.number=ETCU_AI_VBAT;			//Аналоговый: Напряжение батарейки
	sig_cfg[AI_5V].fld.number=ETCU_AI_5V;				//Аналоговый: Напряжение питания 5В
	sig_cfg[AI_TIME].fld.number=ETCU_AI_TIME;			//Аналоговый: Время
	sig_cfg[AI_DATE].fld.number=ETCU_AI_DATE;			//Аналоговый: Дата
	sig_cfg[AI_ROTATION_ETCU].fld.number=ETCU_AI_ROTATE;//Импульсный: количество оборотов в минуту
	sig_cfg[AI_I_AKB_P].fld.number=ETCU_AI_I_P;			//Аналоговый: Ток АКБ втекающий
	sig_cfg[AI_I_AKB_N].fld.number=ETCU_AI_I_N;			//Аналоговый: Ток АКБ вытекающий
	sig_cfg[AI_U_AKB].fld.number=ETCU_AI_U;				//Аналоговый: Напряжение АКБ
	sig_cfg[DO_STARTER].fld.number=0;					//Сигнал: Стратер
	sig_cfg[DO_COOLANT_FAN].fld.number=1;				//Сигнал: Вентилятор ОЖ
	sig_cfg[DO_COOLANT_PUMP].fld.number=2;				//Сигнал: Насос ОЖ
	//sig_cfg[DO_OIL_PUMP].fld.number=3;					//Сигнал: Насос масла
	sig_cfg[DO_OIL_PUMP].fld.number=1;					//Сигнал: Насос масла (перемещено в BCU)
#if BAD_HARDWARE
	sig_cfg[DO_FUEL_PUMP].fld.number=3;					//Сигнал: Включить ТНВД
	sig_cfg[DO_COOLANT_HEATER].fld.number=6;			//Сигнал: Нагреватель ОЖ
	sig_cfg[DO_OIL_HEATER].fld.number=7;				//Сигнал: Нагреватель масла
#else
	sig_cfg[DO_COOLANT_HEATER].fld.number=4;			//Сигнал: Нагреватель ОЖ
	sig_cfg[DO_OIL_HEATER].fld.number=5;				//Сигнал: Нагреватель масла
	sig_cfg[DO_FUEL_PUMP].fld.number=6;					//Сигнал: Включить ТНВД
#endif
//настройка сигналов APS---------------------------------
	for (cnt=AI_T_AIR; cnt<DO_EMERGANCY; cnt++) {
		sig_cfg[cnt].fld.deivice=APS;
		sig_cfg[cnt].fld.activ=1;
		sig_cfg[cnt].fld.type=AI;
		sig_cfg[cnt].fld.number=cnt-AI_T_AIR;
	}
//настройка сигналов BCU---------------------------------
	for (cnt=DO_EMERGANCY; cnt<AI_P_OIL_BRAKE; cnt++) {
		sig_cfg[cnt].fld.deivice=BCU;
		sig_cfg[cnt].fld.activ=1;
		sig_cfg[cnt].fld.type=DO;
	}
	sig_cfg[DO_OIL_PUMP].fld.deivice=/*ETCU*/BCU;
	for (cnt=AI_P_OIL_BRAKE; cnt<AO_FC_FREQ; cnt++) {
		sig_cfg[cnt].fld.deivice=BCU;
		sig_cfg[cnt].fld.activ=1;
		if (cnt<AO_HYDROSTATION) sig_cfg[cnt].fld.type=AI;
		else			  		 sig_cfg[cnt].fld.type=AO;
	}
	sig_cfg[DO_EMERGANCY].fld.number=2;			//Сигнал: Авария
	sig_cfg[DO_SIREN].fld.number=3;				//Сигнал: Сирена
	sig_cfg[DO_OIL_PUMP].fld.number=1;			//Сигнал: Насос масла (перемещено из ETCU)
	sig_cfg[DO_OIL_FAN].fld.number=0;			//Сигнал: Вентилятор охлаждения масла
	sig_cfg[AI_T_OIL_BRAKE].fld.number=0;		//Аналоговый: Температура масла в гидротормозе
	sig_cfg[AI_P_OIL_BRAKE].fld.number=1;		//Аналоговый: Давление масла в гидротормозе
	sig_cfg[AI_VALVE_POSITION].fld.number=2;		//Позиция клапана
	sig_cfg[AI_TORQUE].fld.number=3;				//Аналоговый: Момент вращения
	sig_cfg[AI_ROTATION_SPEED].fld.number=4;		//Аналоговый: Скорость вращения
	sig_cfg[AI_POWER].fld.number=5;				//Аналоговый: Мощность
	sig_cfg[AO_HYDROSTATION].fld.number=0;		//Производительность гидростанции
	sig_cfg[AO_VALVE_ENABLE].fld.number=1;		//Сигнал вкл/выкл клапана
	sig_cfg[AO_VALVE_POSITION].fld.number=2;	//Задаваемая позиция клапана
//настройка сигналов FC------------------------------------
	sig_cfg[AO_FC_FREQ].fld.deivice=FC;
	sig_cfg[AO_FC_FREQ].fld.activ=1;
	sig_cfg[AO_FC_FREQ].fld.type=AO;
	sig_cfg[AO_FC_FREQ].fld.number=0;
	sig_cfg[AI_FC_FREQ].fld.deivice=FC;
	sig_cfg[AI_FC_FREQ].fld.activ=1;
	sig_cfg[AI_FC_FREQ].fld.type=AI;
	sig_cfg[AI_FC_FREQ].fld.number=0;
//настройка сигналов TA------------------------------------
	sig_cfg[AI_SERVO_POSITION].fld.deivice=TA;
	sig_cfg[AI_SERVO_POSITION].fld.activ=1;
	sig_cfg[AI_SERVO_POSITION].fld.type=AI;
	sig_cfg[AI_SERVO_POSITION].fld.number=0;
	sig_cfg[AI_SERVO_ST].fld.deivice=TA;
	sig_cfg[AI_SERVO_ST].fld.activ=1;
	sig_cfg[AI_SERVO_ST].fld.type=AI;
	sig_cfg[AI_SERVO_ST].fld.number=1;
	sig_cfg[AI_SERVO_I].fld.deivice=TA;
	sig_cfg[AI_SERVO_I].fld.activ=1;
	sig_cfg[AI_SERVO_I].fld.type=AI;
	sig_cfg[AI_SERVO_I].fld.number=2;

	sig_cfg[AO_SERVO_POSITION].fld.deivice=TA;
	sig_cfg[AO_SERVO_POSITION].fld.activ=1;
	sig_cfg[AO_SERVO_POSITION].fld.type=AO;
	sig_cfg[AO_SERVO_POSITION].fld.number=0;
//настройка сигналов зарядного устйроства-------------------------------
	for (cnt=AI_CDU_U; cnt<DI_PC_TEST_START; cnt++) {
		sig_cfg[cnt].fld.deivice=CDU;
		sig_cfg[cnt].fld.activ=1;
		sig_cfg[cnt].fld.type=AI;
	}
	sig_cfg[AI_CDU_ERR].fld.number=0;
	sig_cfg[AI_CDU_ST].fld.number=1;
	sig_cfg[AI_CDU_I].fld.number=2;
	sig_cfg[AI_CDU_U].fld.number=3;
//настройка сигналов планшета-------------------------------
	nmb = 0;
	for (cnt=DI_PC_TEST_START; cnt<SIG_END; cnt++) {
		sig_cfg[cnt].fld.deivice=PC;
		sig_cfg[cnt].fld.activ=1;
		sig_cfg[cnt].fld.type=DI;
		if (cnt >= DO_PC_STARTER) {
			if (cnt==DO_PC_STARTER) nmb=0;
			sig_cfg[cnt].fld.type=DO;
		}
		if (cnt>=AI_PC_TORQUE) {
			if (cnt==AI_PC_TORQUE) nmb=0;
			sig_cfg[cnt].fld.type=AI;
		}
		if (cnt>=AO_PC_TORQUE) {
			if (cnt==AO_PC_TORQUE) nmb=0;
			sig_cfg[cnt].fld.type=AO;
		}
		sig_cfg[cnt].fld.number=nmb;
		nmb++;
	}
}
//--------------------------------------------------------------------------------------------
void parametrs_start_cfg (void) {
	set(CFG_KEY_DELAY, DEF_KEY_DELAY);			//Антидребезг на кнопки
	set(CFG_PULSE1, PULSE_SENS_PL);
	set(CFG_PULSE2, PULSE_SENS_PL);
	set(CFG_PULSE3, PULSE_SENS_PL);
	set(CFG_SERVO_MAX_I, DEF_MAX_I_SERVO);
	set(CFG_MAX_ENGINE_SPEED, DEF_MAX_ENGINE_SPEED);
	set(CFG_MAX_SERVO_POSITION, DEF_MAX_SERVO_POSITION);	//максимальное положение сревопривода,
	set(CFG_U_AKB_NORMAL,DEF_U_AKB_NORMAL);	//Минимальное занпряжение АКБ для запуска двигателя
	set(CFG_STARTER_ON_TIME,DEF_STARTER_ON_TIME);	//время включения стартера, если двигатель не запускается
	set(CFG_MIN_ROTATE,DEF_MIN_ROTATE);	//Минимальные обороты двигателя для определения его запуска
	set(CFG_MIN_I_STARTER,DEF_MIN_I_STARTER);	//Минимальный ток стартера
	set(CFG_FUEL_PUMP_TIMEOUT,DEF_FUEL_PUMP_TIMEOUT);	//Время работы ТНВД перед запуском стартера
	set(CFG_TIME_START_FC,DEF_TIME_START_FC);	//Таймаут на запуск ПЧ
	set(CFG_HYDROSTATION_TETS,DEF_HYDROSTATION_TETS);	//Процент нажатия торможа при проверки гидростанции
	set(CFG_SPEED_TIME_HYDRO,DEF_SPEED_TIME_HYDRO);	//Время выхода на полную мощность гидростанции
	set(CFG_TIMOUT_ENGINE_ROTATE,DEF_TIMOUT_ENGINE_ROTATE); //Времяза которое должен выйти на заданные обороты двигатель
	set(CFG_TIMOUT_SET_TORQUE,DEF_TIMOUT_SET_TORQUE); //Время установки заданного крутящего моента
}
//--------------------------------------------------------------------------------------------
void sensors_config (void) {
	pulse_sens_set_pl(1, st(CFG_PULSE1));
	pulse_sens_set_pl(2, st(CFG_PULSE1));
	pulse_sens_set_pl(3, st(CFG_PULSE1));
}
//----------------------------------------------------------------------------------------------
void control_init(void) {
	uint32_t crcj;

	if (!flash_data_rd(&sig_cfg[0].byte[0], sizeof(sig_cfg), 0))
		signals_start_cfg(); //прочитать таблицу описания
	if (!flash_data_rd((int8_t*) (&sg_st.pc.cfg), sizeof(sg_st.pc.cfg), 1))
		parametrs_start_cfg(); //прочитать сигналы конфигурации
	//настроить переменные состояния алгоритма управления
	memset((uint8_t*) (&state), 0, sizeof(state));
	memset((uint8_t*) (&cmd), 0, sizeof(cmd));
	//настроить таймеры алгоритма управления
	//time.key_delay=timers_get_finish_time(0);
	memset((uint8_t*) (&time), 0, sizeof(time));
	set(AO_PC_SPEED_KP_KI, 0x00640064); // множители = 1.0 -> PC
	set(AO_PC_TORQUE_KP_KI, 0x00640064);
	//настроить датчики
	pulse_sens_init(1, 1);
	pulse_sens_init(2, 2);
	pulse_sens_init(3, 3);
#ifdef LOCAL_TEMP
	ds18b20_init(1);
	ds18b20_init(2);
	ds18b20_init(3);
	ds18b20_init(4);
	ds18b20_init(5);
	ds18b20_init(6);
	ds18b20_init(7);
	ds18b20_init(8);
#endif
	//nl_3dpas_init(CH1, ADR_NL_3DPAS);
#ifdef TORQ_DRIVER
	t46_init(CH2, ADR_T46);
#endif
#if UNI_CONTROL
	mu6u_init(CH2, ADR_MU6U);
	EcuPedControl (0.0f);
#elif SPSH_CONTROL
	spsh20_init(SPSH20_ADR);
#elif SERVO_CONTROL
	servo_init();
#else
	#error "Accelerator driver not defined"
#endif
	bcu_init(NODE_ID_BCU);
#ifdef FREQ_DRIVER
	atv61_init(CH1, NODE_ID_FC);
#endif
	pc_device_init();
}

//----------------------------------------------------------------------------------------------
void control_step (void) {
	read_devices(); //прочитать сосотяние датчиков
#if UNI_CONTROL
	if (SpeedCntrl == TSC1Control)  J1939_step();
	else if (SpeedCntrl == EaccControl) mu6u_step();
	else la10p_step(); // ServoControl
#elif SPSH_CONTROL
	pc_link_step();
	spsh20_step();
#elif SERVO_CONTROL
	servo_step();
#endif
	if (state.mode != PC_DEBUG) {
		read_keys();		//проанализировать команды пользователя
		work_step();		//выполнить шаг управления
		set_indication();	//обновить индикацию
	}
	update_devices(); //обновить состояние управляющих сигналов
}
//----------------------------------------------------------------------------------------------
void read_devices (void) {
	udata32_t crc; uint16_t size;

//----данные pc
 	pc_device_step();
//----данные контроллера
	//датчики времени
 	//sg_st.etcu.i.a[ETCU_AI_TIME]=rtc_sens_get_time();
 	//sg_st.etcu.i.a[ETCU_AI_DATE]=rtc_sens_get_date();
 	//датчики питания
	sg_st.etcu.i.a[ETCU_AI_5V]=power_get_5v();
	sg_st.etcu.i.a[ETCU_AI_VBAT]=power_get_vbat();
	//импульсные датчики
	sg_st.etcu.i.a[ETCU_AI_FUEL1]=pulse_sens_get_val(1);
	sg_st.etcu.i.a[ETCU_AI_FUEL2]=pulse_sens_get_val(2);
	sg_st.etcu.i.a[ETCU_AI_ROTATE]=pulse_sens_get_val(3);
	//аналоговые входы
	sg_st.etcu.i.a[ETCU_AI_P_EXHAUST]=adc_get_calc(10,1,0,3,3);//adc_sens_get_val(10); //10-й канал АЦП
	sg_st.etcu.i.a[ETCU_AI_I_P]=i_sens_get_val(12) * 10;//adc_get_calc(11,1,0,1,1);//adc_sens_get_val(11);
	sg_st.etcu.i.a[ETCU_AI_I_N]=i_sens_get_val(11) * 10;//adc_get_calc(12,1,0,1,1);//adc_sens_get_val(12);
	sg_st.etcu.i.a[ETCU_AI_U]=u_sens_get_val();//adc_get_calc(13,1,0,1,1);//adc_sens_get_val(13);
	//sg_st.etcu.i.a[ETCU_AI_FUEL_LEVEL]=p_745_get_val();//adc_get_calc(0,1,0,3,3);//adc_sens_get_val(0);
	sg_st.etcu.i.a[ETCU_AI_FUEL_LEVEL] = adc_get_u(0); // la10p датчик тока
	sg_st.etcu.i.a[ETCU_AI_T1]=t_auto_get_val(1);
	sg_st.etcu.i.a[ETCU_AI_T2]=t_auto_get_val(2);
	sg_st.etcu.i.a[ETCU_AI_T3]=t_auto_get_val(3);
	//sg_st.etcu.i.a[ETCU_AI_T4]=t_auto_get_val(6);
	//sg_st.etcu.i.a[ETCU_AI_T4]=t_auto_get_r(6);
	sg_st.etcu.i.a[ETCU_AI_T4] = adc_get_u(6); // la10p датчик положения
	sg_st.etcu.i.a[ETCU_AI_T5] = t_auto_get_r(7);
	sg_st.etcu.i.a[ETCU_AI_P_OIL]=p_mm370_get_val(14);//adc_get_calc(14,1,0,3,3);//adc_sens_get_val(14);
#define T_COOLANT_HI 		80000UL
#define COOLANT_FAN_HYST 	10000UL
 	int32_t t_cool = st(AI_T_COOLANT_OUT);
 	if (t_cool > T_COOLANT_HI) {
 		set(DO_COOLANT_FAN, ON);
 	} else if (t_cool < (T_COOLANT_HI - COOLANT_FAN_HYST)) {
 		set(DO_COOLANT_FAN, OFF);
 	}
#define LOW_ADD -29000.0f
#define LOW_MUL 35500.0f
#define HI_ADD 155000.0f
#define HI_MUL 73.0f
#define U_LINEAR 570UL
	double val; int32_t u_mv = adc_get_u(9);
	if (u_mv < U_LINEAR) val = log((double)u_mv) * LOW_MUL + LOW_ADD;
	else val = (double)u_mv * HI_MUL + HI_ADD;
	t_cool = (int32_t)val; // выхлопные газы
	if ((t_cool > 900000) || (t_cool < -5000)) t_cool = ERROR_CODE;
	sg_st.etcu.i.a[ETCU_AI_T_EXHAUST] = t_cool;
	//sg_st.etcu.i.a[ETCU_AI_T_EXHAUST] = adc_get_u(9);
	//sg_st.etcu.i.a[ETCU_AI_P_BOOST]=6789;//adc_get_calc(8,1,0,3,3);//adc_sens_get_val(8);
	//sg_st.etcu.i.a[ETCU_AI_T_BOOST]=9875;//adc_get_calc(15,1,0,3,3);//adc_sens_get_val(15);
#ifdef LOCAL_TEMP
 	//датчики температуры ds18b20
 	if (ds18b20_get_error(1)>2)sg_st.etcu.i.a[ETCU_AI_TEMP1] = ERROR_CODE;
 	else sg_st.etcu.i.a[ETCU_AI_TEMP1]=ds18b20_get_temp(1);
 	if (ds18b20_get_error(2)>2)sg_st.etcu.i.a[ETCU_AI_TEMP2] = ERROR_CODE;
 	else sg_st.etcu.i.a[ETCU_AI_TEMP2]=ds18b20_get_temp(2);
 	if (ds18b20_get_error(3)>2)sg_st.etcu.i.a[ETCU_AI_TEMP3] = ERROR_CODE;
 	else sg_st.etcu.i.a[ETCU_AI_TEMP3]=ds18b20_get_temp(3);
 	if (ds18b20_get_error(4)>2)sg_st.etcu.i.a[ETCU_AI_TEMP4] = ERROR_CODE;
 	else sg_st.etcu.i.a[ETCU_AI_TEMP4]=ds18b20_get_temp(4);
 	if (ds18b20_get_error(5)>2)sg_st.etcu.i.a[ETCU_AI_TEMP5] = ERROR_CODE;
 	else sg_st.etcu.i.a[ETCU_AI_TEMP5]=ds18b20_get_temp(5);
 	if (ds18b20_get_error(6)>2)sg_st.etcu.i.a[ETCU_AI_TEMP6] = ERROR_CODE;
 	else sg_st.etcu.i.a[ETCU_AI_TEMP6]=ds18b20_get_temp(6);
 	if (ds18b20_get_error(7)>2)sg_st.etcu.i.a[ETCU_AI_TEMP7] = ERROR_CODE;
 	else sg_st.etcu.i.a[ETCU_AI_TEMP7]=ds18b20_get_temp(7);
 	if (ds18b20_get_error(8)>2)sg_st.etcu.i.a[ETCU_AI_TEMP8] = ERROR_CODE;
 	else sg_st.etcu.i.a[ETCU_AI_TEMP8]=ds18b20_get_temp(8);
#endif
 	//----данные модуля управления гидротормозом
 	int32_t t_bcu;
 	error.bit.no_bcu = bcu_err_link();
	if (error.bit.no_bcu == 0) {
		sg_st.bcu.i.d=bcu_get_in();
		//sg_st.bcu.i.a[0] = bcu_get_t();
		//sg_st.bcu.i.a[0] = bcu_get_p();
		t_bcu = (bcu_get_t() * 24) / 12 + 7000;
		if ((t_bcu > 110000) || (t_bcu < 10000)) t_bcu = ERROR_CODE;
		sg_st.bcu.i.a[1] = t_bcu; // температура вместо давления в гидротормозе
		sg_st.bcu.i.a[2] = bcu_get_position();
		if (sg_st.bcu.i.d & 0x01) error.bit.emergancy_stop = 1;
		else error.bit.emergancy_stop = 0;
	} else {
		sg_st.bcu.i.d = 0;
		sg_st.bcu.i.a[0] = sg_st.bcu.i.a[1] = sg_st.bcu.i.a[2] = 0;
	}
#define T_OIL_ERR 		80000UL
#define T_OIL_HI 		60000UL
#define OIL_FAN_HYST 	5000UL
	if (t_bcu != ERROR_CODE) {
	 	if (t_bcu > T_OIL_HI) set(DO_OIL_FAN, ON);
	 	else if (t_bcu < (T_OIL_HI - OIL_FAN_HYST)) set(DO_OIL_FAN, OFF);
	}
 	if (t_bcu > T_OIL_ERR) {
		error.bit.err_brake = 1; //ощибка гидротормоза
		state.step = ST_STOP_ERR;
		cmd.opr = ST_STOP_ERR;
 	}
//----данные ПЧ
#ifdef FREQ_DRIVER
	error.bit.no_fc = atv61_err_link();
	if (error.bit.no_fc == 0) {
		sg_st.fc.i.a[0] = atv61_get_frequency();
		error.bit.err_fc = atv61_error();
	} else {
		sg_st.fc.i.a[0] = 0;
		error.bit.err_fc = 0;
		state.step = ST_STOP_ERR;
		cmd.opr = ST_STOP_ERR;
	}
#endif
//----данные сервопривода
#if UNI_CONTROL
	if (SpeedCntrl == TSC1Control) {
		if (!CntrlDevSel) { // not selected
			if (EcuCruiseActive()) {
				CntrlDevSel = true;
				goto select_1;
			} else if (EcuCruiseError()) {
				SpeedCntrl = EaccControl;
				//pid_tune_new(&Speed_PID, &Speed_Out, EcuPedControl);
				goto error_1;
			}
		} else {
			if (EcuCruiseError() == false) {
				select_1:
				sg_st.ta.i.a[0] = EcuPedalPos();
			} else {
				sg_st.ta.i.a[0] = 0;
				sg_st.ta.i.a[2] = SERVO_LINK_ERR;
				state.step = ST_STOP_ERR;
				cmd.opr = ST_STOP;
				error_1:
				error.bit.no_cruise = 1;
				//EcuCruiseReset();
			}
		}
	} else if (SpeedCntrl == EaccControl) {
		if (!CntrlDevSel) { // not selected
			if (EcuPedActive()) {
				CntrlDevSel = true;
				goto select_2;
			} else if (EcuPedError()) {
				SpeedCntrl = ServoControl;
				Speed_PID.u = 1500.0f;
				Speed_PID.d = 500.0f;
				pid_tune_new(&Speed_PID, &Speed_Out, la10p_set_out);
				state.step = ST_STOP;
				cmd.opr = ST_STOP;
				goto error_2;
			}
		} else {
			if (EcuPedError() == 0) {
				select_2:
				sg_st.ta.i.a[0] = EcuPedalPos();
			} else {
				sg_st.ta.i.a[0] = 0;
				sg_st.ta.i.a[2] = SERVO_LINK_ERR;
				state.step = ST_STOP_ERR;
				cmd.opr = ST_STOP_ERR;
				error_2:
				error.bit.no_cruise = 0;
				error.bit.no_eacc = 1;
			}
		}
	} else { // ServoControl
		la10p_st srv_st = la10p_state();
		sg_st.ta.i.a[1] = (int32_t)srv_st;
		sg_st.ta.i.a[0] = la10p_get_pos() * 1000;
		if (la10p_state() == LA10P_POWERED) {
			la10p_init();
			Ready = false;
			error.bit.servo_not_init = 1;
		} else if (srv_st == LA10P_STOP_ERR) {
			error.bit.servo_error = 1;
			state.step = ST_STOP_ERR;
			cmd.opr = ST_STOP_ERR;
			sg_st.ta.i.a[0] = ERROR_CODE;
		} else if (srv_st == LA10P_READY) {
			if (Ready == false) {
				Ready = false;
				error.bit.no_eacc = 0;
				error.bit.servo_not_init = 0;
				//error.bit.servo_init_ok = 1;
				//cmd.opr = OPR_START_TEST;
			}
		}
	}
#elif SPSH_CONTROL
	if (spsh20_err_link() == 0) {
		sg_st.ta.i.a[1] = spsh20_get_status();
		if (sg_st.ta.i.a[1] != SERVO_OK) goto servo_err;
		sg_st.ta.i.a[0] = spsh20_get_pos();
	} else {
servo_err:
		sg_st.ta.i.a[2]=SERVO_LINK_ERR;
		error.bit.no_ta = 1;
		state.step = ST_STOP_ERR;
		cmd.opr = ST_STOP_ERR;
	}
#elif SERVO_CONTROL
	servo_st srv_st = servo_state();
	sg_st.ta.i.a[1] = (int32_t)srv_st;
	sg_st.ta.i.a[0] = ERROR_CODE;
	if (SERVO_CURR_HIGH) { // превышение тока!
		if (timers_get_time_left(err_time) == 0) {
			error.bit.servo_error = 1;
			goto servo_stop_error;
		}
	}
	if (srv_st != SERVO_READY) {
		if (srv_st == SERVO_NOT_INIT) {
			error.bit.servo_not_init = 1;
		} else { // SERVO_STOP_ERR
			error.bit.no_ta = 1;
		}
servo_stop_error:
		state.step = ST_STOP_ERR;
		cmd.opr = ST_STOP_ERR;
	} else { //extern uint32_t CurrTime;
		//sg_st.ta.i.a[0] = CurrTime * 1000;
		sg_st.ta.i.a[0] = servo_get_pos() * 1000;
	}
#else
	#error "Accelerator driver not defined"
#endif
//----данные датчика параметров атмосферы
	/*if (nl_3dpas_err_link() == 0) { // ToDo: датчик уже не используется!!!
		sg_st.aps.i.a[0] = nl_3dpas_get_temperature();
		sg_st.aps.i.a[1] = nl_3dpas_get_pressure();
		sg_st.aps.i.a[2] = nl_3dpas_get_humidity();
	} else {
		sg_st.aps.i.a[0] =
		sg_st.aps.i.a[1] =
		sg_st.aps.i.a[2] = ERROR_CODE;
	}*/
//---данные датчика момента
#ifdef TORQ_DRIVER
	if (t46_err_link() == 0) {
		sg_st.bcu.i.a[3] = t46_get_torque();
		sg_st.bcu.i.a[4] = t46_get_freq();
		sg_st.bcu.i.a[5] = t46_get_power();
	} else {
		sg_st.bcu.i.a[3] = sg_st.bcu.i.a[4] = sg_st.bcu.i.a[5] = 0;
		error.bit.no_trq = 1;
		state.step = ST_STOP_ERR;
		cmd.opr = ST_STOP_ERR;
	}
#else
	sg_st.bcu.i.a[3] = (int32_t)(Torque_Out * 1000);
	sg_st.bcu.i.a[4] = (int32_t)(Speed_Out * 1000);
	sg_st.bcu.i.a[5] = (uint32_t)(Speed_Out * Torque_Out / POWER_FACTOR);
#endif
}

//----------------------------------------------------------------------------------------------
void update_devices (void) {
	dio_out_write(sg_st.etcu.o.d);
	bcu_set_pwm1(sg_st.bcu.o.a[0]);
	bcu_set_pwm2(sg_st.bcu.o.a[1]);
	bcu_set_position(sg_st.bcu.o.a[2]);
	bcu_set_out(sg_st.bcu.o.d);
	atv61_set_frequency(sg_st.fc.o.a[0]);
	pc_device_step();
}

//----------------------------------------------------------------------------------------------
void read_keys (void) {
	uint32_t time_all;

	if (timers_get_time_left(time.key_delay) != 0) return;
	if (st(DI_PC_TEST_START)) {
		cmd.opr = OPR_START_TEST;
		time.key_delay = timers_get_finish_time(st(CFG_KEY_DELAY));
		time_all = st(AI_PC_DURATION);
		time_all = st(AI_PC_DURATION) * 60 * 1000;
		time.all = timers_get_finish_time(time_all);
	}
	if (st(DI_PC_TEST_STOP)) {
		cmd.opr = ST_STOP;
		error.dword = 0;
#if 0/*UNI_CONTROL*/
		if ((SpeedCntrl == ServoControl) &&
				(la10p_state() == LA10P_POWERED)) {
			la10p_init();
			error.bit.servo_not_init = 1;
		}
#endif
		time.key_delay = timers_get_finish_time(st(CFG_KEY_DELAY));
	}
#if 0
	if (st(ENGINE_KEY_TASK)) {
		state.opr = OPR_KEY_ON;
		cmd.opr = OPR_KEY_ON;
		set(ENGINE_RELAY, ON);
		set(DO_OIL_PUMP, ON);
		time.key_delay = timers_get_finish_time(st(CFG_KEY_DELAY));
	}
#endif
#if 0/*UNI_CONTROL*/
	if (st(ENGINE_KEY_TASK)) { // Вкл. зажигания
		if (!st(ENGINE_RELAY)) {
			cmd.opr = OPR_KEY_ON;
			set(ENGINE_RELAY, ON);
			set(DO_OIL_PUMP, ON);
		} else {
			if (cmd.opr != ST_STOP) return;
		}
		time.key_delay = timers_get_finish_time(st(CFG_KEY_DELAY));
	}
#endif
}

//----------------------------------------------------------------------------------------------
void work_step (void) {
	int32_t val;
	if (st(SAVE_PID_VAL) || !pid_init) {
		pid_init = true;
		set(AO_PC_SPEED_KP_KI, st(AI_PC_SPEED_KP_KI));
		set(AO_PC_TORQUE_KP_KI, st(AI_PC_TORQUE_KP_KI));
		float32_t sp_Kp, sp_Ti;
		if (SpeedCntrl == EaccControl) {
			sp_Kp = SPEED_KP_EA;
			sp_Ti = SPEED_TI_EA;
		} else {
			sp_Kp = SPEED_KP_SP /** StSens_K*/;
			sp_Ti = SPEED_TI_SP /** StSens_K*/;
		}
		SpeedKp = sp_Kp * (float32_t)(AI_PC_SPEED_KP) / 100.0;
		SpeedTi = sp_Ti * (float32_t)(AI_PC_SPEED_TI) / 100.0;
		TorqueKp = TORQUE_KP * (float32_t)(AI_PC_TORQUE_KP) / 100.0;
		TorqueTi = TORQUE_TI * (float32_t)(AI_PC_TORQUE_TI) / 100.0;
		init_PID(); // Регуляторы контуров управления
	}
//--------------------ОБКАТКА----------------------------
	if (cmd.opr != state.opr) {
		state.opr = cmd.opr;
		state.step = ST_STOP;
	}
	if (state.step == ST_STOP_ERR) {
		set(DO_EMERGANCY, ON);
		set(DO_SIREN, ON);
	} else {
		set(DO_EMERGANCY, OFF);
		set(DO_SIREN, OFF);
	}
	if ((state.step == ST_STOP_ERR) || (state.step == ST_STOP_TIME)) {
		work_stop();
		return;
	}
	if (state.opr == OPR_START_TEST) {
		if (timers_get_time_left(time.all) == 0)
			state.step = ST_STOP_TIME;
//------------------ГОРЯЧАЯ ОБКАТКА----------------------
#if UNI_CONTROL
		if (st(DI_PC_HOT_TEST)) {
			if (SpeedCntrl != ServoControl) {
				if (state.step == ST_STOP) {
					set(START_RELAY, ON);
					state.step = ST_WAIT_ENGINE_START;
					time.alg = timers_get_finish_time(15000);
#ifdef MODEL_OBJ
					init_obj(); // нач. установка ОР1, ОР2
					Speed_Out = (float)st(CFG_MIN_ROTATE) / 1000.0 - 50.0;
#endif
				}
			} else { // SpeedCntrl == ServoControl
				if (state.step == ST_STOP) {
					set(DO_OIL_PUMP, ON);
					set(DO_FUEL_PUMP, ON);
					state.step = ST_FUEL_PUMP;
					time.alg = timers_get_finish_time(st(CFG_FUEL_PUMP_TIMEOUT));
				}
				if (state.step == ST_FUEL_PUMP) {
					if (timers_get_time_left(time.alg) == 0) {
#ifndef NO_BATTERY
						if (st(AI_U_AKB) >= st(CFG_U_AKB_NORMAL)) {
#ifndef SERVO_CONTROL
							Speed_Out = (float)st(CFG_MIN_ROTATE) / 1000.0 + 50.0;
#endif
#endif
							set(DO_STARTER, ON);
#ifdef MODEL_OBJ
							init_obj(); // нач. установка ОР1, ОР2
							state.step = ST_SET_ROTATION;
							time.alg = timers_get_finish_time(4000);
							cntrl_M_time = timers_get_finish_time(0);
#else
							time.alg = timers_get_finish_time(st(CFG_STARTER_ON_TIME));
							state.step = ST_STARTER_ON;
#endif
#ifndef NO_BATTERY
						} else {
							error.bit.err_akb = 1;
							state.step = ST_STOP_ERR;
						}
#endif
					}
				}
				if (state.step == ST_STARTER_ON) {
					if (st(AO_PC_CURRENT) >= st(CFG_MIN_I_STARTER)) {
						state.step = ST_WAIT_ENGINE_START;
					} else {
						if (timers_get_time_left(time.alg) == 0) {
							error.bit.err_starter = 1;
							state.step = ST_STOP_ERR;
						}
					}
				}
			}
#else // !UNI_CONTROL
		if (st(DI_PC_HOT_TEST)) { //горячая обкатка
			if (state.step == ST_STOP) {
				set(DO_OIL_PUMP, ON);
				set(DO_FUEL_PUMP, ON);
				state.step = ST_FUEL_PUMP;
				time.alg = timers_get_finish_time(st(CFG_FUEL_PUMP_TIMEOUT));
			}
			if (state.step == ST_FUEL_PUMP) {
				if (timers_get_time_left(time.alg) == 0) {
#ifndef NO_BATTERY
					if (st(AI_U_AKB) >= st(CFG_U_AKB_NORMAL)) {
#ifndef SERVO_CONTROL
						Speed_Out = (float)st(CFG_MIN_ROTATE) / 1000.0 + 50.0;
#endif
#endif
						set(DO_STARTER, ON);
#ifdef MODEL_OBJ
						init_obj(); // нач. установка ОР1, ОР2
						state.step = ST_SET_ROTATION;
						time.alg = timers_get_finish_time(4000);
						cntrl_M_time = timers_get_finish_time(0);
#else
						time.alg = timers_get_finish_time(st(CFG_STARTER_ON_TIME));
						state.step = ST_STARTER_ON;
#endif
#ifndef NO_BATTERY
					} else {
						error.bit.err_akb = 1;
						state.step = ST_STOP_ERR;
					}
#endif
				}
			}
			if (state.step == ST_STARTER_ON) {
				if (st(AO_PC_CURRENT) >= st(CFG_MIN_I_STARTER)) {
					state.step = ST_WAIT_ENGINE_START;
				} else {
					if (timers_get_time_left(time.alg) == 0) {
						error.bit.err_starter = 1;
						state.step = ST_STOP_ERR;
					}
				}
			}
#endif // #if UNI_CONTROL
			if (state.step == ST_WAIT_ENGINE_START) {
				if (st(AI_ROTATION_SPEED) >= st(CFG_MIN_ROTATE)) { // Запуск двигателя
					engine_start:
#if UNI_CONTROL
					if (SpeedCntrl != ServoControl) {
						set(START_RELAY, OFF);
						set(GEN_EXC_RELAY, ON); // включить Возбуждение Генератора
						time.alg = timers_get_finish_time(2000); // на 2 сек.
					} else { // SpeedCntrl == ServoControl
						set(DO_STARTER, OFF);
						time.alg = timers_get_finish_time(0);
					}
#else
					set(DO_STARTER, OFF);
					time.alg = timers_get_finish_time(0);
#endif
					state.step = ST_SET_ROTATION;
					cntrl_M_time = timers_get_finish_time(0);
				} else {
					if (timers_get_time_left(time.alg) == 0) {
#ifdef MODEL_OBJ
						goto engine_start;
#else
						set(START_RELAY, OFF);
						error.bit.engine_start = 1;
						state.step = ST_STOP_ERR;
#endif
					}
				}
			}
			if (state.step == ST_SET_ROTATION) { // Двухконтурная регулировка оборотов и момента
#if UNI_CONTROL
				if (SpeedCntrl != ServoControl) {
					if (timers_get_time_left(time.alg) == 0) {
						set(GEN_EXC_RELAY, OFF); // выключить Возбуждение Генератора
					}
#ifdef MODEL_OBJ
				} else {
					if (timers_get_time_left(time.alg) == 0) {
						if (st(AI_ROTATION_SPEED) >= st(CFG_MIN_ROTATE)) { // Запуск двигателя
							set(DO_STARTER, OFF);
						}
					}
#endif // MODEL_OBJ
				}
#else
#ifdef MODEL_OBJ
				if (timers_get_time_left(time.alg) == 0) {
					if (st(AI_ROTATION_SPEED) >= st(CFG_MIN_ROTATE)) { // Запуск двигателя
						set(DO_STARTER, OFF);
					}
				}
#endif // MODEL_OBJ
#endif // UNI_CONTROL
#ifdef SPSH_CONTROL
				int32_t pos = st(AI_SERVO_POSITION);
				if (pos > MAX_SERVO_POSITION || pos < MIN_SERVO_POSITION) {
					error.bit.engine_start = 1; // сбой управления сервоприводом
					state.step = ST_STOP_ERR;
				} else {
					if (st(AI_ROTATION_SPEED) < DEF_ERR_ROTATE) { // аварийный останов двигателя
						error.bit.engine_rotate = 1; // сбой управления сервоприводом
						state.step = ST_STOP_ERR;
					}
				}
#endif
				Speed_loop(); // управление контуром оборотов
				Torque_loop(Absolute); // управление контуром крутящего момента
			}
		} else { // ! if (st(DI_PC_HOT_TEST))
//------------------ХОЛОДНАЯ ПРОКРУТКА--------------------------
			if (state.step == ST_STOP) {
				time.alg = timers_get_finish_time(st(CFG_TIME_START_FC));
				state.step = ST_STRAT_FC;
			}
			if (state.step == ST_STRAT_FC) { // Запуск электромотора
				if (st(DI_PC_BRAKE_TEST) == ON) {
					set(DO_OIL_PUMP, ON);
				}
#ifndef FREQ_DRIVER
				// Cкорость вращения сразу на индикацию PC минуя ПЧ
				if (st(AI_PC_ROTATE) > st(CFG_MAX_ENGINE_SPEED))
					set(AI_FC_FREQ, st(CFG_MAX_ENGINE_SPEED));
				else set(AI_FC_FREQ, st(AI_PC_ROTATE));
#else
				// Заданая скорость вращения -> на частотник
				if (st(AI_PC_ROTATE) > st(CFG_MAX_ENGINE_SPEED))
					set(AO_FC_FREQ, st(CFG_MAX_ENGINE_SPEED));
				else set(AO_FC_FREQ, st(AI_PC_ROTATE));
#endif
#ifdef MODEL_OBJ
				Speed_Out = (float)st(AI_PC_ROTATE) / 1000.0;
				init_obj(); // нач. установка ОР1, ОР2
#endif
				if (timers_get_time_left(time.alg) == 0) {
					if (st(DI_PC_BRAKE_TEST) == ON) {
						state.step = ST_BREAK_TEST;
						cntrl_M_time = timers_get_finish_time(0);
					} else state.step = ST_COLD;
				}
			}
			if (state.step == ST_BREAK_TEST) {
				Torque_loop(Percent); // управление контуром крутящего момента
			}
#ifdef FREQ_DRIVER
			set(AO_FC_FREQ, st(AI_PC_ROTATE)); // Заданая скорость вращения -> на частотник
			if ((error.bit.err_fc) || (error.bit.err_fc_speed)) state.step = ST_STOP_ERR;
#else
#ifdef MODEL_OBJ
			set(AO_FC_FREQ, (int32_t)(Speed_Out * 1000.0)); // Заданая скорость вращения -> на частотник
			//sg_st.bcu.i.a[4] = st(AI_PC_ROTATE);
#endif
#endif
		}
//--------------------ОСТАНОВКА-------------------------------
	} else {
		cmd.opr = ST_STOP;
		servo_stop = true;
		work_stop();
	}
}

/*
 * ИНДИКАЦИЯ ПК
 */
void set_indication (void) {
	int32_t val = st(AI_TORQUE);
	if (val < 0) val *= -1;
	set(AO_PC_TORQUE, val);
	set(AO_PC_POWER, st(AI_POWER));
	//датчик момента
	if ((state.opr == OPR_START_TEST) && (st(DI_PC_HOT_TEST) == OFF)) {
#ifndef FREQ_DRIVER
		set(AO_PC_ROTATE, st(AI_PC_ROTATE));
#else
		//set(AO_PC_ROTATE, st(AI_FC_FREQ));
		set(AO_PC_ROTATE, st(AI_ROTATION_SPEED));
#endif
	} else {
		//set(AO_PC_ROTATE, st(AI_ROTATION_ETCU));
		set(AO_PC_ROTATE, st(AI_ROTATION_SPEED));
	}
	//температура выхлопных газов
	set(AO_PC_T_EXHAUST, st(AI_T_EXHAUST));
	//set(AO_PC_T_EXHAUST, st(AI_VALVE_POSITION) * 1000); // ToDo датчик тока СП
	//датчики температуры двигателя (датчик ВГ?)
	set(AO_PC_T_COOLANT_IN, st(AI_T_COOLANT_IN));
	set(AO_PC_T_COOLANT_OUT, st(AI_T_COOLANT_OUT));
	set(AO_PC_T_OIL_IN, st(AI_T_OIL_IN));
	//set(AO_PC_T_OIL_OUT, st(AI_T_OIL_OUT));
	//set(AO_PC_T_FUEL, st(AI_T_FUEL));
	set(AO_PC_T_FUEL, st(AI_T_AIR));
	//датчик топлива
	val = (st(AP_FUEL_IN) - st(AP_FUEL_OUT)) * 60;
	set(AO_PC_FUEL_CONSUM, val);
	// датчики уровней
#define LEVEL_MAX	100000
#define LEVEL_MIN	10000
#define LEVEL_ERR	125000
	val = (st(AI_T_FUEL) * 100000) / 92; // Уровень топлива XP8
	if (val > LEVEL_MAX) {
		if (val < LEVEL_ERR) val = LEVEL_MAX;
		else val = ERROR_CODE;
	}
	set(AO_PC_FUEL_LEVEL, val); // Уровень топлива XP8
	set(AO_PC_T_OIL_OUT, st(AI_T_OIL_OUT)); // ToDo: la10p датчик положения
#if 0 //
	val = (st(AI_T_OIL_OUT) * 100000) / 92; // Уровень масла XP7
	if (val > LEVEL_MAX) {
		if (val < LEVEL_ERR) val = LEVEL_MAX;
		else val = ERROR_CODE;
	}
	if ((val < LEVEL_MIN) || (val == ERROR_CODE)) {
		error.bit.err_brake = 1; //ощибка гидротормоза
		state.step = ST_STOP_ERR;
		cmd.opr = ST_STOP_ERR;
	}
	set(AO_PC_T_OIL_OUT, val); // Уровень масла XP7
#endif
	//датчики локального нагрева
	set(AO_PC_T_EXT1, st(AI_T_EXT1));
	set(AO_PC_T_EXT2, st(AI_T_EXT2));
	set(AO_PC_T_EXT3, st(AI_T_EXT3));
	set(AO_PC_T_EXT4, st(AI_T_EXT4));
	set(AO_PC_T_EXT5, st(AI_T_EXT5));
	set(AO_PC_T_EXT6, st(AI_T_EXT6));
	set(AO_PC_T_EXT7, st(AI_T_EXT7));
	set(AO_PC_T_EXT8, st(AI_T_EXT8));
	//аналоговые датчики
	set(AO_PC_P_EXHAUST, st(AI_P_EXHAUST)); //Аналоговый: Давление выхлопных газов
	set(AO_PC_P_OIL, st(AI_P_OIL)); //Аналоговый: Давление масла
	//set(AO_PC_P_CHARGE, st(AI_P_CHARGE)); //Аналоговый: Давление наддувочного воздуха
	//set(AO_PC_T_CHARGE, st(AI_T_CHARGE)); //Аналоговый: Температура наддувочного воздуха
	//set(AO_PC_P_MANIFOLD, st(AI_P_MANIFOLD)); //Аналоговый: Давление впускного коллектора
	//extern uint32_t curr;
	//set(AO_PC_P_MANIFOLD, curr); //Аналоговый: Давление впускного коллектора
	//параметры атмосферы
	set(AO_PC_T_ENV_AIR, st(AI_T_AIR));
	set(AO_PC_P_ENV_AIR, st(AI_P_AIR));
	set(AO_PC_H_ENV_AIR, st(AI_H_AIR));

	//ток и напряжение АКБ
	val=st(AI_I_AKB_P)-st(AI_I_AKB_N);
	set(AO_PC_CURRENT, val);
	set(AO_PC_VOLTAGE, st(AI_U_AKB));
	//сигналы педали газа
#ifdef SPSH_CONTROL
#if MIN_SERVO_POSITION < 0
	#define SERVO_MOVE (float32_t)(MIN_SERVO_POSITION - MAX_SERVO_POSITION)
#else
	#define SERVO_MOVE (float32_t)(MAX_SERVO_POSITION - MIN_SERVO_POSITION)
#endif
	float32_t fval = (float32_t)st(AI_SERVO_POSITION) / SERVO_MOVE * 100000.0f;
	val = (int32_t)fval;
	set(AO_PC_TROTTLE, val);		//Аналоговый: положение педали газа
#else
	set(AO_PC_TROTTLE, st(AI_SERVO_POSITION));		//Аналоговый: положение педали газа
#endif
	//сигналы гидротормоза
	set(AO_PC_T_BRAKE,	st(AI_T_OIL_BRAKE)); //Аналоговый: Температура в гидротормозе
	set(AO_PC_P_BRAKE,	st(AI_P_OIL_BRAKE)); //Аналоговый: давление в гидротормозе
	set(AO_PC_SET_BRAKE, st(AO_HYDROSTATION));	//Аналоговый: Установленная мощность гидротормоза в м%
	//время и дата
	//set(AO_PC_TIME, rtc_sens_get_time());
	//set(AO_PC_DATE, rtc_sens_get_date());
//---------------ламочки
#if UNI_CONTROL
	set(ENGINE_ON_LED, st(ENGINE_RELAY));				//Сигнал: Зажигание Включено
	set(DO_PC_COOLANT_FAN, st(DO_COOLANT_FAN));			//Сигнал:
	set(DO_PC_COOLANT_PUMP, st(DO_COOLANT_PUMP));		//Сигнал:
	set(DO_PC_OIL_PUMP,	st(DO_OIL_PUMP));				//Сигнал:
	set(DO_PC_COOLANT_HEATER, st(DO_COOLANT_HEATER));	//Сигнал:
	set(DO_PC_OIL_HEATER, st(DO_OIL_HEATER));			//Сигнал:
	set(GEN_EXC_LED, st(GEN_EXC_RELAY));				//Сигнал: Возбуждение Генератора
	set(DO_PC_OIL_FAN, st(DO_OIL_FAN));     			//Сигнал:
#else
	set(DO_PC_STARTER, st(DO_STARTER));					//Сигнал: Стратер
	set(DO_PC_COOLANT_FAN, st(DO_COOLANT_FAN));			//Сигнал: Вентилятор ОЖ
	set(DO_PC_COOLANT_PUMP, st(DO_COOLANT_PUMP));		//Сигнал: Насос ОЖ
	set(DO_PC_OIL_PUMP,	st(DO_OIL_PUMP));				//Сигнал: Насос масла
	set(DO_PC_COOLANT_HEATER, st(DO_COOLANT_HEATER));	//Сигнал: Нагреватель ОЖ
	set(DO_PC_OIL_HEATER, st(DO_OIL_HEATER));			//Сигнал: Нагреватель масла
	set(DO_PC_FUEL_PUMP, st(DO_FUEL_PUMP));				//Сигнал: Включить ТНВД
	set(DO_PC_OIL_FAN, st(DO_OIL_FAN));     			//Сигнал: Вентилятор масла
#endif
	set(DO_PC_BRAKE_FAIL, error.bit.err_brake);			//Сигнал: авария гидротормоза
//---------------статуcы
	set(AO_PC_STATUS, state.step);
	set(AO_PC_ERR_MAIN, error.dword);
}

//----------------------------------------------------------------------------------------------
uint8_t chek_out_val (int32_t val1, int32_t val2, int32_t delta) {
	int32_t val;

	val = val1 - val2;
	if ((val < -delta) || (val > delta)) return 0;
	return 1;
}

//----------------------------------------------------------------------------------------------
void work_stop (void) {
#if UNI_CONTROL
	set(ENGINE_RELAY, OFF);
#else
	set(DO_STARTER, OFF);
#endif
	set(AO_FC_FREQ, 0);
#ifndef FREQ_DRIVER
	set(AI_PC_ROTATE, 0);
#endif
	uint32_t pwm1_out = st(AO_HYDROSTATION);
	uint32_t pwm2_out = st(AO_VALVE_ENABLE);
	if (pwm1_out > 10) { // плавное снижение давления
		set(AO_HYDROSTATION, pwm1_out - 10);
	} else {
		set(AO_HYDROSTATION, 0);
		pwm1_out = 0;
	}
	if (pwm2_out > 10) { // плавное снижение давления
		set(AO_VALVE_ENABLE, pwm2_out - 10);
	} else {
		set(AO_VALVE_ENABLE, 0);
		pwm2_out = 0;
	}
	if (!pwm1_out && !pwm2_out) {
		set(DO_OIL_PUMP, OFF);
		set(DO_FUEL_PUMP, OFF);
	}
	set(AO_SERVO_POSITION, 0);
	//init_PID();
	pid_init = false;
#if UNI_CONTROL
	if (SpeedCntrl == TSC1Control) {
		if (timers_get_time_left(time.alg) == 0) { // останов
			time.alg = timers_get_finish_time(SPEED_LOOP_TIME);
			if (EcuCruiseState()) {
				StopCount = TSC1_STOP_RETRY;
			}
			if (StopCount) {
				StopCount--;
				EcuCruiseControl(0.0f, 0.0f);
			} else EcuCruiseReset();
		}
	} else if (SpeedCntrl == EaccControl) {
		EcuPedControl(0.0f);
	} else { // SpeedCntrl == ServoControl
		if (la10p_get_pos() > 0) la10p_set_out(0);
	}
#else
	ACCEL_SET(0);
#endif
#ifdef MODEL_OBJ
	ServoPos = 0;
	set(AI_FC_FREQ, 0);
	Speed_Out = Torque_Out = 0;
	Pwm1_Out = Pwm2_Out = 0;
	init_obj();
#endif
}

#ifdef MODEL_OBJ
/*
 * Характеристика объектов регулирования (ОР)
 * FrequeObj: выходной параметр ОР - обороты двигателя.
 * Входной параметр компексный, зависит на 60% положения положения привода
 * акселератора (+), на 40% от значения крутящего момента на валу двигателя (-).
 * TorqueObj: выходной параметр ОР - крутящий момент.
 * Входной параметр: производительность гидростанции (+).
 */
void init_obj (void) {
	TorqueObj.st = FrequeObj.st = HAL_GetTick();
	FrequeObj.out = (float32_t)st(CFG_MIN_ROTATE) / 1000.0; // пред. знач-е вых. парам.
	FrequeObj.tau = 3000.0; // постоянная времени апериодического звена, ms
	TorqueObj.out = 0.0; // пред. знач-е вых. парам.
	TorqueObj.tau = 2000.0; // постоянная времени апериодического звена, ms
}

/*
 * output: float32_t реакция ОР на входное воздействие
 * inp входное воздействие на ОР
 * obj указатель на ОР
 */
float32_t get_obj (obj_t * obj, float32_t inp) {
	float_t dt, z0, z1, out;
	uint32_t time = HAL_GetTick();
	if (obj->st <= time) dt = (float32_t)(time - obj->st);
	else dt = (float32_t)((0xffffffff - obj->st) + time);
	z0 = dt / (dt + obj->tau);
	z1 = 1 - z0;
	out = (z0 * inp) + (z1 * obj->out);
	obj->out = out;
	obj->st = time;
	return out;
}
#endif

/*
 * Инициализация контуров регулирования
 */
void init_PID (void) { // ToDo:
	pid_r_init(&Speed_PID);
	Speed_PID.Kp = SpeedKp;
	Speed_PID.Ti = SpeedTi;
	Speed_PID.Td = SPEED_TD;
	Speed_PID.Tf = SPEED_DF_TAU;
	pid_r_init(&Torque_PID);
	Torque_PID.Kp = TorqueKp;
	Torque_PID.Ti = TorqueTi;
	Torque_PID.Td = TORQUE_TD;
	Torque_PID.Tf = TORQUE_DF_TAU;
}

/*
 * Управление контуром оборотов
 */
void Speed_loop (void) {
	int32_t set_out; float32_t pi_out, task, torq_corr;
	if (timers_get_time_left(time.alg) == 0) { // управление контуром оборотов
		time.alg = timers_get_finish_time(SPEED_LOOP_TIME);
		tune_st tune = pid_tune_step();
		if (tune == TUNE_PROCEED) {
			tune_start = true;
#ifdef MODEL_OBJ
			if (SpeedCntrl == EaccControl) { // ToDo: накопление убрать
				pi_out = (float32_t)EcuPedalPos() / 100.0;
			} else { // SpeedCntrl == ServoControl
				pi_out = la10p_get_pos() * SPEED_FACT_LA10P;
			}
			Speed_Out = get_obj(&FrequeObj, pi_out);
#endif // MODEL_OBJ
			return;
		} else if (tune == TUNE_COMPLETE) {
			if (tune_start) {
				tune_start = false;
				error.bit.pid_stune_ok = 1;
				state.step = ST_STOP_TIME;
			}
		} else { // tune == TUNE_STOP_ERR
			tune_start = false;
		}
		task = (float32_t)st(AI_PC_ROTATE) / 1000.0;
		if (task < SPD_MIN) task = SPD_MIN;
		torq_corr = (Torque_Out / TORQUE_MAX);
		float32_t acc_state;
#if UNI_CONTROL
		if (SpeedCntrl == TSC1Control) {
			EcuCruiseControl(task, torq_corr);
		} else if (SpeedCntrl == EaccControl) {
			acc_state = EcuPedalPos();
			acc_state /= 1000.0;
		} else if (SpeedCntrl == ServoControl) {
			acc_state = la10p_get_pos();
		}
#else
		acc_state = ACCEL_STATE;
#endif
		task -= Speed_Out; // PID input Error
		/*float32_t tmp = fabs(task);
		Speed_PID.Kp = SpeedKp * (1 + torq_corr);
		if (SpeedCntrl == ServoControl)
			Speed_PID.Kp *= exp(-tmp / SPEED_MAX);*/
		/*if (((acc_state >= 95.0) && (task > 0)) ||
				((acc_state <= 5.0) && (task < 0))) {
			Speed_PID.Xi = 0;
		}*/ /*else {
			Speed_PID.Ki = SpeedKi * exp(-tmp / SPEED_MAX);
			Speed_PID.Kp = SpeedKp * (1 + torq_corr);
			if (SpeedCntrl == ServoControl)
				Speed_PID.Kp *= exp(-tmp / SPEED_MAX);
		}*/
#if UNI_CONTROL
		float32_t zone_dead = 0;
		if (SpeedCntrl == EaccControl) {
			zone_dead = ZONE_DEAD_EACC;
		} else if (SpeedCntrl == ServoControl) {
			zone_dead = ZONE_DEAD_LA10P / StSens_K;
		}
		Speed_PID.Xd = zone_dead;
#else
		if (tmp < ZONE_DEAD_REF) task = 0;
#endif
		pi_out = pid_r(&Speed_PID, task);
#if UNI_CONTROL
		if (SpeedCntrl == EaccControl) { // ToDo: накопление убрать
			float32_t acc_out = acc_state / 100.0
					+ (pi_out - SPD_MIN) * SPEED_MUL_EACC;
			EcuPedControl(acc_out);
		} else if (SpeedCntrl == ServoControl)
			la10p_set_out(pi_out /** SPEED_MUL_LA10P*/);
#else
		ACCEL_SET(pi_out * SPEED_MUL);
#endif
#ifdef MODEL_OBJ
		torq_corr *= TORQUE_FACTOR;
#if UNI_CONTROL
		if (SpeedCntrl == TSC1Control) {
			task = (float32_t)st(AI_PC_ROTATE) / 1000.0;
			if (!EcuCruiseActive()) task = SPD_ERR;
			else if (task < SPD_MIN) task = SPD_MIN;
			pi_out = task;
		} else {
			if (SpeedCntrl == EaccControl) { // ToDo: накопление убрать
				pi_out += acc_state / 100.0;
			} else { // SpeedCntrl == ServoControl
				pi_out = la10p_get_pos() * SPEED_FACT_LA10P;
			}
			pi_out *= (1 - torq_corr);
		}
#else
		pi_out = ACCEL_STATE * SPEED_FACT;
		pi_out *= (1 - torq_corr);
#endif
		Speed_Out = get_obj(&FrequeObj, pi_out);
#endif // MODEL_OBJ
	}
}

/*
 * Управление контуром крутящего момента
 */
void Torque_loop (torq_val_t val) {
	uint32_t pwm_out, pwm1_out, pwm2_out;
	float32_t pwm_flo;
	if (timers_get_time_left(cntrl_M_time) == 0) { // управление контуром крутящего момента
		if (val == Percent) { // тест ИМ гидросистемы без ОС
			pwm1_out = st(AI_PC_TORQUE) & 0xFFFF; // Гидро Насос
			pwm2_out = st(AI_PC_TORQUE) >> 16; // Гидро Клапан
#ifdef MODEL_OBJ
			pwm_out = (pwm1_out * 2) + (pwm2_out * 8); // распр. возд. 1:4
			Torque_Out = ((float32_t)pwm_out * PERCENT_FACTOR)
				* TFILTER_TAU + Torque_Out * (1.0 - TFILTER_TAU);
#endif
			pwm1_out = pwm1_out * (PWM_SCALE / 10000); // Гидро Насос
			if (pwm1_out > PWM_SCALE) pwm1_out = PWM_SCALE;
			pwm2_out = pwm2_out * (PWM_SCALE / 10000); // Гидро Клапан
			if (pwm2_out > PWM_SCALE) pwm2_out = PWM_SCALE;
			pwm1_out = (uint32_t)((float32_t)pwm1_out
					* TFILTER_TAU + (float32_t)Pwm1_Out * (1.0 - TFILTER_TAU));
			pwm2_out = (uint32_t)((float32_t)pwm2_out
					* TFILTER_TAU + (float32_t)Pwm2_Out * (1.0 - TFILTER_TAU));
			Pwm1_Out = pwm1_out;
			Pwm2_Out = pwm2_out;
		} else { // val_type == Absolute
			float32_t pi_out, task;
			task = st(AI_PC_TORQUE) / 1000.0;
			task -= Torque_Out; // PID input Error
			pi_out = pid_r(&Torque_PID, task);
#ifdef MODEL_OBJ
			Torque_Out = get_obj(&TorqueObj, pi_out);
#endif
			if (pi_out < 0) pi_out = 0;
			pi_out *= TORQUE_SCALE;
			if (pi_out > PWM_FSCALE) pi_out = PWM_FSCALE;
			pwm_flo = (uint32_t)(PWM_P_MIN + pi_out * (PUMP_MAX - PUMP_MIN)); // Гидро Насос
			if (pwm_flo > PWM_P_MAX) pwm_flo = PWM_P_MAX;
			pwm1_out = (uint32_t)pwm_flo;
			pwm_flo = (PWM_V_MIN + pi_out * (VALVE_MAX - VALVE_MIN)); // Гидро Клапан
			if (pwm_flo > PWM_V_MAX) pwm_flo = PWM_V_MAX;
			pwm2_out = (uint32_t)pwm_flo;
		}
		set(AO_HYDROSTATION, pwm1_out);
		set(AO_VALVE_ENABLE, pwm2_out);
		cntrl_M_time = timers_get_finish_time(TORQUE_LOOP_TIME);
	}
}

