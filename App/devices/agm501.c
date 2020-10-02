#include <stdbool.h>
#include "rs485_1.h"
#include "agm501.h"
#include "modbus.h"
#include "timers.h"
#include "_signals.h"

static uint8_t ChN, Addr, err_send, Cmd, ChS;
static uint16_t Status, Error, Mode;
static stime_t tx_time;
static int16_t cmd_reg, mode_reg;
static int32_t Res[AGM_CH];

static void agm_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function) {
	if (adr != Addr) return;
	err_send = 0;
	if (function == MODBUS_READ_INPUTS_REGISTERS) {
		int32_t int_res;
		int16_t *dp = (int16_t *)data;
		for (unsigned i = 0; i < AGM_CH; i++) Res[i] = SWAP16(dp[i]);
		tx_time = timers_get_finish_time(AGM_DATA_TX_TIME); // время отправки следующего пакета
	}
}

void agm_init (uint8_t ch, uint8_t addr) {
	uint8_t cnt = 0;
	cmd_reg = mode_reg = 0;
	ChN = ch;
	if ((addr <= 247) && (addr > 0)) {
		while ((pf_rx[ch][cnt] != NULL) && (cnt < MODBUS2_MAX_DEV)) cnt++; //найти свободный указатель
		if (cnt < MODBUS2_MAX_DEV) {
			pf_rx[ch][cnt] = agm_update_data; // обработчик принятых пакетов
			err_send = 0; //счётчик пакетов без ответа
			Addr = addr; // сохранить адрес первого устройства
			tx_time = timers_get_finish_time(AGM_DATA_TX_TIME); // время отправки следующего пакета
		}
	}
}

void agm_step (void) {
	if (timers_get_time_left(tx_time) == 0) {
		if (modbus_get_busy(ChN, Addr, Low_pr)) return; // интерфейс занят
		rs485_1_reinit(9600);
		int16_t reg = st(AI_PC_GA_TASK) >> 16;
		if (reg != mode_reg) {
			if (modbus_wr_1reg(ChN, Addr, MODE_REG, reg)) {
				mode_reg = reg;
				goto tx_complete;
			}
		} else {
			reg = st(AI_PC_GA_TASK) & 0x0000ffff;
			if (reg != cmd_reg) {
				if (modbus_wr_1reg(ChN, Addr, CMD_REG, reg)) {
					cmd_reg = reg;
					goto tx_complete;
				}
			}
		}
		if (modbus_rd_in_reg(ChN, Addr, FIRST_IN_REG, AGM_CH)) {
			cmd_reg = 0; // send new task
tx_complete:
			tx_time = timers_get_finish_time(AGM_DATA_TX_TIME);
			if (err_send < AGM_MAX_ERR_SEND) err_send++;
		}
	}
}

int32_t agm_read_res (uint8_t ch) {
	int32_t data = Res[ch];
	if (((ch < 8) && (ch > 4)) || (ch > 15)) {
		data *= 1000;
	} else if (ch < 14) {
		data *= 10;
	}
	return data;
}

uint8_t agm_err_link (void) {
	if (err_send >= AGM_MAX_ERR_SEND) return 1;
	return 0;
}
