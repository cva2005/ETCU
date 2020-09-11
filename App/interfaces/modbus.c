#include "modbus.h"
#include "rs485_1.h"
#include "rs485_2.h"
#include "timers.h"
#include "crc.h"
#include <string.h>

rx_data_t f_rx_data[MB_NET] = {rs485_1_read_rx_data, rs485_2_read_rx_data};
rx_size_t f_rx_size[MB_NET] = {rs485_1_get_rx_size, rs485_2_get_rx_size};
tx_data_t f_tx_data[MB_NET] = {rs485_1_write_tx_data, rs485_2_write_tx_data};
modbus_rx_t mb1_rx[MODBUS_MAX_DEV]; //указатели на функции обработчики пакетов от устйроств ModBus
modbus_rx_t mb2_rx[MODBUS_MAX_DEV]; //указатели на функции обработчики пакетов от устйроств ModBus
modbus_rx_t* pf_rx[MB_NET] = {mb1_rx, mb2_rx}; //указатели на указатели на функций
stime_t modbus_tx_time[MB_NET]; //время когда разрешен следующий приём
stime_t modbus_rx_time[MB_NET]; //врея когда должен быть принят следующий байт
uint8_t modbus_busy[MB_NET];		//состояние интерфейса ModBus: 0-свободен, 1-ожидание пакетв ответа на запрос
uint16_t modbus_rx_size[MB_NET]; //размер принятых данных
uint8_t modbus_buf[MB_NET][MB_BL];
uint8_t modbus_rq_tx[MB_NET][MODBUS_MAX_DEV]; //таблица запросов tx
uint8_t modbus_rq_point[MB_NET]; //указатель в таблице запросов на устйроство с максимальным приоритетом отправки: этот мехнизм нужен чтобы гарантировать всем устйроства на шине модбас равноценный доступ к процедуре отправки
uint8_t modbus_tx (uint8_t ch, uint8_t *data, uint16_t len); //Отправляет пакет по интерфейсу ModBus

void modbus_init (void) {
	uint8_t cnt, ch;
	for (ch = 0; ch < MB_NET; ch++) {
		for (cnt = 0; cnt < MODBUS_MAX_DEV; cnt++) {
			pf_rx[ch][cnt] = NULL;
			modbus_rq_tx[ch][cnt] = 0;
		}
		modbus_busy[ch] = 0;
		modbus_tx_time[ch] = timers_get_finish_time(0);
		modbus_rx_time[ch] = timers_get_finish_time(0);
		modbus_rx_size[ch] = 0;
		modbus_rq_point[ch] = 0;
	}
}

void modbus_step (void) {
	uint32_t rx_size;
	uint16_t crc_rx, crc_calc;
	uint8_t cnt, ch;
	for (ch = 0; ch < MB_NET; ch++) {
		if (timers_get_time_left(modbus_tx_time[ch]) == 0)
			modbus_busy[ch] = 0; //если не дождались ответа, то освободить  интерфейс
		rx_size = f_rx_size[ch](); //прочитать кол-во принятых байт
		if (rx_size != modbus_rx_size[ch]) { //если количество принятых байт увеличилось
			modbus_rx_size[ch] = rx_size; //обновить кол-во принятых байт
			modbus_rx_time[ch] = timers_get_finish_time(MODBUS_BYTE_RX_PAUSE);	//установить максимально допустимое время приёма следующего байта
			continue;
		}
		if (rx_size != 0) { //если есть принятые байты
			if (timers_get_time_left(modbus_rx_time[ch]) == 0) { //если истекло время ожидания следующего байта: приём закончен
				f_rx_data[ch](modbus_buf[ch], rx_size);	//прочитать все принятые байты из буфера
				crc_rx = ((uint16_t)modbus_buf[ch][rx_size - 2] << 8)|modbus_buf[ch][rx_size - 1];		//прочитать CRC принятого пакета
				crc_calc = modbus_crc_rtu(modbus_buf[ch], rx_size - 2);	//расчитать CRC принятого пакета
				if (crc_rx == crc_calc) {//если CRC cовпадают
					modbus_busy[ch] = 0; //если пришёл ответ, то освободить  интерфейс
					for (cnt = 0; cnt < MODBUS_MAX_DEV; cnt++) {//найти обработчики принятых пакетов
						if (pf_rx[ch][cnt] != NULL) //вызвать обработчик принятых пакетов
							pf_rx[ch][cnt](&modbus_buf[ch][3], modbus_buf[ch][2], modbus_buf[ch][0], modbus_buf[ch][1]);
					}
				}
			}
		}
	}
}

