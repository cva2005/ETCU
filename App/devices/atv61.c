/*
 * atv61.c
 *
 *  Created on: 4 февр. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "atv61.h"
#include "canopen.h"
#include "modbus.h"
#include "timers.h"
#include <string.h>

static atv_pdo1_t atv61_tx_data; //отправляемые данные в PDO1
static atv_pdo1_t atv61_rx_data; //принмаемые данные из PDO1
static uint8_t atv61_node_id=0;		//NOD ID устйроства ATV61
static stime_t atv61_connect_time;	//таймер ожидания ответов
static stime_t atv61_tx_time;		//таймер отправки пакетов
//static uint8_t atv61_cmd=ATV61_STOP; //команда управления: вращенияе влево, вращение вправо
static int16_t atv61_frequency=0;
static uint8_t atv61_fault = 0; //ошибка преобразователя
static uint8_t atv61_cfg_step = 0;
static void atv61_cmd_prepare(void);
#ifdef ATV61_MB
uint8_t atv61_err_send = 0; //счётчик пакетов без ответа
#else
static udata8_t atv61_err;			//ошибка ATV61
extern CanOpen_rx_object_t CanOpen_rx_object[MAX_DEV_CANOPEN]; //указатели на функции обработчики пакетов от устройств CanOpen
#endif

/**
  * @brief Инициализация устйроства ATV61
  *
  * @param  node_id: адрес устйроства на шине CanOpen (NODE_ID)
  */
void atv61_init(uint8_t node_id) {
	uint8_t cnt=0;
	memset(&atv61_tx_data.byte[0], 0, sizeof(atv61_tx_data));
	memset(&atv61_rx_data.byte[0], 0, sizeof(atv61_rx_data));
	atv61_frequency = 0;
	atv61_fault = 0;
	atv61_cfg_step = 0;
#ifdef ATV61_MB
	atv61_err_send = 0; //счётчик пакетов без ответа
	if ((node_id <= 247) && (node_id > 0)) {
		while ((modbus_rx[cnt] != NULL) && (cnt < MODBUS_MAX_DEV)) cnt++; //найти свободный указатель
		if (cnt < MODBUS_MAX_DEV) {
			modbus_rx[cnt] = atv61_update_data; // обработчик принятых пакетов
			atv61_err_send = 0; //счётчик пакетов без ответа
			atv61_node_id = node_id; // сохранить адрес
			atv61_tx_time = timers_get_finish_time(ATV61_DATA_TX_TIME); // время отправки следующего пакета
			atv61_connect_time = timers_get_finish_time(ATV61_CONNECT_TIME); // время ответа от slave устйройства
		}
	}
#else
	atv61_err.byte = 0;
	if (node_id<=32) {
		while ((CanOpen_rx_object[cnt]!=NULL)&&(cnt<MAX_DEV_CANOPEN))	cnt++; //найти свободный указатель
		if (cnt<MAX_DEV_CANOPEN) {
			CanOpen_rx_object[cnt]=atv61_update_data; //указать обработчик принатых пакетов
			atv61_node_id=node_id;				  //сохранить адрес
			atv61_tx_time=timers_get_finish_time(ATV61_DATA_TX_INIT); 	   //установить время отправки следующего пакета
			atv61_connect_time=timers_get_finish_time(ATV61_CONNECT_TIME); //Установить ограничение времени когда должен быть принят пакет PDO от slave устйроства
		}
	}
#endif
}

/**
  * @brief  Обработчик пакетов принятых от ATV61
  *
  * @param  *data: указатель на принятые данные
  * 		len: длина принятых данных
  * 		adr: COB ID пакета (CAN адрес)
  */
