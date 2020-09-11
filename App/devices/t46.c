#include "t46.h"
#include "timers.h"

t46_rx_data_t t46_rx_data; //данные принимаемые от датчика момента
uint8_t t46_err_send; //счётчик пакетов без ответа
static uint8_t ChN, t46_adr; //канал, адрес датчика T46
stime_t t46_tx_time;  //время следующей отправки пакетов
uint8_t t46_tx_type;  //номер шага обмена: преобразование или чтения параметров
float32_t Speed_Out, Torque_Out;

/*
 * Инициализация датчика момента
 */
void t46_init (uint8_t ch, uint8_t adr) {
	uint8_t cnt = 0;
	ChN = ch;
	t46_err_send = 0;
	t46_adr = 0;
	t46_tx_type = START_MEASURING;
	if ((adr <= 247) && (adr > 0)) {
		while ((pf_rx[ch][cnt] != NULL) && (cnt < MODBUS2_MAX_DEV)) cnt++; // найти свободный указатель
		if (cnt < MODBUS2_MAX_DEV) {
			pf_rx[ch][cnt] = t46_update_data; 		 //указать обработчик принатых пакетов
			t46_adr = adr;					  		 //сохранить адрес
			t46_tx_time = timers_get_finish_time(0); //установить время отправки следующего пакета
		}
	}
}



/**
  * @brief  Обработчик пакетов принятых по ModBus
  *
  * @param  *data: указатель на данные принятые по ModBus
  * 		len: длина принятых данных
  * 		adr: Адрес устйроства от которого пришли данные
  * 		function: номер функции
  */
void t46_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function) {
	if (adr == t46_adr) {
		t46_err_send = 0;
		if (function == READ_COMPLEX) {
			if (len == RES_REG_NUM * 2) {
				char *dp;
#ifdef T46_DEBUG
				float32_t speed, torque;
	            dp = (char *)&speed;
#else
	            dp = (char *)&Speed_Out;
#endif
	            *dp++ = data[5];
	            *dp++ = data[4];
	            *dp++ = data[7];
	            *dp++ = data[6];
#ifdef T46_DEBUG
	            dp = (char *)&torque;
#else
	            dp = (char *)&Torque_Out;
#endif
	            *dp++ = data[1];
	            *dp++ = data[0];
	            *dp++ = data[3];
	            *dp++ = data[2];
#ifdef T46_DEBUG
				t46_rx_data.freq = (int32_t)(speed * 1000);
				t46_rx_data.torque = (int32_t)(torque * 1000);
				t46_rx_data.power = (uint32_t)(speed * torque / POWER_FACTOR);
#else
				t46_rx_data.freq = (int32_t)(Speed_Out * 1000);
				t46_rx_data.torque = (int32_t)(Torque_Out * 1000);
				//t46_rx_data.temperature = (int32_t)(*((int16_t *)&data[8]) * 100);
				t46_rx_data.power = (uint32_t)(Speed_Out * Torque_Out / POWER_FACTOR);
#endif
			}
		} else if (function == START_MEASURING) { // запуск измерений
			t46_tx_time = timers_get_finish_time(T46_DATA_CONVERT_TIME);
			t46_tx_type = READ_COMPLEX;
		}
	}
}

/*
 * Шаг обработки данных датчика
 */
void t46_step (void) {
	if (timers_get_time_left(t46_tx_time) == 0) { // время отправки пакета
		//если потеряна связь с датчиком моента, то переинициализировать его
		if (t46_err_send > T46_MAX_ERR_SEND) t46_tx_type = START_MEASURING;
		if (modbus_get_busy(ChN, t46_adr, Hi_pr)) return; // интерфейс занят
		if (t46_tx_type == START_MEASURING) {
			if (modbus_wr_1reg(ChN, t46_adr, CONFIG_ADDR, CONFIG_WORD)) goto tx_compl;
		} else if (t46_tx_type == READ_COMPLEX) {
			if (modbus_rd_in_reg(ChN, t46_adr, RESULT_ALL, RES_REG_NUM)) {
tx_compl:
				t46_tx_time = timers_get_finish_time(T46_DATA_REINIT_TIME + MODBUS_MAX_WAIT_TIME);
				if (t46_err_send < 0xFF) t46_err_send++;
			}
		}
	}
}

/*
 * Возвращает состояние связи
 */
uint8_t t46_err_link (void) {
	if (t46_err_send > T46_MAX_ERR_SEND) return 1;
	else return 0;
}

/*
 * Кртуящий момент, (Нм) * 1000
 */
int32_t t46_get_torque (void) {
	return t46_rx_data.torque;
}

#if 0
/*
 * Температура:м°C (°C*1000)
 */
int32_t t46_get_temperature (void) {
	return t46_rx_data.temperature;
}
#endif

/*
 * Частота: (об/мин) * 1000
 */
int32_t t46_get_freq (void) {
	return t46_rx_data.freq;
}

/*
 * Мощность: Вт * 1000
 */
int32_t t46_get_power (void) {
	return t46_rx_data.power;
}
