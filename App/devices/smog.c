#include "rs485_1.h"
#include "smog.h"
#include "modbus.h"
#include "timers.h"

static uint8_t ChN, Addr, err_send;
static uint16_t stat, err_reg;
static stime_t connect_time, tx_time;
smog_rx_t rx;
int32_t Res[SMOG_CH]; //NO_Res, K_Res, NH_Res;

uint8_t BCDtoBIN(uint8_t c) {
	uint8_t tmp;
    tmp = (c >> 4) << 1;
    return (c - (tmp + tmp + tmp));
}

static void smog_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function) {

	if (adr != Addr) return;
	err_send = 0;
	if (function == MODBUS_READ_HOLDING_REGISTERS) {
		int32_t int_res;
		smog_rx_t *rx = (smog_rx_t *)data;
		for (unsigned i = 0; i < SMOG_CH; i++) {
			float32_t f_res = 0;
			for (unsigned j = 0; j < N_BYTE; j++) {
				f_res *= 100;
				f_res += BCDtoBIN(rx->ch[i].Data[j]);
			}
			uint8_t div = rx->ch[i].Point;
			while (div--) f_res /= 10;
			if (rx->ch[i].Sign) f_res *= -1.0;
			Res[i] = (int32_t)(f_res * 1000.0);

		}
		stat = rx->Status;
		err_reg = rx->Error;
		tx_time = timers_get_finish_time(SMOG_DATA_TX_TIME); // время отправки следующего пакета
		connect_time = timers_get_finish_time(SMOG_CONNECT_TIME); // время ответа от slave устйройства
	}
}

void smog_init (uint8_t ch, uint8_t addr) {
	uint8_t cnt = 0;
	ChN = ch;
	if ((addr <= 247) && (addr > 0)) {
		while ((pf_rx[ch][cnt] != NULL) && (cnt < MODBUS2_MAX_DEV)) cnt++; //найти свободный указатель
		if (cnt < MODBUS2_MAX_DEV) {
			pf_rx[ch][cnt] = smog_update_data; // обработчик принятых пакетов
			err_send = 0; //счётчик пакетов без ответа
			Addr = addr; // сохранить адрес первого устройства
			tx_time = timers_get_finish_time(SMOG_DATA_TX_TIME); // время отправки следующего пакета
			connect_time = timers_get_finish_time(SMOG_CONNECT_TIME); // время ответа от slave устйройства
		}
	}
}

void smog_step (void) {
	if (timers_get_time_left(connect_time) == 0) {
		for (uint32_t i = 0; i < SMOG_CH; i++) Res[i] = ERROR_CODE;
		connect_time = timers_get_finish_time(SMOG_CONNECT_TIME);
	}
	if (timers_get_time_left(tx_time) == 0) {
		if (modbus_get_busy(ChN, Addr, Hi_pr)) return; // интерфейс занят
		rs485_1_reinit(4800);
		if (modbus_rd_hold_reg(ChN, Addr, N0_43_REG, REG_NUM)) {
			tx_time = timers_get_finish_time(SMOG_DATA_TX_TIME);
			connect_time = timers_get_finish_time(SMOG_CONNECT_TIME);
			if (err_send < 0xFF) err_send++;
		}
	}
}

int32_t smog_get_N0_43 (void) { // результат измерения "Дымность, N0,43"
	return Res[0];
}

int32_t smog_get_K (void) { // результат измерения "Коэффициент поглощения"
	return Res[1];
}

int32_t smog_get_NH (void) { // результат измерения "Дымность, NН"
	return Res[2];
}

int32_t smog_get_T (void) { // результат измерения "Температура"
	return Res[3];
}

uint8_t smog_err_link (void) {
	if (timers_get_time_left(connect_time) == 0) return 1;
	if (err_send > SMOG_MAX_ERR_SEND) return 1;
	return 0;
}