#ifdef ATV61_MB
void atv61_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function) {
	if (adr == atv61_node_id) {
		atv61_err_send = 0;
		if (function == MODBUS_READ_HOLDING_REGISTERS) {
			atv61_rx_data.word[0] = GET_UINT16(data); // Status word
			atv61_rx_data.f.frequency = GET_UINT16(data + 2); // Output frequency
			atv61_tx_time = timers_get_finish_time(ATV61_DATA_TX_TIME); // время отправки следующего пакета
			atv61_connect_time = timers_get_finish_time(ATV61_CONNECT_TIME); // время ответа от slave устйройства
		}
	}
#else
void atv61_update_data (char *data, uint8_t len, uint32_t adr) Х
	uint16_t object, index, subindex;

	if (CanOpen_get_nodeid(adr)==atv61_node_id) {
		object=CanOpen_get_object(adr);
		if (object==PDO1_TX_SLAVE) {
			if (len>sizeof(atv61_rx_data)) len=sizeof(atv61_rx_data);
			memcpy(atv61_rx_data.byte, data, len);
			atv61_connect_time=timers_get_finish_time(ATV61_CONNECT_TIME); //установить счётчик ожидания подключения
			atv61_cfg_step=0;
		}
		if (object==EMERGENCY)
			if (len>=1) atv61_err.byte=data[0];
		if (object==SDO_TX_SLAVE) {
			index=CanOpen_get_index(data);
			subindex=CanOpen_get_subindex(data);
			atv61_cfg_step++;					//если SDO принят, значит получили подтверждение на отправку SDO перейти к отправки следующего слова конфигурации
			atv61_connect_time=timers_get_finish_time(0);	//установить "время ожидания данных от ПЧ истекло"
			atv61_tx_time=timers_get_finish_time(1);		//следующий пакет о править через 1 мс
		}
		if (object==BOOTUP) { //Если устйроство перезагрузилось
			atv61_connect_time=timers_get_finish_time(0); //установить "время ожидания данных от ПЧ истекло", т.к. после перезагрузки ПЧ не будет слать данные
			atv61_cfg_step=1;			//пропустить  0-1 шаг (сброс коммуникации)
		}
	}
#endif
}

/**
  * @brief  Подготавливает командное слово для ATV61 в зависимости от команды пользователя и состояния привода
  */
static void atv61_cmd_prepare(void)
{
	atv61_fault = 0;
	if (atv61_rx_data.f.cmd_st.eta.fault) { //если есть ошибка привода
		atv61_tx_data.f.cmd_st.word = 0;
		atv61_tx_data.f.cmd_st.cmd.fault_reset = 1; //установить флаг сброса ошибки
		atv61_tx_data.f.frequency = 0;
		atv61_fault = 1;
	} else if (atv61_rx_data.f.cmd_st.eta.switch_on_dsb) { //если запрещено включение привода
		atv61_tx_data.f.cmd_st.word = 0;
		atv61_tx_data.f.cmd_st.cmd.en_volage = 1; //разрешить включение привода
		atv61_tx_data.f.cmd_st.cmd.quick_stop = 1;
		atv61_tx_data.f.frequency = 0;
	} else if ((atv61_frequency != 0) && (atv61_fault == 0)) { // если есть команда пользователя
		if ((atv61_rx_data.f.cmd_st.eta.rdy_switch) && (atv61_rx_data.f.cmd_st.eta.quick_stop)) { //проверить готовность привода
			atv61_tx_data.f.cmd_st.word = 0;
			atv61_tx_data.f.cmd_st.cmd.switch_on = 1;
			atv61_tx_data.f.cmd_st.cmd.en_volage = 1;
			atv61_tx_data.f.cmd_st.cmd.quick_stop = 1;
			atv61_tx_data.f.cmd_st.cmd.enable_opr = 1;
			//atv61_tx_data.f.cmd_st.cmd.revers = 1;
			atv61_tx_data.f.frequency = atv61_frequency;
		}
	} else {
		atv61_tx_data.f.cmd_st.word = 0;
		atv61_tx_data.f.cmd_st.cmd.switch_on = 0;
		atv61_tx_data.f.cmd_st.cmd.en_volage = 1;
		atv61_tx_data.f.cmd_st.cmd.quick_stop = 1;
		if (atv61_rx_data.f.cmd_st.eta.opr_en) //если привод ещё не остановился
			atv61_tx_data.f.cmd_st.cmd.enable_opr = 1;
		atv61_tx_data.f.frequency = 0;
	}
#ifdef ATV61_MB
	atv61_tx_data.word[0] = SWAP16(atv61_tx_data.word[0]);
	atv61_tx_data.word[1] = SWAP16(atv61_tx_data.word[1]);
#endif
}

