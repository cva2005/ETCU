#ifndef APP_DEVICES_T46_H_
#define APP_DEVICES_T46_H_
#include "arm_math.h"
#include "modbus2.h"

#define T46_DATA_CONVERT_TIME	100	// ����� ��� ��������� ����������
#define T46_DATA_REINIT_TIME	100 // ����� ����� ������� ���������� � ��������� ������������ �� ��������������
#define T46_MAX_ERR_SEND		2	// ����������� ���������� ���������� �������� ��� ������
#define RESULT_ALL		0 // 5 ���������: ������, �������, �����������
//#define RES_REG_NUM		5 // 5 ���������: ������, �������, �����������
#define RES_REG_NUM		4 // 4 ��������: ������, �������
#define CONFIG_ADDR		0 // ����� �������� "����� ������������"
#define CONFIG_WORD		0x0009 // ����� � ���������� ������: StartStop + UsingFloat
#define POWER_FACTOR	9.549 // ����������� ��� ���������� ��������

void t46_init (uint8_t adr);		// ������������� �������
void t46_step (void);				// ��� ��������� ������ �������
uint8_t t46_err_link (void);		// ��������� �����
int32_t t46_get_torque (void);		// �������� ��������� �������
int32_t t46_get_temperature (void);	// �������� �����������
int32_t t46_get_freq (void);		// �������� ��������
int32_t t46_get_power (void);		// �������� ��������
void t46_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function); // ���������� ������� �������� �� ModBus
extern float32_t Speed_Out, Torque_Out;

typedef struct {
	int32_t torque;
	int32_t freq;
	int32_t temperature;
	int32_t power;
} t46_rx_data_t;

typedef enum {
	READ_COMPLEX = MODBUS2_READ_INPUTS_REGISTERS,
	START_MEASURING = MODBUS2_FORCE_SINGLE_REGISTER,
} t46_modbus_function_t;

uint8_t t46_start_measure(void);

#endif /* APP_DEVICES_T46_H_ */