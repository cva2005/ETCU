#ifndef APP_DEVICES_MV8A_H_
#define APP_DEVICES_MV8A_H_
#include "types.h"
#include "timers.h"
#include "_control.h"

#pragma pack(1)
typedef	struct {
	uint8_t status;
	uint8_t	time;
	uint8_t fdata[4];
} mv8a_rx_t;
#pragma pack()

#define MV8A_CONNECT_TIME	300	// ����� ������ ��� ����. ����� ��������� ����������
#define MV8A_DATA_TX_TIME	100	// ������� ����� ������������� ��������
#define MV8A_MAX_ERR_SEND	3	// ���������� ���������� �������� ��� ������
#define MV8A_NUM 			3 	// ���������� ������� MV110-8A
#define CH_NUM				8 	// ���������� ������� � MV110-8A
#define INP_NUM				20 	// ���������� ������������ ������� ���������

void mv8a_init (uint8_t ch, uint8_t addr);	// ������������� ����������
int32_t mv8a_read_res (uint8_t ch);	// ��������� ���������
uint8_t mv8a_err_link (void);	// ���������� ��������� ����� � MV8A
void mv8a_step (void);

/*
 * ������ ��������� MODBUS
 * ������ ����� 1		Int16 		0x0002
 * ����. ����� ���.		Int16 		0x0003
 * ��������� ����� 1	Float32 	0x0004, 0x0005
 * ������ ����� 2		Int16 		0x0008
 * ����. ����� ���.		Int16 		0x0009
 * ��������� ����� 2	Float32 	0x000A, 0x000B
 * ������ ����� 3		Int16 		0x000E
 * ����. ����� ���.		Int16 		0x000F
 * ��������� ����� 3	Float32 	0x0010, 0x0011
 * ������ ����� 4		Int16 		0x0014
 * ����. ����� ���.		Int16 		0x0015
 * ��������� ����� 4	Float32 	0x0016, 0x0017
 * ������ ����� 5		Int16 		0x001A
 * ����. ����� ���.		Int16 		0x001B
 * ��������� ����� 5	Float32 	0x001C, 0x001D
 * ������ ����� 6		Int16 		0x0020
 * ����. ����� ���.		Int16 		0x0021
 * ��������� ����� 6	Float32 	0x0022, 0x0023
 * ������ ����� 7		Int16 		0x0026
 * ����. ����� ���.		Int16 		0x0027
 * ��������� ����� 7	Float32 	0x0028, 0x0029
 * ������ ����� 8		Int16 		0x002C
 * ����. ����� ���.		Int16 		0x002D
 * ��������� ����� 8	Float32 	0x002E, 0x002F
 * 1. ��� �������� ������ ��� ������. �������� �����������
 * ��������� 03 ��� 04 (������ ������������ ��� �������).
 * 2. �� ����� �������� ��������������� �������� (��� Float32)
 * ������� ����� ���������� � �������� � ������� �������.
 */

#define	STATUS_REG	2
#define RESULT_REG	4
#define CH_REG_NUM	4 // ��������� ��� ������ ������
#define CH_OFFSET	6 // �������� ��������� �������

#endif /* APP_DEVICES_MV8A_H_ */