/**
  * @brief  Функция: Получение текущего значения одного или нескольких входных регистров
  *
  * @param  adr: адрес устйроства
  * 		reg: номер запрашиваемого регистра
  * 		number: количество запрашиваемых регисторв
  *
  * @retval результат запроса: 1 - успешно, 0 - запрос не отправлен
  */
uint8_t modbus_rd_in_reg (uint8_t ch, uint8_t adr, uint16_t reg, uint16_t number) {
	uint8_t pack[8], st; udata16_t crc;
	if (modbus_busy[ch]) return(0);
	pack[0] = adr;
	pack[1] = MODBUS_READ_INPUTS_REGISTERS;
	pack[2] = reg >> 8;
	pack[3] = reg;
	pack[4] = number >> 8;
	pack[5] = number;
	crc.word = modbus_crc_rtu(pack, 6);
	pack[6] = crc.byte[1];
	pack[7] = crc.byte[0];
	if (modbus_tx(ch, pack, sizeof(pack))) {
		if (adr > 0) adr--;
		modbus_rq_tx[ch][adr] = 0;
		modbus_rq_point[ch]++;
		if (modbus_rq_point[ch] >= MODBUS_MAX_DEV) modbus_rq_point[ch] = 0;
		return (1);
	}
	return(0);
}

/**
  * @brief  Функция: Получение текущего значения одного или нескольких регистров хранения
  *
  * @param  adr: адрес устйроства
  * 		reg: номер запрашиваемого регистра
  * 		number: количество запрашиваемых регисторв
  *
  * @retval результат запроса: 1 - успешно, 0 - запрос не отправлен
  */
uint8_t modbus_rd_hold_reg (uint8_t ch, uint8_t adr, uint16_t reg, uint16_t number) {
	uint8_t pack[8]; udata16_t crc;
	if (modbus_busy[ch]) return(0);
	pack[0] = adr;
	pack[1] = MODBUS_READ_HOLDING_REGISTERS;
	pack[2] = reg >> 8;
	pack[3] = reg;
	pack[4] = number >> 8;
	pack[5] = number;
	crc.word = modbus_crc_rtu(pack, 6);
	pack[6] = crc.byte[1];
	pack[7] = crc.byte[0];
	if (modbus_tx(ch, pack, sizeof(pack))) {
		if (adr > 0) adr--;
		modbus_rq_tx[ch][adr] = 0;
		modbus_rq_point[ch]++;
		if (modbus_rq_point[ch] >= MODBUS_MAX_DEV) modbus_rq_point[ch] = 0;
		return(1);
	}
	return(0);
}

/**
  * @brief  Функция: Запись нового значения в регистр хранения
  *
  * @param  adr: адрес устйроства
  * 		reg: номер  регистра
  * 		val: записываемое значение
  *
  * @retval результат запроса: 1 - успешно, 0 - запрос не отправлен
  */
uint8_t modbus_wr_1reg (uint8_t ch, uint8_t adr, uint16_t reg, uint16_t val) {
	uint8_t pack[8]; udata16_t crc;
	if (modbus_busy[ch]) return(0);
	pack[0] = adr;
	pack[1] = MODBUS_FORCE_SINGLE_REGISTER;
	pack[2] = reg >> 8;
	pack[3] = reg;
	pack[4] = val >> 8;
	pack[5] = val;
	crc.word = modbus_crc_rtu(pack, 6);
	pack[6] = crc.byte[1];
	pack[7] = crc.byte[0];
	if (modbus_tx(ch, pack, sizeof(pack))) {
		if (adr > 0) adr--;
		modbus_rq_tx[ch][adr] = 0;
		modbus_rq_point[ch]++;
		if (modbus_rq_point[ch] >= MODBUS_MAX_DEV) modbus_rq_point[ch] = 0;
		return (1);
	}
	return(0);
}


/**
  * @brief  Функция: Пользовательская функция
  *
  * @param  adr: адрес устйроства
  * 		func: номер функции
  * 		ln: длина данных
  * 		*data: указатель данных функции
  *
  * @retval результат запроса: 1 - успешно, 0 - запрос не отправлен
  */
