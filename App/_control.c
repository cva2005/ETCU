/*
 * _control.c
 *
 *  Created on: 12 ���. 2016 �.
 *      Author: ������ �.�
 */
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "arm_math.h"
#include "timers.h"
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
#include "servo.h"

extern sig_cfg_t sig_cfg[SIG_END];  //�������� ��������
extern sg_t sg_st;					//��������� ��������

static state_t state;
static cmd_t cmd;
static timeout_t time;
static error_t error;
static stime_t cntrl_M_time; // ������ ������������� ���������� �������
arm_pid_instance_f32 Speed_PID;
arm_pid_instance_f32 Torque_PID;
void init_PID (void);
void Speed_loop (void);
void Torque_loop (torq_val_t val);
#ifdef MODEL_OBJ
	obj_t FrequeObj;
	obj_t TorqueObj;
	float32_t get_obj (obj_t * obj, float32_t inp);
	void init_obj (void);
#endif
uint32_t Pwm1_Out = 0, Pwm2_Out = 0;

void signals_start_cfg (void) {
	uint16_t cnt, nmb;

	memset((uint8_t*) (&sig_cfg), 0, sizeof(sig_cfg));
//��������� �������� ������������--------------------------
	for (cnt=0; cnt<DO_STARTER; cnt++) {
		sig_cfg[cnt].fld.deivice=PC;
		sig_cfg[cnt].fld.activ=1;
		sig_cfg[cnt].fld.type=CFG;
		sig_cfg[cnt].fld.number=cnt;
	}
//��������� �������� ETCU----------------------------------
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
	sig_cfg[AI_T_EXHAUST].fld.number=ETCU_AI_T_EXHAUST;	//����������: ����������� ��������� �����
	sig_cfg[AI_T_COOLANT_IN].fld.number=ETCU_AI_T1;		//����������: ����������� �� �� �����
	sig_cfg[AI_T_COOLANT_OUT].fld.number=ETCU_AI_T2;	//����������: ����������� �� �� ������
	sig_cfg[AI_T_OIL_IN].fld.number=ETCU_AI_T3;			//����������: ����������� ����� �� �����
	sig_cfg[AI_T_OIL_OUT].fld.number=ETCU_AI_T4;		//����������: ����������� ����� �� ������
	sig_cfg[AI_T_FUEL].fld.number=ETCU_AI_T5;			//����������: ����������� �������
	sig_cfg[AP_FUEL_IN].fld.number=ETCU_AI_FUEL1;		//����������: ���������� �������
	sig_cfg[AP_FUEL_OUT].fld.number=ETCU_AI_FUEL2;		//����������: ������������ �������
	sig_cfg[AI_FUEL_LEVEL].fld.number=ETCU_AI_FUEL_LEVEL;//����������: ������ ������ �������
	sig_cfg[AI_T_EXT1].fld.number=ETCU_AI_TEMP1;		//����������: ������ ���������� ���������� ������� 1
	sig_cfg[AI_T_EXT2].fld.number=ETCU_AI_TEMP2;		//����������: ������ ���������� ���������� ������� 2
	sig_cfg[AI_T_EXT3].fld.number=ETCU_AI_TEMP3;		//����������: ������ ���������� ���������� ������� 3
	sig_cfg[AI_T_EXT4].fld.number=ETCU_AI_TEMP4;		//����������: ������ ���������� ���������� ������� 4
	sig_cfg[AI_T_EXT5].fld.number=ETCU_AI_TEMP5;		//����������: ������ ���������� ���������� ������� 5
	sig_cfg[AI_T_EXT6].fld.number=ETCU_AI_TEMP6;		//����������: ������ ���������� ���������� ������� 6
	sig_cfg[AI_T_EXT7].fld.number=ETCU_AI_TEMP7;		//����������: ������ ���������� ���������� ������� 7
	sig_cfg[AI_T_EXT8].fld.number=ETCU_AI_TEMP8;		//����������: ������ ���������� ���������� ������� 8
	sig_cfg[AI_P_EXHAUST].fld.number=ETCU_AI_P_EXHAUST;	//����������: �������� ��������� �����
	sig_cfg[AI_P_OIL].fld.number=ETCU_AI_P_OIL;			//����������: �������� �����
	sig_cfg[AI_P_CHARGE].fld.number=ETCU_AI_P_BOOST;	//����������: �������� ������������ �������
	sig_cfg[AI_T_CHARGE].fld.number=ETCU_AI_T_BOOST;	//����������: ����������� ������������ �������
	sig_cfg[AI_P_MANIFOLD].fld.number=ETCU_AI_FUEL_LEVEL;//����������: �������� ��������� ����������
	sig_cfg[AI_VBAT].fld.number=ETCU_AI_VBAT;			//����������: ���������� ���������
	sig_cfg[AI_5V].fld.number=ETCU_AI_5V;				//����������: ���������� ������� 5�
	sig_cfg[AI_TIME].fld.number=ETCU_AI_TIME;			//����������: �����
	sig_cfg[AI_DATE].fld.number=ETCU_AI_DATE;			//����������: ����
	sig_cfg[AI_ROTATION_ETCU].fld.number=ETCU_AI_ROTATE;//����������: ���������� �������� � ������
	sig_cfg[AI_I_AKB_P].fld.number=ETCU_AI_I_P;			//����������: ��� ��� ���������
	sig_cfg[AI_I_AKB_N].fld.number=ETCU_AI_I_N;			//����������: ��� ��� ����������
	sig_cfg[AI_U_AKB].fld.number=ETCU_AI_U;				//����������: ���������� ���
	sig_cfg[DO_STARTER].fld.number=0;					//������: �������
	sig_cfg[DO_COOLANT_FAN].fld.number=1;				//������: ���������� ��
	sig_cfg[DO_COOLANT_PUMP].fld.number=2;				//������: ����� ��
	//sig_cfg[DO_OIL_PUMP].fld.number=3;					//������: ����� �����
	sig_cfg[DO_OIL_PUMP].fld.number=1;					//������: ����� ����� (���������� � BCU)
	sig_cfg[DO_COOLANT_HEATER].fld.number=4;			//������: ����������� ��
	sig_cfg[DO_OIL_HEATER].fld.number=5;				//������: ����������� �����
	sig_cfg[DO_FUEL_PUMP].fld.number=6;					//������: �������� ����
//��������� �������� APS---------------------------------
	for (cnt=AI_T_AIR; cnt<DO_EMERGANCY; cnt++) {
		sig_cfg[cnt].fld.deivice=APS;
		sig_cfg[cnt].fld.activ=1;
		sig_cfg[cnt].fld.type=AI;
		sig_cfg[cnt].fld.number=cnt-AI_T_AIR;
	}
//��������� �������� BCU---------------------------------
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
	sig_cfg[DO_EMERGANCY].fld.number=2;			//������: ������
	sig_cfg[DO_SIREN].fld.number=3;				//������: ������
	sig_cfg[DO_OIL_PUMP].fld.number=1;			//������: ����� ����� (���������� �� ETCU)
	sig_cfg[DO_OIL_FAN].fld.number=0;			//������: ���������� ���������� �����
	sig_cfg[AI_T_OIL_BRAKE].fld.number=0;		//����������: ����������� ����� � ������������
	sig_cfg[AI_P_OIL_BRAKE].fld.number=1;		//����������: �������� ����� � ������������
	sig_cfg[AI_VALVE_POSITION].fld.number=2;		//������� �������
	sig_cfg[AI_TORQUE].fld.number=3;				//����������: ������ ��������
	sig_cfg[AI_ROTATION_SPEED].fld.number=4;		//����������: �������� ��������
	sig_cfg[AI_POWER].fld.number=5;				//����������: ��������
	sig_cfg[AO_HYDROSTATION].fld.number=0;		//������������������ ������������
	sig_cfg[AO_VALVE_ENABLE].fld.number=1;		//������ ���/���� �������
	sig_cfg[AO_VALVE_POSITION].fld.number=2;	//���������� ������� �������
//��������� �������� FC------------------------------------
	sig_cfg[AO_FC_FREQ].fld.deivice=FC;
	sig_cfg[AO_FC_FREQ].fld.activ=1;
	sig_cfg[AO_FC_FREQ].fld.type=AO;
	sig_cfg[AO_FC_FREQ].fld.number=0;
	sig_cfg[AI_FC_FREQ].fld.deivice=FC;
	sig_cfg[AI_FC_FREQ].fld.activ=1;
	sig_cfg[AI_FC_FREQ].fld.type=AI;
	sig_cfg[AI_FC_FREQ].fld.number=0;
//��������� �������� TA------------------------------------
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
//��������� �������� ��������� ����������-------------------------------
	for (cnt=AI_CDU_U; cnt<DI_PC_TEST_START; cnt++) {
		sig_cfg[cnt].fld.deivice=CDU;
		sig_cfg[cnt].fld.activ=1;
		sig_cfg[cnt].fld.type=AI;
	}
	sig_cfg[AI_CDU_ERR].fld.number=0;
	sig_cfg[AI_CDU_ST].fld.number=1;
	sig_cfg[AI_CDU_I].fld.number=2;
	sig_cfg[AI_CDU_U].fld.number=3;
//��������� �������� ��������-------------------------------
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
	set(CFG_KEY_DELAY, DEF_KEY_DELAY);			//����������� �� ������
	set(CFG_PULSE1, PULSE_SENS_PL);
	set(CFG_PULSE2, PULSE_SENS_PL);
	set(CFG_PULSE3, PULSE_SENS_PL);
	set(CFG_SERVO_MAX_I, DEF_MAX_I_SERVO);
	set(CFG_MAX_ENGINE_SPEED, DEF_MAX_ENGINE_SPEED);
	set(CFG_MAX_SERVO_POSITION, DEF_MAX_SERVO_POSITION);	//������������ ��������� ������������,
	set(CFG_U_AKB_NORMAL,DEF_U_AKB_NORMAL);	//����������� ����������� ��� ��� ������� ���������
	set(CFG_STARTER_ON_TIME,DEF_STARTER_ON_TIME);	//����� ��������� ��������, ���� ��������� �� �����������
	set(CFG_MIN_ROTATE,DEF_MIN_ROTATE);	//����������� ������� ��������� ��� ����������� ��� �������
	set(CFG_MIN_I_STARTER,DEF_MIN_I_STARTER);	//����������� ��� ��������
	set(CFG_FUEL_PUMP_TIMEOUT,DEF_FUEL_PUMP_TIMEOUT);	//����� ������ ���� ����� �������� ��������
	set(CFG_TIME_START_FC,DEF_TIME_START_FC);	//������� �� ������ ��
	set(CFG_HYDROSTATION_TETS,DEF_HYDROSTATION_TETS);	//������� ������� ������� ��� �������� ������������
	set(CFG_SPEED_TIME_HYDRO,DEF_SPEED_TIME_HYDRO);	//����� ������ �� ������ �������� ������������
	set(CFG_TIMOUT_ENGINE_ROTATE,DEF_TIMOUT_ENGINE_ROTATE); //������� ������� ������ ����� �� �������� ������� ���������
	set(CFG_TIMOUT_SET_TORQUE,DEF_TIMOUT_SET_TORQUE); //����� ��������� ��������� ��������� ������
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

	init_PID(); // ���������� �������� ����������
	if (!flash_data_rd(&sig_cfg[0].byte[0], sizeof(sig_cfg), 0))
		signals_start_cfg(); //��������� ������� ��������
	if (!flash_data_rd((int8_t*) (&sg_st.pc.cfg), sizeof(sg_st.pc.cfg), 1))
		parametrs_start_cfg(); //��������� ������� ������������
	//��������� ���������� ��������� ��������� ����������
	memset((uint8_t*) (&state), 0, sizeof(state));
	memset((uint8_t*) (&cmd), 0, sizeof(cmd));
	//��������� ������� ��������� ����������
	//time.key_delay=timers_get_finish_time(0);
	memset((uint8_t*) (&time), 0, sizeof(time));
	//��������� �������
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
	nl_3dpas_init(ADR_NL_3DPAS);
#ifndef NO_TORQ_DRIVER
	t46_init(ADR_T46);
#endif
#ifndef NO_SPSH_20
	spsh20_init(SPSH20_ADR);
#elif !NO_SERVO_DRIVER
	servo_init();
#endif
	bcu_init(NODE_ID_BCU);
#ifndef NO_FREQ_DRIVER
	atv61_init(NODE_ID_FC);
#endif
	pc_device_init();
}

