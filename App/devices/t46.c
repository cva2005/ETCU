#include "t46.h"
#include "timers.h"

t46_rx_data_t t46_rx_data; //������ ����������� �� ������� �������
uint8_t t46_err_send; //������� ������� ��� ������
uint8_t t46_adr;	  //����� ������� T46
stime_t t46_tx_time;  //����� ��������� �������� �������
uint8_t t46_tx_type;  //����� ���� ������: �������������� ��� ������ ����������
float32_t Speed_Out, Torque_Out;

/*
 * ������������� ������� �������
 */
void t46_init (uint8_t adr) {
	uint8_t cnt = 0;
	t46_err_send = 0;
	t46_adr = 0;
	t46_tx_type = START_MEASURING;
	if ((adr <= 247) && (adr > 0)) {
		while ((modbus2_rx[cnt] != NULL) && (cnt < MODBUS2_MAX_DEV)) cnt++; // ����� ��������� ���������
		if (cnt < MODBUS2_MAX_DEV) {
			modbus2_rx[cnt] = t46_update_data; 		 //������� ���������� �������� �������
			t46_adr = adr;					  		 //��������� �����
			t46_tx_time = timers_get_finish_time(0); //���������� ����� �������� ���������� ������
		}
	}
}



/**
  * @brief  ���������� ������� �������� �� ModBus
  *
  * @param  *data: ��������� �� ������ �������� �� ModBus
  * 		len: ����� �������� ������
  * 		adr: ����� ���������� �� �������� ������ ������
  * 		function: ����� �������
  */
void t46_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function) {
	if (adr == t46_adr) {
		t46_err_send = 0;
		if (function == READ_COMPLEX) {
			if (len == RES_REG_NUM * 2) {
				char *dp;
	            dp = (char *)&Speed_Out;
	            *dp++ = data[5];
	            *dp++ = data[4];
	            *dp++ = data[7];
	            *dp++ = data[6];
				t46_rx_data.freq = (int32_t)(Speed_Out * 1000);
	            dp = (char *)&Torque_Out;
	            *dp++ = data[1];
	            *dp++ = data[0];
	            *dp++ = data[3];
	            *dp++ = data[2];
				t46_rx_data.torque = (int32_t)(Torque_Out * 1000);
				//t46_rx_data.temperature = (int32_t)(*((int16_t *)&data[8]) * 100);
				t46_rx_data.power = (uint32_t)(Speed_Out * Torque_Out / POWER_FACTOR);
			}
		} else if (function == START_MEASURING) { // ������ ���������
			t46_tx_time = timers_get_finish_time(T46_DATA_CONVERT_TIME);
			t46_tx_type = READ_COMPLEX;
		}
	}
}

/*
 * ��� ��������� ������ �������
 */
void t46_step (void) {
	if (timers_get_time_left(t46_tx_time) == 0) { // ����� �������� ������
		//���� �������� ����� � �������� ������, �� �������������������� ���
		if (t46_err_send > T46_MAX_ERR_SEND) t46_tx_type = START_MEASURING;
		if (modbus2_get_busy(t46_adr)) return; // ��������� �����
		if (t46_tx_type == START_MEASURING) {
			if (modbus2_wr_1reg(t46_adr, CONFIG_ADDR, CONFIG_WORD)) goto tx_compl;
		} else if (t46_tx_type == READ_COMPLEX) {
			if (modbus2_rd_in_reg(t46_adr, RESULT_ALL, RES_REG_NUM)) {
tx_compl:
				t46_tx_time = timers_get_finish_time(T46_DATA_REINIT_TIME + MODBUS2_MAX_WAIT_TIME);
				if (t46_err_send < 0xFF) t46_err_send++;
			}
		}
	}
}

/*
 * ���������� ��������� �����
 */
uint8_t t46_err_link (void) {
	if (t46_err_send > T46_MAX_ERR_SEND) return 1;
	else return 0;
}

/*
 * �������� ������, (��) * 1000
 */
int32_t t46_get_torque (void) {
	return t46_rx_data.torque;
}

#if 0
/*
 * �����������:�C (�C*1000)
 */
int32_t t46_get_temperature (void) {
	return t46_rx_data.temperature;
}
#endif

/*
 * �������: (��/���) * 1000
 */
int32_t t46_get_freq (void) {
	return t46_rx_data.freq;
}

/*
 * ��������: �� * 1000
 */
int32_t t46_get_power (void) {
	return t46_rx_data.power;
}