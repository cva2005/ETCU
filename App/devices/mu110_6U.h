#ifndef APP_DEVICES_MU6U_H_
#define APP_DEVICES_MU6U_H_
#include "types.h"
#include "timers.h"
#include "_control.h"

#pragma pack(1)
typedef union {
	uint8_t byte[4];
	uint16_t word[2];
} mu6u_tx_t;
#pragma pack()

#define MU6U_CONNECT_TIME	2000 // ����� ������ �� MU6U, ��� ����. ����� ��������� ����������
#define MU6U_DATA_TX_TIME	SPEED_LOOP_TIME //������� ����� ������������� �������� � MU6U
#define MU6U_MAX_ERR_SEND	3	// ���������� ���������� �������� ��� ������

void mu6u_init (uint8_t ch, uint8_t addr); //������������� ����������
void mu6u_set_out (uint16_t data);//������������� ������ DAC0, DAC1
uint8_t mu6u_err_link (void);	//���������� ��������� ����� � MU6U
void mu6u_step (void);

/*
 * ������ ��������� MODBUS
 */
#define	DAC0_OUT	0x0000
#define DAC1_OUT	0x0001

#endif /* APP_DEVICES_MU6U_H_ */