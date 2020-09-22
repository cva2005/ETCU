#include "rs485_1.h"
#include "agm501.h"
#include "modbus.h"
#include "timers.h"

static uint8_t ChN, Addr, err_send, Cmd, ChS;
static uint16_t Status, Error, Mode;
static stime_t connect_time, tx_time;
agm_rx_t rx;
int32_t Res[AGM_CH]; //NO_Res, K_Res, NH_Res;

static void agm_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function) {
	if (adr != Addr) return;
	err_send = 0;
	if (function == MODBUS_READ_HOLDING_REGISTERS) {
		int32_t int_res;
		agm_rx_t *rx = (agm_rx_t *)data;
		for (unsigned i = 0; i < AGM_CH; i++) {
			float32_t f_res = 0;
			Res[i] = rx->res[i];
		}
		Status = rx->stat;
		Error = rx->err;
		Mode = rx->mode;
		Cmd = rx->cmd;
		ChS = rx->ch;
		tx_time = timers_get_finish_time(AGM_DATA_TX_TIME); // время отправки следующего пакета
		connect_time = timers_get_finish_time(AGM_CONNECT_TIME); // время ответа от slave устйройства
	}
}

void agm_init (uint8_t ch, uint8_t addr) {
	uint8_t cnt = 0;
	ChN = ch;
	if ((addr <= 247) && (addr > 0)) {
		while ((pf_rx[ch][cnt] != NULL) && (cnt < MODBUS2_MAX_DEV)) cnt++; //найти свободный указатель
		if (cnt < MODBUS2_MAX_DEV) {
			pf_rx[ch][cnt] = agm_update_data; // обработчик принятых пакетов
			err_send = 0; //счётчик пакетов без ответа
			Addr = addr; // сохранить адрес первого устройства
			tx_time = timers_get_finish_time(AGM_DATA_TX_TIME); // время отправки следующего пакета
			connect_time = timers_get_finish_time(AGM_CONNECT_TIME); // время ответа от slave устйройства
		}
	}
}

void agm_step (void) {
	if (timers_get_time_left(connect_time) == 0) {
		for (uint32_t i = 0; i < AGM_CH; i++) Res[i] = ERROR_CODE;
		connect_time = timers_get_finish_time(AGM_CONNECT_TIME);
	}
	if (timers_get_time_left(tx_time) == 0) {
		if (modbus_get_busy(ChN, Addr, Low_pr)) return; // интерфейс занят
		rs485_1_init(9600);
		if (modbus_rd_in_reg(ChN, Addr, FIRST_IN_REG, sizeof(agm_rx_t) / 2)) {
			tx_time = timers_get_finish_time(AGM_DATA_TX_TIME);
			connect_time = timers_get_finish_time(AGM_CONNECT_TIME);
			if (err_send < 0xFF) err_send++;
		}
	}
}

int32_t agm_read_res (uint8_t ch) {
	int32_t data = Res[ch];
	if ((ch < 3) || (ch > 10)) {
		data *= 1000;
	} else if (ch < 9) {
		data *= 10;
	}
	return data;
}

uint8_t agm_err_link (void) {
	if (timers_get_time_left(connect_time) == 0) return 1;
	if (err_send > AGM_MAX_ERR_SEND) return 1;
	return 0;
}