//----------------------------------------------------------------------------------------------
void control_step (void) {
	read_devices(); //��������� ��������� ��������
	if (state.mode != PC_DEBUG) {
		read_keys();		//���������������� ������� ������������
		work_step();		//��������� ��� ����������
		set_indication();	//�������� ���������
	}
	update_devices(); //�������� ��������� ����������� ��������
}
//----------------------------------------------------------------------------------------------
void read_devices (void) {
	udata32_t crc; uint16_t size;

//----������ pc
 	pc_device_step();
//----������ �����������
	//������� �������
 	sg_st.etcu.i.a[ETCU_AI_TIME]=rtc_sens_get_time();
 	sg_st.etcu.i.a[ETCU_AI_DATE]=rtc_sens_get_date();
 	//������� �������
	sg_st.etcu.i.a[ETCU_AI_5V]=power_get_5v();
	sg_st.etcu.i.a[ETCU_AI_VBAT]=power_get_vbat();
	//���������� �������
	sg_st.etcu.i.a[ETCU_AI_FUEL1]=pulse_sens_get_val(1);
	sg_st.etcu.i.a[ETCU_AI_FUEL2]=pulse_sens_get_val(2);
	sg_st.etcu.i.a[ETCU_AI_ROTATE]=pulse_sens_get_val(3);
	//���������� �����
	sg_st.etcu.i.a[ETCU_AI_P_EXHAUST]=adc_get_calc(10,1,0,3,3);//adc_sens_get_val(10); //10-� ����� ���
	sg_st.etcu.i.a[ETCU_AI_I_P]=i_sens_get_val(12) * 10;//adc_get_calc(11,1,0,1,1);//adc_sens_get_val(11);
	sg_st.etcu.i.a[ETCU_AI_I_N]=i_sens_get_val(11) * 10;//adc_get_calc(12,1,0,1,1);//adc_sens_get_val(12);
	sg_st.etcu.i.a[ETCU_AI_U]=u_sens_get_val();//adc_get_calc(13,1,0,1,1);//adc_sens_get_val(13);
	sg_st.etcu.i.a[ETCU_AI_FUEL_LEVEL]=p_745_get_val();//adc_get_calc(0,1,0,3,3);//adc_sens_get_val(0);
	sg_st.etcu.i.a[ETCU_AI_T1]=t_auto_get_val(1);
	sg_st.etcu.i.a[ETCU_AI_T2]=t_auto_get_val(2);
	sg_st.etcu.i.a[ETCU_AI_T3]=t_auto_get_val(3);
	//sg_st.etcu.i.a[ETCU_AI_T4]=t_auto_get_val(6);
	sg_st.etcu.i.a[ETCU_AI_T4]=t_auto_get_r(6);
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
	t_cool = (int32_t)val; // ��������� ����
	if ((t_cool > 900000) || (t_cool < -5000)) t_cool = ERROR_CODE;
	sg_st.etcu.i.a[ETCU_AI_T_EXHAUST] = t_cool;
	//sg_st.etcu.i.a[ETCU_AI_T_EXHAUST] = adc_get_u(9);
	//sg_st.etcu.i.a[ETCU_AI_P_BOOST]=6789;//adc_get_calc(8,1,0,3,3);//adc_sens_get_val(8);
	//sg_st.etcu.i.a[ETCU_AI_T_BOOST]=9875;//adc_get_calc(15,1,0,3,3);//adc_sens_get_val(15);
#ifdef LOCAL_TEMP
 	//������� ����������� ds18b20
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
 	//----������ ������ ���������� �������������
 	int32_t t_bcu;
 	error.bit.no_bcu = bcu_err_link();
	if (error.bit.no_bcu == 0) {
		sg_st.bcu.i.d=bcu_get_in();
		//sg_st.bcu.i.a[0] = bcu_get_t();
		//sg_st.bcu.i.a[0] = bcu_get_p();
		t_bcu = (bcu_get_t() * 24) / 12 + 7000;
		if ((t_bcu > 110000) || (t_bcu < 10000)) t_bcu = ERROR_CODE;
		sg_st.bcu.i.a[1] = t_bcu; // ����������� ������ �������� � ������������
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
		error.bit.err_brake = 1; //������ ������������
		state.step = ST_STOP_ERR;
		cmd.opr = ST_STOP_ERR;
 	}
//----������ ��
#ifndef NO_FREQ_DRIVER
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
//----������ ������������
#ifndef NO_SPSH_20
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
#elif !NO_SERVO_DRIVER
	servo_st srv_st = servo_state();
	sg_st.ta.i.a[1] = (int32_t)srv_st;
	sg_st.ta.i.a[0] = ERROR_CODE;
	if (SERVO_CURR_HIGH) { // ���������� ����!
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
		sg_st.ta.i.a[0] = servo_get_pos()  * 1000;
	}
#endif
//----������ ������� ���������� ���������
	if (nl_3dpas_err_link() == 0) {
		sg_st.aps.i.a[0] = nl_3dpas_get_temperature();
		sg_st.aps.i.a[1] = nl_3dpas_get_pressure();
		sg_st.aps.i.a[2] = nl_3dpas_get_humidity();
	} else {
		sg_st.aps.i.a[0] =
		sg_st.aps.i.a[1] =
		sg_st.aps.i.a[2] = ERROR_CODE;
	}
//---������ ������� �������
#ifndef NO_TORQ_DRIVER
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

	if (timers_get_time_left(time.key_delay)!=0) return;
	if (st(DI_PC_TEST_START)) {
		cmd.opr=OPR_START_TEST;
		time.key_delay=timers_get_finish_time(st(CFG_KEY_DELAY));
		time_all=st(AI_PC_DURATION);
		time_all=st(AI_PC_DURATION)*60*1000;
		time.all=timers_get_finish_time(time_all);
	}
	if (st(DI_PC_TEST_STOP)) {
		cmd.opr=OPR_STOP_TEST;
		error.dword=0;
		time.key_delay=timers_get_finish_time(st(CFG_KEY_DELAY));
	}
}

