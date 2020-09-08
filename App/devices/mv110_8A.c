#include "mv110_8A.h"
#include "modbus2.h"
#include "timers.h"

uint8_t mv8a_addr, mv8a_err_send, inp;
stime_t mv8a_connect_time, mv8a_tx_time;
mv8a_rx_t rx;
int32_t Data[INP_NUM];

static void mv8a_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function) {

	if (adr != (mv8a_addr + (inp / CH_NUM))) return;
	mv8a_err_send = 0;
	if (function == MODBUS2_READ_HOLDING_REGISTERS) {
		int32_t int_res;
		mv8a_rx_t *rx = (mv8a_rx_t *)data;
		if (rx->status == 0) int_res = (int32_t)(rx->fres * 1000.0);
		else int_res = ERROR_CODE;
		Data[inp] = int_res;
		mv8a_tx_time = timers_get_finish_time(MV8A_DATA_TX_TIME); // время отправки следующего пакета
	}
}

void mv8a_init (uint8_t addr) {
	uint8_t cnt;
	if ((addr <= 247) && (addr > 0)) {
		while ((modbus2_rx[cnt] != NULL) && (cnt < MODBUS2_MAX_DEV)) cnt++; //найти свободный указатель
		if (cnt < MODBUS2_MAX_DEV) {
			modbus2_rx[cnt] = mv8a_update_data; // обработчик принятых пакетов
			mv8a_err_send = 0; //счётчик пакетов без ответа
			mv8a_addr = addr; // сохранить адрес первого устройства
			inp = 0;
			mv8a_tx_time = timers_get_finish_time(MV8A_DATA_TX_TIME); // время отправки следующего пакета
			mv8a_connect_time = timers_get_finish_time(MV8A_CONNECT_TIME); // время ответа от slave устйройства
		}
	}
}

void mv8a_step (void) {
	if (timers_get_time_left(mv8a_tx_time) == 0) {
		uint8_t addr = mv8a_addr + (inp / CH_NUM);
		if (modbus2_get_busy(addr)) return; // интерфейс занят
		uint8_t ch = inp % CH_NUM;
		uint8_t reg = STATUS_REG + (ch * CH_OFFSET);
		if (modbus2_rd_hold_reg(addr, reg, CH_REG_NUM)) {
			if (++inp == INP_NUM) inp = 0;
			mv8a_tx_time = timers_get_finish_time(MV8A_DATA_TX_TIME);
			mv8a_connect_time = timers_get_finish_time(MV8A_CONNECT_TIME);
			if (mv8a_err_send < 0xFF) mv8a_err_send++;
		}
	}
}

int32_t mv8a_read_res (uint8_t ch) {
	return Data[ch];
}

uint8_t mv8a_err_link (void) {
	if (timers_get_time_left(mv8a_connect_time) == 0) return 1;
	if (mv8a_err_send > MV8A_MAX_ERR_SEND) return 1;
	return 0;
}