uint8_t modbus_user_function(uint8_t ch, uint8_t adr, uint8_t func, uint8_t ln, uint8_t *data) {
	uint8_t pack[255]; udata16_t crc;

	if (modbus_busy[ch]) return(0);
	if (ln > (sizeof(pack) - sizeof(adr) - sizeof(func) - sizeof(ln))) return(0);
	pack[0] = adr;
	pack[1] = func;
	pack[2] = ln;
	memcpy(&pack[3], data, ln);
	crc.word=modbus_crc_rtu(pack, ln+sizeof(adr)+sizeof(func)+sizeof(ln));
	pack[3 + ln] = crc.byte[1];
	pack[4 + ln] = crc.byte[0];
	if (modbus_tx(ch, pack, ln + sizeof(adr) + sizeof(func) + sizeof(ln) + 2)) {
		if (adr > 0) adr--;
		modbus_rq_tx[ch][adr] = 0;
		modbus_rq_point[ch]++;
		if (modbus_rq_point[ch] >= MODBUS_MAX_DEV) modbus_rq_point[ch] = 0;
		return (1);
	}
	return (0);
}

/**
  * @brief  Возвращает готовность ModBus к передаче данных
  *
  * @param  adr: адрес устйроство которе запрашивает доступ к шине.
  * Если 0 - то проверяется готовность линии без учёта приоритетов
  * Если >0 - проверяется очереь запросов, и результат возвращается с учётом приорететов на отправку
  * в случае если линяя занята, то заданное устйроство ставится в очередь запросов на отправку
  *
  * @retval Состояние интерфейса: 1 - ModBus занят, 0 - ModBus свободен
  */
uint8_t modbus_get_busy(uint8_t ch, uint8_t adr, pr_t pr) {
	uint8_t cnt;
	if (adr > 0) { //если проверка доступности линии с учётом приоритетов
		adr--;
		if (modbus_busy[ch] == 0) { //шина доступна
			cnt = modbus_rq_point[ch]; // очередь устйроств с большим приоритетом
			while ((cnt != adr) && (modbus_rq_tx[ch][cnt] == 0)) {
				cnt++;
				if (cnt >= MODBUS_MAX_DEV) cnt = 0;
			}
			if (cnt == adr) { //если нет устройств с большим приоритетом
				return (0); //вернуть что канал свободен
			} else { // есть устйроства с большим приоритетом
				if (pr == Hi_pr) modbus_rq_tx[ch][adr] = 1; //установить флаг запроса на отправку
				return (1); // шина занята (ожидает более приоритетного устйроства)
			}
		} else return (1);
	} else return (modbus_busy[ch]);
}

/**
  * @brief  Отправляет пакет по интерфейсу ModBus
  *
  * @param  *data: указатель на пакет
  * 		len: размер данных
  *
  * @retval результат отправки: 1 - успешно, 0 - паакет не отправлен
  */
uint8_t modbus_tx (uint8_t ch, uint8_t *data, uint16_t len) {
	if (modbus_busy[ch]) return(0);
	if (f_tx_data[ch](data, len)) {
		modbus_busy[ch] = 1;
		modbus_tx_time[ch] = timers_get_finish_time(MODBUS_MAX_WAIT_TIME);
		return (1);
	}
	return (0);
}

/**
  * @brief  Функция: Запись нового значения в регистры хранения
  *
  * @param  adr: адрес устйроства
  * 		reg: номер  1-го регистра
  * 		num: количество регистров
  * 		val: записываемое значение
  *
  * @retval результат запроса: 1 - успешно, 0 - запрос не отправлен
  */
uint8_t modbus_wr_mreg (uint8_t ch, uint8_t adr, uint16_t reg, uint16_t num, uint8_t* data) {
	uint8_t pack[MB_TX_BUFF], i, j; udata16_t crc;
	if (modbus_busy[ch]) return(0);
	pack[0] = adr;
	pack[1] = MODBUS_FORCE_MULTIPLE_REGISTERS;
	pack[2] = (uint8_t)(reg >> 8);
	pack[3] = (uint8_t)reg;
	pack[4] = (uint8_t)(num >> 8);
	pack[5] = (uint8_t)num;
	num *= 2;
	pack[6] = (uint8_t)num;
	for (i = 0, j = 7; i < num; i++, j++)
		pack[j] = data[i];
	crc.word = modbus_crc_rtu(pack, j);
	pack[j++] = crc.byte[1];
	pack[j++] = crc.byte[0];
	if (modbus_tx(ch, pack, j)) {
		if (adr > 0) adr--;
		modbus_rq_tx[ch][adr] = 0;
		modbus_rq_point[ch]++;
		if (modbus_rq_point[ch] >= MODBUS_MAX_DEV)
			modbus_rq_point[ch] = 0;
		return 1;
	}
	return 0;
}