//----------------------------------------------------------------------------------------------
void work_step (void) {
	int32_t val;

	if (st(DI_PC_SET_TIME))
		rtc_sens_set_datetime(st(AI_PC_DATE),st(AI_PC_TIME));
//--------------------�������----------------------------
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
//------------------������� �������----------------------
		if (st(DI_PC_HOT_TEST)) { //������� �������
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
#elif NO_SERVO_DRIVER
						Speed_Out = (float)st(CFG_MIN_ROTATE) / 1000.0 + 50.0;
#endif
						set(DO_STARTER, ON);
#ifdef MODEL_OBJ
						init_obj(); // ���. ��������� ��1, ��2
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
			if (state.step == ST_WAIT_ENGINE_START) {
				if (st(AI_ROTATION_SPEED) >= st(CFG_MIN_ROTATE)) { // ������ ���������
					set(DO_STARTER, OFF);
					state.step = ST_SET_ROTATION;
					time.alg = timers_get_finish_time(0);
					cntrl_M_time = timers_get_finish_time(0);
				} else {
					if (timers_get_time_left(time.alg) == 0) {
						error.bit.engine_start = 1;
						state.step = ST_STOP_ERR;
					}
				}
			}
			if (state.step == ST_SET_ROTATION) { // ������������� ����������� �������� � �������
#ifdef MODEL_OBJ
				if (timers_get_time_left(time.alg) == 0) {
					if (st(AI_ROTATION_SPEED) >= st(CFG_MIN_ROTATE)) { // ������ ���������
						set(DO_STARTER, OFF);
					}
				}
#endif // MODEL_OBJ
#ifndef NO_SPSH_20
				int32_t pos = st(AI_SERVO_POSITION);
				if (pos > MAX_SERVO_POSITION || pos < MIN_SERVO_POSITION) {
					error.bit.engine_start = 1; // ���� ���������� �������������
					state.step = ST_STOP_ERR;
				} else {
					if (st(AI_ROTATION_SPEED) < DEF_ERR_ROTATE) { // ��������� ������� ���������
						error.bit.engine_rotate = 1; // ���� ���������� �������������
						state.step = ST_STOP_ERR;
					}
				}
#endif
				Speed_loop(); // ���������� �������� ��������
				Torque_loop(Absolute); // ���������� �������� ��������� �������
			}
		} else { // ! if (st(DI_PC_HOT_TEST))
//------------------�������� ���������--------------------------
			if (state.step == ST_STOP) {
				time.alg = timers_get_finish_time(st(CFG_TIME_START_FC));
				state.step = ST_STRAT_FC;
			}
			if (state.step == ST_STRAT_FC) { // ������ �������������
				if (st(DI_PC_BRAKE_TEST) == ON) {
					set(DO_OIL_PUMP, ON);
				}
#ifdef NO_FREQ_DRIVER
				// C������� �������� ����� �� ��������� PC ����� ��
				if (st(AI_PC_ROTATE) > st(CFG_MAX_ENGINE_SPEED))
					set(AI_FC_FREQ, st(CFG_MAX_ENGINE_SPEED));
				else set(AI_FC_FREQ, st(AI_PC_ROTATE));
#else
				// ������� �������� �������� -> �� ���������
				if (st(AI_PC_ROTATE) > st(CFG_MAX_ENGINE_SPEED))
					set(AO_FC_FREQ, st(CFG_MAX_ENGINE_SPEED));
				else set(AO_FC_FREQ, st(AI_PC_ROTATE));
#endif
#ifdef MODEL_OBJ
				Speed_Out = (float)st(AI_PC_ROTATE) / 1000.0;
				init_obj(); // ���. ��������� ��1, ��2
#endif
				if (timers_get_time_left(time.alg) == 0) {
					if (st(DI_PC_BRAKE_TEST) == ON) {
						state.step = ST_BREAK_TEST;
						cntrl_M_time = timers_get_finish_time(0);
					} else state.step = ST_COLD;
				}
			}
			if (state.step == ST_BREAK_TEST) {
				Torque_loop(Percent); // ���������� �������� ��������� �������
			}
#ifndef NO_FREQ_DRIVER
			set(AO_FC_FREQ, st(AI_PC_ROTATE)); // ������� �������� �������� -> �� ���������
			if ((error.bit.err_fc) || (error.bit.err_fc_speed)) state.step = ST_STOP_ERR;
#else
#ifdef MODEL_OBJ
			set(AO_FC_FREQ, (int32_t)(Speed_Out * 1000.0)); // ������� �������� �������� -> �� ���������
			//sg_st.bcu.i.a[4] = st(AI_PC_ROTATE); // ToDo
#endif
#endif
		}
//--------------------���������-------------------------------
	} else {
		cmd.opr = ST_STOP;
		servo_stop = true;
		work_stop();
	}
}

