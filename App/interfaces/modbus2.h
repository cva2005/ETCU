/*
 * modbus2.h
 *
 *  Created on: 29 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_INTERFACES_MODBUS2_H_
#define APP_INTERFACES_MODBUS2_H_
#include "types.h"

#define MODBUS2_MAX_DEV 	1 //������������ ��������� ���������� ��������� ModBus
#define MB2_TX_BUFF			20
typedef void (*modbus2_rx_t) (char *data, uint8_t len, uint8_t adr, uint8_t function); //�������� ��������� - ����������� �������� ������ ModBus

#define MODBUS2_MAX_WAIT_TIME 500	//����������� ���������� ����� �������� ������ �� slave ����������
#define MODBUS2_BYTE_RX_PAUSE 5		//����������� ���������� ����� ����� ��������� �������

void modbus2_init(uint8_t chan);			//������������� ���������� ModBus
void modbus2_step(void);					//��� ������ ModBus: ���������� �������� � �������� ����
uint8_t modbus2_rd_in_reg(uint8_t adr, uint16_t reg, uint16_t number);	//�������: ��������� �������� �������� ������ ��� ���������� ��������� ��������
uint8_t modbus2_rd_hold_reg(uint8_t adr, uint16_t reg, uint16_t number);	//�������: ��������� �������� �������� ������ ��� ���������� ��������� ��������
uint8_t modbus2_wr_1reg(uint8_t adr, uint16_t reg, uint16_t val);			//�������: ������ ������ �������� � ������� ��������
uint8_t modbus2_user_function(uint8_t adr, uint8_t func, uint8_t ln, uint8_t *data);//�������: ���������������� �������
uint8_t modbus2_get_busy(uint8_t adr);							//���������� ���������� ModBus � �������� ������
uint8_t modbus2_wr_mreg(uint8_t adr, uint16_t reg, uint16_t num, uint8_t* data);
extern modbus2_rx_t modbus2_rx[MODBUS2_MAX_DEV]; //��������� �� ������� ����������� ������� �� modbus

typedef enum
{
	MODBUS2_READ_COIL_STATUS=1,
	MODBUS2_READ_INPUT_STATUS=2,
	MODBUS2_READ_HOLDING_REGISTERS=3,
	MODBUS2_READ_INPUTS_REGISTERS=4,
	MODBUS2_FORCE_SINGLE_COIL=5,
	MODBUS2_FORCE_SINGLE_REGISTER=6,
	MODBUS2_READ_EXCEPTION_STATUS=7,
	MODBUS2_LOOPBACK_DIAGNOSTIC_TEST=8,
	MODBUS2_FETCH_EVENT_COUNTER=11,
	MODBUS2_FETCH_COMMUNICATION=12,
	MODBUS2_PROGRAM=13,
	MODBUS2_POLL_PROGRAM_COMPLETE=14,
	MODBUS2_FORCE_MULTIPLE_COILS=15,
	MODBUS2_FORCE_MULTIPLE_REGISTERS=16,
	MODBUS2_REPORT_SLAVE=17,
	MODBUS2_RESET_COMMUNACATION_LINK=19
}modbus2_function_t;

#define GET2_UINT16(addr)      (((uint16_t)((uint8_t *)(addr))[0] << 8) | ((uint8_t *)(addr))[1])
#define SET2_UINT16(addr, val) do { ((uint8_t *)(addr))[0] = ((val) >> 8) & 0xFF; \
                                   ((uint8_t *)(addr))[1] = (val) & 0xFF; } while (0)
#define SWAP16_2(x)             ( (((x) & 0xFF) << 8) | (((x) >> 8) & 0xFF) )

#endif /* APP_INTERFACES_MODBUS_H_ */