/**
  * @brief  Шаг обработки данных ATV61
  */
void atv61_step(void) {
	uint8_t tx_st;
#ifdef ATV61_MB
	if (timers_get_time_left(atv61_tx_time) == 0)	{
		if (modbus_get_busy(atv61_node_id)) return; // интерфейс занят
		if (atv61_cfg_step == 0) {
			atv61_cfg_step = 1;
			if (modbus_rd_hold_reg(atv61_node_id, MB_Status_word2, sizeof(atv_pdo1_t) / 2)) goto tx_compl;
		} else {
			atv61_cmd_prepare(); // подготовить данные для отправки
			if (modbus_wr_mreg(atv61_node_id, MB_Control_word2, sizeof(atv_pdo1_t) / 2, atv61_tx_data.byte)) {
				atv61_cfg_step = 0;
tx_compl:
				atv61_tx_time = timers_get_finish_time(ATV61_DATA_TX_TIME);
				if (atv61_err_send < 0xFF) atv61_err_send++;
			}
		}
	}

#else
	CanOpen_step();
	if (timers_get_time_left(atv61_tx_time)==0) { //если пришло время отправки пакета
		if (timers_get_time_left(atv61_connect_time)==0) { //если истекло время ожидания данных (нет данных от ПЧ)
			if (atv61_cfg_step>4) atv61_cfg_step=0;
			if (atv61_cfg_step==0)
				tx_st=CanOpen_tx_nmt(NODE_RESET_COMMUNICATION, atv61_node_id);
			if (atv61_cfg_step==1)
				tx_st=CanOpen_tx_sdo(atv61_node_id, 0x1800, 0x02, 0xFE, WR); //записать циклическую пережачу данных
			if (atv61_cfg_step==2)
				tx_st=CanOpen_tx_sdo(atv61_node_id, 0x1800, 0x05, 50, WR);	//период передачи каждые 50мс
			if (atv61_cfg_step==3)
				tx_st=CanOpen_tx_sdo(atv61_node_id, 0x605C, 0x00, 0x01, WR);	//?
			if (atv61_cfg_step==4) {
				tx_st=CanOpen_tx_nmt(NODE_START, atv61_node_id);	//отправить команду старта
				atv61_cfg_step++;
			}
			if (tx_st)
				atv61_tx_time=timers_get_finish_time(ATV61_DATA_TX_INIT);	//в случае успешной отправки, установить новое время следующей отправки
		} else {
			atv61_cmd_prepare();												//подготовить данные для отправки
			if (CanOpen_tx_pdo(1, atv61_node_id, atv61_tx_data.byte, sizeof(atv_pdo1_t))) //отправить пакет данных
				atv61_tx_time=timers_get_finish_time(ATV61_DATA_TX_TIME); //установить время отправки следующего пакета
		}
	}
#endif
}

/**
  * @brief  Устанавливает частоту ATV61
  *
  * @param  data: частота(об/мин * 1000)
  *
  */
void atv61_set_frequency(int32_t data)
{
	atv61_frequency = data / 1000;
}

/**
  * @brief  Возвращает значение частоты ATV61
  *
  * @retval частота об/мин*1000
  */
int32_t atv61_get_frequency(void)
{
	return (atv61_rx_data.f.frequency * 1000);
}

/**
  * @brief  Возвращает состояние связи
  *
  * @retval состояние связи: 1-ошибка связи (нет данных от устйроства) 0-связь исправна
  */
uint8_t atv61_err_link (void)
{
	if (timers_get_time_left(atv61_connect_time) == 0) return 1;
#ifdef ATV61_MB
	if (atv61_err_send > ATV61_MAX_ERR_SEND) return 1;
#endif
	return 0;
}

/**
  * @brief  Возвращает состояние ошибки ПЧ
  *
  * @retval состояние ПЧ: 1-ошибка 0-исправен
  */
uint8_t atv61_error (void)
{
	return atv61_fault;
}