/*
 * ��������� ��
 */
void set_indication (void) {
	int32_t val = st(AI_TORQUE);
	if (val < 0) val *= -1;
	set(AO_PC_TORQUE, val);
	set(AO_PC_POWER, st(AI_POWER));
	//������ �������
	if ((state.opr == OPR_START_TEST) && (st(DI_PC_HOT_TEST) == OFF)) {
#ifdef NO_FREQ_DRIVER
		set(AO_PC_ROTATE, st(AI_PC_ROTATE));
#else
		//set(AO_PC_ROTATE, st(AI_FC_FREQ));
		set(AO_PC_ROTATE, st(AI_ROTATION_SPEED));
#endif
	} else {
		//set(AO_PC_ROTATE, st(AI_ROTATION_ETCU));
		set(AO_PC_ROTATE, st(AI_ROTATION_SPEED));
	}
	//����������� ��������� �����
	set(AO_PC_T_EXHAUST, st(AI_T_EXHAUST));
	//set(AO_PC_T_EXHAUST, st(AI_VALVE_POSITION) * 1000); // ToDo ������ ���� ��
	//������� ����������� ��������� ToDo ������ ��
	set(AO_PC_T_COOLANT_IN, st(AI_T_COOLANT_IN));
	set(AO_PC_T_COOLANT_OUT, st(AI_T_COOLANT_OUT));
	set(AO_PC_T_OIL_IN, st(AI_T_OIL_IN));
	//set(AO_PC_T_OIL_OUT, st(AI_T_OIL_OUT));
	//set(AO_PC_T_FUEL, st(AI_T_FUEL));
	set(AO_PC_T_FUEL, st(AI_T_AIR));
	//������ �������
	val = (st(AP_FUEL_IN) - st(AP_FUEL_OUT)) * 60;
	set(AO_PC_FUEL_CONSUM, val);
	// ������� �������
#define LEVEL_MAX	100000
#define LEVEL_MIN	10000
#define LEVEL_ERR	125000
	val = (st(AI_T_FUEL) * 100000) / 92; // ������� ������� XP8
	if (val > LEVEL_MAX) {
		if (val < LEVEL_ERR) val = LEVEL_MAX;
		else val = ERROR_CODE;
	}
	set(AO_PC_FUEL_LEVEL, val); // ������� ������� XP8
	val = (st(AI_T_OIL_OUT) * 100000) / 92; // ������� ����� XP7
	if (val > LEVEL_MAX) {
		if (val < LEVEL_ERR) val = LEVEL_MAX;
		else val = ERROR_CODE;
	}
	if ((val < LEVEL_MIN) || (val == ERROR_CODE)) {
		error.bit.err_brake = 1; //������ ������������
		state.step = ST_STOP_ERR;
		cmd.opr = ST_STOP_ERR;
	}
	set(AO_PC_T_OIL_OUT, val); // ������� ����� XP7
	//������� ���������� �������
	set(AO_PC_T_EXT1, st(AI_T_EXT1));
	set(AO_PC_T_EXT2, st(AI_T_EXT2));
	set(AO_PC_T_EXT3, st(AI_T_EXT3));
	set(AO_PC_T_EXT4, st(AI_T_EXT4));
	set(AO_PC_T_EXT5, st(AI_T_EXT5));
	set(AO_PC_T_EXT6, st(AI_T_EXT6));
	set(AO_PC_T_EXT7, st(AI_T_EXT7));
	set(AO_PC_T_EXT8, st(AI_T_EXT8));
	//���������� �������
	set(AO_PC_P_EXHAUST, st(AI_P_EXHAUST)); //����������: �������� ��������� �����
	set(AO_PC_P_OIL, st(AI_P_OIL)); //����������: �������� �����
	//set(AO_PC_P_CHARGE, st(AI_P_CHARGE)); //����������: �������� ������������ �������
	//set(AO_PC_T_CHARGE, st(AI_T_CHARGE)); //����������: ����������� ������������ �������
	set(AO_PC_P_MANIFOLD, st(AI_P_MANIFOLD)); //����������: �������� ��������� ����������
	//��������� ���������
	set(AO_PC_T_ENV_AIR, st(AI_T_AIR));
	set(AO_PC_P_ENV_AIR, st(AI_P_AIR));
	set(AO_PC_H_ENV_AIR, st(AI_H_AIR));
	//extern int32_t ServoCount;
	//set(AO_PC_H_ENV_AIR, ServoCount * 1000); // ToDo!

	//��� � ���������� ���
	val=st(AI_I_AKB_P)-st(AI_I_AKB_N);
	set(AO_PC_CURRENT, val);
	set(AO_PC_VOLTAGE, st(AI_U_AKB));
	//������� ������ ����
#ifndef NO_SPSH_20
#if MIN_SERVO_POSITION < 0
	#define SERVO_MOVE (float32_t)(MIN_SERVO_POSITION - MAX_SERVO_POSITION)
#else
	#define SERVO_MOVE (float32_t)(MAX_SERVO_POSITION - MIN_SERVO_POSITION)
#endif
	float32_t fval = (float32_t)st(AI_SERVO_POSITION) / SERVO_MOVE * 100000.0f;
	val = (int32_t)fval;
	set(AO_PC_TROTTLE, val);		//����������: ��������� ������ ����
#else
	set(AO_PC_TROTTLE, st(AI_SERVO_POSITION));		//����������: ��������� ������ ����
#endif
	//������� ������������
	set(AO_PC_T_BRAKE,	st(AI_T_OIL_BRAKE)); //����������: ����������� � ������������
	set(AO_PC_P_BRAKE,	st(AI_P_OIL_BRAKE)); //����������: �������� � ������������
	set(AO_PC_SET_BRAKE, st(AO_HYDROSTATION));	//����������: ������������� �������� ������������ � �%
	//����� � ����
	set(AO_PC_TIME, rtc_sens_get_time());
	set(AO_PC_DATE, rtc_sens_get_date());
//---------------�������
	set(DO_PC_STARTER, st(DO_STARTER));					//������: �������
	set(DO_PC_COOLANT_FAN, st(DO_COOLANT_FAN));			//������: ���������� ��
	set(DO_PC_COOLANT_PUMP, st(DO_COOLANT_PUMP));		//������: ����� ��
	set(DO_PC_OIL_PUMP,	st(DO_OIL_PUMP));				//������: ����� �����
	set(DO_PC_COOLANT_HEATER, st(DO_COOLANT_HEATER));	//������: ����������� ��
	set(DO_PC_OIL_HEATER, st(DO_OIL_HEATER));			//������: ����������� �����
	set(DO_PC_FUEL_PUMP, st(DO_FUEL_PUMP));				//������: �������� ����
	set(DO_PC_BRAKE_FAIL, error.bit.err_brake);			//������: ������ ������������
	set(DO_PC_OIL_FAN, st(DO_OIL_FAN));     			//������: ���������� �����
//---------------�����c�
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

void work_stop (void) {
	set(DO_STARTER, OFF);
	set(AO_FC_FREQ, 0);
#ifdef NO_FREQ_DRIVER
	set(AI_PC_ROTATE, 0);
#endif
	uint32_t pwm1_out = st(AO_HYDROSTATION);
	uint32_t pwm2_out = st(AO_VALVE_ENABLE);
	if (pwm1_out > 10) { // ������� �������� ��������
		set(AO_HYDROSTATION, pwm1_out - 10);
	} else {
		set(AO_HYDROSTATION, 0);
		pwm1_out = 0;
	}
	if (pwm2_out > 10) { // ������� �������� ��������
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
	init_PID();
#ifndef NO_SPSH_20
	spsh20_set_pos(0);
#elif !NO_SERVO_DRIVER
	servo_set_out(0);
#endif
#ifdef MODEL_OBJ
	set(AI_FC_FREQ, 0);
	Speed_Out = Torque_Out = 0;
	Pwm1_Out = Pwm2_Out = 0;
	init_obj();
#endif
}

#ifdef MODEL_OBJ
/*
 * �������������� �������� ������������� (��)
 * FrequeObj: �������� �������� �� - ������� ���������.
 * ������� �������� ����������, ������� �� 60% ��������� ��������� �������
 * ������������ (+), �� 40% �� �������� ��������� ������� �� ���� ��������� (-).
 * TorqueObj: �������� �������� �� - �������� ������.
 * ������� ��������: ������������������ ������������ (+).
 */
void init_obj (void) {
	TorqueObj.st = FrequeObj.st = HAL_GetTick();
	FrequeObj.out = (float32_t)st(CFG_MIN_ROTATE) / 1000.0; // ����. ����-� ���. �����.
	FrequeObj.tau = 3000.0; // ���������� ������� ��������������� �����, ms
	TorqueObj.out = 0.0; // ����. ����-� ���. �����.
	TorqueObj.tau = 2000.0; // ���������� ������� ��������������� �����, ms
}

/*
 * output: float32_t ������� �� �� ������� �����������
 * inp ������� ����������� �� ��
 * obj ��������� �� ��
 */
float32_t get_obj (obj_t * obj, float32_t inp) {
	float_t dt, z0, z1, out;
	uint32_t time = HAL_GetTick();
	if (obj->st <= time) dt = (float32_t)(time - obj->st);
	else dt = (float32_t)((0xffffffff - obj->st)  + time);
	z0 = dt / (dt + obj->tau);
	z1 = 1 - z0;
	out = (z0 * inp) + (z1 * obj->out);
	obj->out = out;
	obj->st = time;
	return out;
}
#endif

#define PID_RESET				1
#define PID_NO_RESET			0
#ifdef MODEL_OBJ
	#define SPEED_KI			0.0009f
#else
	#define SPEED_KI			0.001f
#endif

/*
 * ������������� �������� �������������
 */
void init_PID (void) {
#ifdef MODEL_OBJ
	Speed_PID.Kp = 0.1;
	Speed_PID.Ki = SPEED_KI;
	Speed_PID.Kd = 0.00001;
	Torque_PID.Kp = 0.15;
	Torque_PID.Ki = 0.1;
	Torque_PID.Kd = 0.001;
#else
	Speed_PID.Kp = 0.30;
	Speed_PID.Ki = SPEED_KI;
	Speed_PID.Kd = 0.0001;
	Torque_PID.Kp = 0.10;
	Torque_PID.Ki = 0.01;
	Torque_PID.Kd = 0.0001;
#endif
	arm_pid_init_f32(&Speed_PID, PID_RESET);
	arm_pid_init_f32(&Torque_PID, PID_RESET);
}

#define SPEED_LOOP_TIME		100 // ������������� �� ������� ������� ������������� ��������, ��
#define TORQUE_MAX			600.0f // ��
#define SPEED_MAX			2000.0f
#define TORQUE_FACTOR		0.6f
#define SPEED_MUL			-40.00f
#define SPEED_FACT			50.0f
#define SERVO_FACT			1.20f
#define ZONE_DEAD_REF		50.0f
/*
 * ���������� �������� ��������
 */
void Speed_loop (void) {
	int32_t set_out; float32_t pi_out, task, torq_corr;
	if (timers_get_time_left(time.alg) == 0) { // ���������� �������� ��������
		time.alg = timers_get_finish_time(SPEED_LOOP_TIME);
		task = (float32_t)st(AI_PC_ROTATE) / 1000.0;
		task -= Speed_Out; // PID input Error
#ifndef NO_SERVO_DRIVER
		float32_t tmp = abs(task);
		if (servo_get_pos() >= 100.0) {
			Speed_PID.Ki = 0;
		} else {
			Speed_PID.Ki = SPEED_KI * exp(-tmp / SPEED_MAX);
		}
		arm_pid_init_f32(&Speed_PID, PID_NO_RESET);
		if (tmp < ZONE_DEAD_REF) task = 0;
#endif
		pi_out = arm_pid_f32(&Speed_PID, task);
#ifndef NO_SPSH_20
		set_out = (int32_t)(pi_out * SPEED_MUL);
		spsh20_set_pos(set_out);
#elif !NO_SERVO_DRIVER
		servo_set_out(pi_out * SERVO_FACT);
#endif
#ifdef MODEL_OBJ
		torq_corr = (Torque_Out / TORQUE_MAX) * TORQUE_FACTOR;
#ifndef NO_SERVO_DRIVER
		pi_out = servo_get_pos() * SPEED_FACT * (1 - torq_corr);
#endif
		Speed_Out = get_obj(&FrequeObj, pi_out);
#endif // MODEL_OBJ
	}
}

#define TORQUE_LOOP_TIME	200 // ������������� �� ������� ������� ��������� �������, ��
#define SPEED_DIFF			50
#define PWM_SCALE			100000U
#define PWM_FSCALE			100000.0f
#define PWM_PERCENT			(PWM_SCALE / 100000)
#define SCALE_DIV			4U // �������� ��������� ���������� ����� ������� � �������� 1:4
#define VALVE_DIV			8U
#define VALVE_NULL			(PWM_SCALE / VALVE_DIV) // ��������� ���� �������� ������� 20...30%
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
/*
 * ���������� �������� ��������� �������
 */
void Torque_loop (torq_val_t val) {
	uint32_t pwm_out, pwm1_out, pwm2_out;
	float32_t pwm_flo;
	if (timers_get_time_left(cntrl_M_time) == 0) { // ���������� �������� ��������� �������
		if (val == Percent) { // ���� �� ������������ ��� ��
			pwm1_out = st(AI_PC_TORQUE) & 0xFFFF; // ����� �����
			pwm2_out = st(AI_PC_TORQUE) >> 16; // ����� ������
#ifdef MODEL_OBJ
			pwm_out = (pwm1_out * 2) + (pwm2_out * 8); // ������������� ���������� 1:4
			Torque_Out = ((float32_t)pwm_out * PERCENT_FACTOR) * TFILTER_TAU + Torque_Out * (1.0 - TFILTER_TAU);
#endif
			pwm1_out = pwm1_out * (PWM_SCALE / 10000); // ����� �����
			if (pwm1_out > PWM_SCALE) pwm1_out = PWM_SCALE;
			pwm2_out = pwm2_out * (PWM_SCALE / 10000); // ����� ������
			if (pwm2_out > PWM_SCALE) pwm2_out = PWM_SCALE;
			pwm1_out = (uint32_t)((float32_t)pwm1_out * TFILTER_TAU + (float32_t)Pwm1_Out * (1.0 - TFILTER_TAU));
			pwm2_out = (uint32_t)((float32_t)pwm2_out * TFILTER_TAU + (float32_t)Pwm2_Out * (1.0 - TFILTER_TAU));
			Pwm1_Out = pwm1_out;
			Pwm2_Out = pwm2_out;
		} else { // val_type == Absolute
			float32_t pi_out, task;
			task = st(AI_PC_TORQUE) / 1000.0;
			task -= Torque_Out; // PID input Error
			pi_out = arm_pid_f32(&Torque_PID, task); // ToDo: ������������ ��. ������� ��
#ifdef MODEL_OBJ
			Torque_Out = get_obj(&TorqueObj, pi_out);
#endif
			if (pi_out < 0) pi_out = 0;
			pi_out *= TORQUE_SCALE;
			if (pi_out > PWM_FSCALE) pi_out = PWM_FSCALE;
			pwm_flo = (uint32_t)(PWM_P_MIN + pi_out * (PUMP_MAX - PUMP_MIN)); // ����� �����
			if (pwm_flo > PWM_P_MAX) pwm_flo = PWM_P_MAX;
			pwm1_out = (uint32_t)pwm_flo;
			pwm_flo = (PWM_V_MIN + pi_out * (VALVE_MAX - VALVE_MIN)); // ����� ������
			if (pwm_flo > PWM_V_MAX) pwm_flo = PWM_V_MAX;
			pwm2_out = (uint32_t)pwm_flo;
		}
		set(AO_HYDROSTATION, pwm1_out);
		set(AO_VALVE_ENABLE, pwm2_out);
		cntrl_M_time = timers_get_finish_time(TORQUE_LOOP_TIME);
	}
}
