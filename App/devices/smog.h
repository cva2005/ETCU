#ifndef APP_DEVICES_SMOG_H_
#define APP_DEVICES_SMOG_H_
#include "types.h"
#include "timers.h"
#include "_control.h"


#define SMOG_CONNECT_TIME	4000 // ����� ������ ��� ����. ����� ��������� ����������
#define SMOG_DATA_TX_TIME	2000 // ������� ����� ������������� ��������
#define SMOG_MAX_ERR_SEND	3 // ���������� ���������� �������� ��� ������
#define SMOG_CH				4 // ������� ���������
#define N_BYTE				3 // ���� � ���� ������

#pragma pack(1)
typedef	struct {
	struct {
	    uint8_t Point:	3; // ��������� �������
	    uint8_t Unused:	4;
	    uint8_t Sign:	1; // ��� ����
	    uint8_t	Data[N_BYTE]; // ���������� �������� (����� �� 0 �� 9 ����������)
	} ch[SMOG_CH];
    uint16_t Empty[2];
    uint8_t Error;
    uint8_t Status;
} smog_rx_t;
#pragma pack()

void smog_init (uint8_t ch, uint8_t addr); // ������������� ����������
int32_t smog_get_N0_43 (void); // ��������� ��������� "��������, N0,43"
int32_t smog_get_NH (void);	// ��������� ��������� "��������, N�"
int32_t smog_get_K (void);	// ��������� ��������� "����������� ����������"
int32_t smog_get_T (void); // ��������� ��������� "�����������"
uint8_t smog_err_link (void); // ���������� ��������� �����
void smog_step (void);

/*
 * ������ ��������� MODBUS:
 * ��������, ����������� � ����������� ���������������
 * ����� �������� L = 430 ��, N[0 ... 100%]
 * N0,43		Int16 		0x0000
 * ����������� ���������� ��������� ������, K	[0 ... +Inf]
 * K			Int16 		0x0002
 * ��������, N� [0 ... 100%]
 * N			Int16 		0x0004
 * ������ ���	Int16 		0x000D
 * ������ ���	Int16 		0x000E
 */
#define N0_43_REG	0x0000 // �������� N0,43
#define K_REG		0x0002 // ����������� ����������
#define NH_REG		0x0004 // ��������, N�
#define ERR_REG		0x000D // ������ ���
#define STAT_REG	0x000E // ������ ���
#define REG_NUM		(STAT_REG + 1) // ���������� ��������� ��� ������ � SMOG

// �������� �������� ������� ��Ȼ:
#define MEAS_MODE	00 // ����� ���������
#define WARMING		04 // �������
#define CORRECTION	01 // ���� �������������

// �������� �������� ������� ��Ȼ:
#define LAMP_FAULT	01 // ���������� �����
#define T_SENS_ERR	02 // ���������� ������ �����������
#define P_SENS_ERR	03 // ���������� ������ ��������
#define COMM_ERR	08 // ������ ������

#endif /* APP_DEVICES_SMOG_H_ */