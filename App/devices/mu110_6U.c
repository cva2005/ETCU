#include <string.h>
#include "mu110_6U.h"
#include "modbus.h"
#include "timers.h"

static uint8_t ChN, mu6u_addr, mu6u_err_send;
stime_t mu6u_connect_time, mu6u_tx_time;
mu6u_tx_t tx;

static void mu6u_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function) {
	if (adr == mu6u_addr) mu6u_err_send = 0;
}

void mu6u_init (uint8_t ch, uint8_t addr) {
	uint8_t cnt = 0;
	ChN = ch;
	if ((addr <= 247) && (addr > 0)) {
		while ((pf_rx[ch][cnt] != NULL) && (cnt < MODBUS2_MAX_DEV)) cnt++; //найти свободный указатель
		if (cnt < MODBUS2_MAX_DEV) {
			pf_rx[ch][cnt] = mu6u_update_data; // обработчик принятых пакетов
			mu6u_err_send = 0; //счётчик пакетов без ответа
			mu6u_addr = addr; // сохранить адрес
			mu6u_tx_time = timers_get_finish_time(MU6U_DATA_TX_TIME); // время отправки следующего пакета
			mu6u_connect_time = timers_get_finish_time(MU6U_CONNECT_TIME); // время ответа от slave устйройства
		}
	}
}

void mu6u_step (void) {
	if (timers_get_time_left(mu6u_tx_time) == 0) {
		if (modbus_get_busy(ChN, mu6u_addr, Low_pr)) return; // интерфейс занят
		if (modbus_wr_mreg(ChN, mu6u_addr, DAC0_OUT, sizeof(tx) / 2, tx.byte)) {
			mu6u_tx_time = timers_get_finish_time(MU6U_DATA_TX_TIME);
			mu6u_connect_time = timers_get_finish_time(MU6U_CONNECT_TIME);
			if (mu6u_err_send < 0xFF) mu6u_err_send++;
		}
	}
}

void mu6u_set_out (uint16_t data) {
	tx.byte[1] = data & 0xff;
	tx.byte[0] = data >> 8;
	data /= 2;
	tx.byte[3] = data & 0xff;
	tx.byte[2] = data >> 8;
}

uint8_t mu6u_err_link (void) {
	if (timers_get_time_left(mu6u_connect_time) == 0) return 1;
	if (mu6u_err_send > MU6U_MAX_ERR_SEND) return 1;
	return 0;
}
