/*
 * _control.h
 *
 *  Created on: 12 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP__CONTROL_H_
#define APP__CONTROL_H_
#include "_signals.h"

#define ERROR_CODE					0x7FFFFFFF
#define DEF_KEY_DELAY				500		//����������� �� ������
#define DEF_MAX_I_SERVO				5000	//������������ ��� ������������, ��
#define DEF_MAX_SERVO_POSITION		5000000	//������������ ��������� ������������,
#define DEF_MAX_ENGINE_SPEED		(735 * 1000) //������������ ������� �������������
#define DEF_U_AKB_NORMAL			20000	//����������� ����������� ��� ��� ������� ���������
#define DEF_STARTER_ON_TIME			10000	//����� ��������� ��������, ���� ��������� �� �����������
#define DEF_MIN_ROTATE				400000	//����������� ������� ��������� ��� ����������� ��� �������
#define DEF_ERR_ROTATE				200000	//����������� ������� ��������� ��� ����������� ������
#define DEF_MIN_I_STARTER			10000	//����������� ��� ��������
#define DEF_FUEL_PUMP_TIMEOUT		2000	//����� ������ ���� ����� �������� ��������
#define DEF_TIME_START_FC			5000	//������� �� ������ ��
#define DEF_HYDROSTATION_TETS		10000	//������� ������� ������� ��� �������� ������������
#define DEF_SPEED_TIME_HYDRO		10000	//����� ������ �� ������ �������� ������������
#define DEF_TIMOUT_ENGINE_ROTATE	20000 	//������� ������� ������ ����� �� �������� ������� ���������
#define DEF_TIMOUT_SET_TORQUE		20000 	//����� ��������� ��������� ��������� ������

#define OPR_STOP_TEST		0
#define OPR_START_TEST		1

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

/* ������ �������� � ������� */
#define NODE_ID_BCU 1	//����� CanOpen ������ ���������� �������������
#define NODE_ID_FC	2	//����� CanOpen/ModBus ��������������� �������
#define SPSH20_ADR	1	//����� ������������
#define ADR_NL_3DPAS 1	//����� ModBus ������� ���������� ��������� NL-3DPAS
#define ADR_T46 	1	//����� ModBus ������� ��������� �������
#define ADR_CDU		1	//����� CDUlink �������-���������� ����������

/* ������������ ���������� �������� ETCU */
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

void control_init (void); //������������� ������� ����������
void control_step (void); //1 ��� ��������� ����������
void signals_start_cfg (void);
void parametrs_start_cfg (void);
void sensors_config (void);
void read_devices (void);
void update_devices (void);
void read_keys (void);		//�������� �������� ������
void set_indication (void);	//�������� ��������� �����������
void work_step (void);		//��������� ��� ��������� ����������

uint8_t chek_out_val(int32_t val1, int32_t val2, int32_t delta);
void work_stop(void);

#ifdef MODEL_OBJ
typedef struct {
	uint32_t st; // ������ ������� �������������
	float32_t out; // ���������� �������� ��������� ��������� ����� 1
	float32_t tau; // ���������� ������� ��������������� ����� 1, ms
} obj_t;
#endif

typedef enum {
	Percent = 0,
	Absolute
} torq_val_t;

#endif /* APP__CONTROL_H_ */
