/*
 * atv61.h
 *
 *  Created on: 4 ����. 2016 �.
 *      Author: ������ �.�.
 */
/*
 * ��������� ��������������� Altivar 61/71
 */
#ifndef APP_DEVICES_ATV61_H_
#define APP_DEVICES_ATV61_H_
#include "types.h"

#define ATV61_CONNECT_TIME 2000	//������������ ���������� ���� �������� ������� �� ATV61, ��� ���������� ��������� ����� � ATV61 ����������
#define ATV61_DATA_TX_TIME 200	//������� ����� ������������� �������� � ATV61
#define ATV61_DATA_TX_INIT 200	//������� ����� ������������� �������� �������������
#define ATV61_MAX_ERR_SEND	2	// ����������� ���������� ���������� �������� ��� ������

#define ATV61_STOP 0
#define ATV61_TURN_LEFT 1
#define ATV61_TURN_RIGHT 2

#pragma pack(1)
typedef union
	{
	struct
		{
		uint8_t switch_on:1;	//Switch On
		uint8_t en_volage:1;	//���������� ����������
		uint8_t quick_stop:1;	//������� ���������
		uint8_t enable_opr:1;	//Enable operation
		uint8_t reserv1:3;		//
		uint8_t fault_reset:1;	//C���� ������
		uint8_t halt:1;			//
		uint8_t reserv2:2;		//
		uint8_t revers:1;		//�������� � �������� �������
		uint8_t reserv3:4;		//
		}cmd; //��������� �����
	struct
		{
		uint8_t rdy_switch:1;	//���������� �������
		uint8_t switch_on:1;	//���������� ������� � �������
		uint8_t opr_en:1;		//������ ��������
		uint8_t fault:1;		//���� ������� �������
		uint8_t voltage_en:1;	//���������� ����������
		uint8_t quick_stop:1;	//������� ���������
		uint8_t switch_on_dsb:1;//���������� ��������� �������
		uint8_t warning:1;		//�������������� ������� �������
		uint8_t reserv1:1;		//
		uint8_t remote:1;		//���������� �� ����
		uint8_t target_reached:1;//������� ����������
		uint8_t internal_limit:1;//������� ��� �������� ��������
		uint8_t reserv2:2;		//
		uint8_t stop_key:1;		//��������� �� ������� ����
		uint8_t revers:1;		//����������� �������� ��������� �����
		}eta; //����� ���������
	uint8_t byte[2];
	uint16_t word;
	}atv_cmd_st_t;

typedef union
	{
	struct
		{
		atv_cmd_st_t cmd_st;
		uint16_t frequency;
		}f;
	uint8_t byte[4];
	uint16_t word[2];
	}atv_pdo1_t; //��������� ������������� ������
#pragma pack()

void atv61_init(uint8_t node_id);			//������������� ���������� ATV61
static void atv61_cmd_prepare(void);		//�������������� ��������� ����� ��� ATV61 � ����������� �� ������� ������������ � ��������� �������
void atv61_step(void);					//��� ��������� ������ ���������� ATV61
void atv61_set_frequency(int32_t data);		//������������� ������� ATV61
int32_t atv61_get_frequency(void);		//���������� �������� ������� ATV61
uint8_t atv61_error (void);			//���������� ��������� ������ ��
uint8_t atv61_err_link (void);			//���������� ��������� ����� � ATV61
#ifdef ATV61_MB
void atv61_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function);
#else
void atv61_update_data (char *data, uint8_t len, uint32_t adr);	//���������� ������� �������� �� CanOpen �� ATV61
#endif

/*
 * ������ ��������� MODBUS
 */

#define MB_Status_word1		3201 // R-O
#define MB_Output_frequency	3202 // R-O
#define MB_Output_torque	3205 // R-O
#define MB_Control_word1	8501 // R/W
#define MB_Frequency_ref	8502 // R/W ( 0.1 Hz INT 16 signed bits)
#define MB_Control_wordExt	8504 // R/W
#define MB_Control_word2	8601 // R/W
#define MB_Speed_ref		8602 // R/W (rpm INT 16 signed bits)
#define MB_Status_word2		8603 // R-O
#define MB_Output_speed		8604 // R-O

#endif /* APP_DEVICES_ATV61_H_ */