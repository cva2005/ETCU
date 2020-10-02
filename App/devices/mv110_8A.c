#include "mv110_8A.h"
#include "modbus.h"
#include "timers.h"

static uint8_t ChN, mv8a_addr, mv8a_err_send, inp;
stime_t mv8a_tx_time;
mv8a_rx_t rx;
int32_t Data[INP_NUM];

static void mv8a_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function) {

	if (adr != (mv8a_addr + (inp / CH_NUM))) return;
	mv8a_err_send = 0;
	if (function == MODBUS_READ_HOLDING_REGISTERS) {
		int32_t int_res; float32_t f_res;
		mv8a_rx_t *rx = (mv8a_rx_t *)data;
		int8_t *dp = (int8_t *)&f_res;
        *dp++ = rx->fdata[1];
        *dp++ = rx->fdata[0];
        *dp++ = rx->fdata[3];
        *dp++ = rx->fdata[2];
		if (rx->status == 0) int_res = (int32_t)(f_res * 1000.0);
		else int_res = ERROR_CODE;
		Data[inp] = int_res;
		if (++inp == INP_NUM) inp = 0;
		mv8a_tx_time = timers_get_finish_time(MV8A_DATA_TX_TIME); // время отправки следующего пакета
	}
}

void mv8a_init (uint8_t ch, uint8_t addr) {
	uint8_t cnt = 0;
	ChN = ch;
	if ((addr <= 247) && (addr > 0)) {
		while ((pf_rx[ch][cnt] != NULL) && (cnt < MODBUS2_MAX_DEV)) cnt++; //найти свободный указатель
		if (cnt < MODBUS2_MAX_DEV) {
			pf_rx[ch][cnt] = mv8a_update_data; // обработчик принятых пакетов
			mv8a_err_send = 0; //счётчик пакетов без ответа
			mv8a_addr = addr; // сохранить адрес первого устройства
			inp = 0;
			mv8a_tx_time = timers_get_finish_time(MV8A_DATA_TX_TIME); // время отправки следующего пакета
		}
	}
}

void mv8a_step (void) {
	if (timers_get_time_left(mv8a_tx_time) == 0) {
		uint8_t addr = mv8a_addr + (inp / CH_NUM);
		if (modbus_get_busy(ChN, addr, Hi_pr)) return; // интерфейс занят
		uint8_t ch = inp % CH_NUM;
		uint8_t reg = STATUS_REG + (ch * CH_OFFSET);
		if (modbus_rd_hold_reg(ChN, addr, reg, CH_REG_NUM)) {
			mv8a_tx_time = timers_get_finish_time(MV8A_DATA_TX_TIME);
			if (mv8a_err_send <= MV8A_MAX_ERR_SEND) mv8a_err_send++;
		}
	}
}

int32_t mv8a_read_res (uint8_t ch) {
	return Data[ch];
}

uint8_t mv8a_err_link (void) {
	if (mv8a_err_send >= MV8A_MAX_ERR_SEND) return 1;
	return 0;
}
