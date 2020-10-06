/*
 * bcu.c
 *
 *  Created on: 10 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include <string.h>
#include <arm_math.h>
#include "bcu.h"
#include "canopen.h"
#include "timers.h"
#include "t_auto.h"

//--отправляемые данные--
bcu_tx_data_t bcu_tx_data;			//ШИМ1, ШИМ2, Положение, Выходы
//--принимаемые данные--
static udata16_t bcu_t_st; 			//значение датчика температуры
static udata16_t bcu_p_st; 			//значение датчика давления
static udata16_t bcu_position_st;	//положение сервопривода
static udata8_t  bcu_in_st;			//состояние выходов
static udata16_t bcu_torque_st;		//крутящий момент
static udata16_t bcu_frequency_st;	//частота вращения датчика момента
static udata16_t bcu_power_st;		//мощность на датчике момента

static udata8_t bcu_err;			//ошибка BCU

static uint8_t bcu_node_id=0;		//NOD ID устйроства BCU
static stime_t bcu_connect_time;	//таймер ожидания ответов
static stime_t bcu_tx_time;		//таймер отправки пакетов
static stime_t Qint_time;

static uint32_t bcu_k_pressure=BCU_MAX_PRESSURE/(BCU_I_MAX_PRESSURE-BCU_I_MIN_PRESSURE); //K - коэффициент пересчёта датчика давления
static uint32_t bcu_b_pressure=BCU_I_MIN_PRESSURE; 										//B - смещение дачтика давления
static uint32_t plsVnew = 0, plsVold = 0; // накопленное значение объема, л
static float32_t inQ = 0; // расход, л/мин
static float32_t outQ = 0; // расход, л/мин

/**
  * @brief Инициализация устйроства управления гидротормозом (BCU)
  *
  * @param  node_id: адрес устйроства на шине CanOpen (NODE_ID)
  */
void bcu_init(uint8_t node_id) {
	uint8_t cnt=0;
	memset(&bcu_tx_data.byte[0], 0, sizeof(bcu_tx_data));
	bcu_t_st.word = 5000;
	bcu_p_st.word = 0;
	bcu_position_st.word = 0;
	bcu_in_st.byte = 0;
	bcu_torque_st.word = 0;
	bcu_frequency_st.word = 0;
	bcu_power_st.word = 0;
	bcu_err.byte = 0;
	if (node_id <= 32) {
		while ((CanOpen_rx_object[cnt] != NULL) && (cnt < MAX_DEV_CANOPEN))	cnt++;
		if (cnt < MAX_DEV_CANOPEN) { //найти свободный указатель
			CanOpen_rx_object[cnt]=bcu_update_data; //указать обработчик принатых пакетов
			bcu_node_id = node_id;				  //сохранить адрес
			bcu_tx_time = timers_get_finish_time(BCU_DATA_TX_INIT);
			bcu_connect_time = timers_get_finish_time(BCU_CONNECT_TIME);
		}
	}
	Qint_time = timers_get_finish_time(Q_INTEGRAL_TIME);
}

/**
  * @brief  Обработчик пакетов принятых по CanOpen от BCU
  *
  * @param  *data: указатель на данные принятые по CanOpen
  * 		len: длина принятых данных
  * 		adr: COB ID пакета (CAN адрес)
  */
void bcu_update_data (char *data, uint8_t len, uint32_t adr) {
	uint16_t object;
	if (CanOpen_get_nodeid(adr) == bcu_node_id) {
		bcu_connect_time = timers_get_finish_time(BCU_CONNECT_TIME);
		object = CanOpen_get_object(adr);
		if (object == EMERGENCY)
			if (len >= 1) bcu_err.byte = data[0];
		if (object == PDO1_TX_SLAVE) {
			if (len >= 2) {
				bcu_t_st.byte[0]=data[0];
				bcu_t_st.byte[1]=data[1];
			}
			if (len>=6) {
				plsVnew = *((uint32_t *)&data[2]);
				if (plsVold == 0) plsVold = plsVnew;
			}
			if (len==8) bcu_in_st.byte=data[7];
		}
		/*if (object == PDO2_TX_SLAVE) {
			if (len >= 2) {
				bcu_torque_st.byte[0] = data[0];
				bcu_torque_st.byte[1] = data[1];
			}
			if (len >= 4) {
				bcu_frequency_st.byte[0] = data[2];
				bcu_frequency_st.byte[1] = data[3];
			}
			if (len >= 6) {
				bcu_power_st.byte[0] = data[4];
				bcu_power_st.byte[1] = data[5];
			}
		}
		if (object==SDO_TX_SLAVE)
			{
			if (len==8)
				{
				if ((CanOpen_get_index(data)==0x6200)&&(CanOpen_get_subindex(data)==0x00))
					{
					bcu_mask_st.byte[0]=data[4];
					}
				}
			}
		}*/
	}
}

/**
  * @brief  Шаг обработки данных устйроства управления гидротормозом (BCU)
  */
void bcu_step (void) {
	CanOpen_step();
	if (timers_get_time_left(bcu_tx_time) == 0) {
		if (timers_get_time_left(bcu_connect_time) == 0) {
			if (CanOpen_tx_nmt(NODE_START, bcu_node_id)) {
				bcu_tx_time = timers_get_finish_time(BCU_DATA_TX_INIT);
			}
		} else {
			if (CanOpen_tx_pdo(1, bcu_node_id, &bcu_tx_data.byte[0], 8)) {
				bcu_tx_time = timers_get_finish_time(BCU_DATA_TX_TIME);
			}
		}
	}
	if (timers_get_time_left(Qint_time) == 0) {
		Qint_time = timers_get_finish_time(Q_INTEGRAL_TIME);
		if (plsVnew > plsVold) inQ = (plsVnew - plsVold) * Q_T_MUL;
		else inQ = 0;
		outQ = (Q_TAU * inQ) + ((1.0 - Q_TAU) * outQ);
		plsVold = plsVnew;
	}
}

/**
  * @brief  Устанавливает значение ШИМ1
  *
  * @param  data: Значение ШИМ1 в м% от 0 до 100000
  */
void bcu_set_pwm1 (int32_t data)
{
	data/=100;
	bcu_tx_data.fld.pwm1=data;
}
/**
  * @brief  Устанавливает значение ШИМ2
  *
  * @param  data: Значение ШИМ2 в м% от 0 до 100000
  */
void bcu_set_pwm2 (int32_t data)
{
	data/=100;
	bcu_tx_data.fld.pwm2=data;
}
/**
  * @brief  Устанавливает положение сервопривода
  *
  * @param  data: положение сервопривода
  */
void bcu_set_position (int32_t data)
{
	bcu_tx_data.fld.position=data;
}
/**
  * @brief  Устанавливает значение дискретных выходов
  *
  * @param  data: битовое поле, используются только 4 младших бита, выход1 - бит0, выход4 - бит3
  */
void bcu_set_out (uint8_t data)
{
	bcu_tx_data.fld.out=(data&0x0F);
}

/**
  * @brief  Возвращает состояние дискретных входов
  *
  * @retval значение дискретных входов (битовое поле)
  */
uint8_t bcu_get_in (void)
{
	return(bcu_in_st.byte);
}
/**
  * @brief  Возвращает значение датчика температуры
  *
  * @retval температура: м°C
  */
int32_t bcu_get_t (void)
{
	return(t_auto_convert_r_to_val(bcu_t_st.word));
	//return(bcu_t_st.word);
}
/**
  * @brief  Возвращает значение датчика давления
  *
  * @retval давление в мБар
  */
int32_t bcu_get_p (void)
{
	return((bcu_p_st.word-bcu_b_pressure)*bcu_k_pressure);
}
/**
  * @brief  Возвращает положение сервопривода
  *
  * @retval положение сервопривода в условных единицах
  */
int32_t bcu_get_position (void)
{
	return(bcu_position_st.word);
}
/**
  * @brief  Возвращает значение крутящего момента с датчика момента
  *
  * @retval крутящий момент
  */
int32_t bcu_get_torque(void)
{
	return(bcu_torque_st.word);
}
/**
  * @brief  Возвращает значение частоты с датчика момента
  *
  * @retval значение частоты в об/мин
  */
int32_t bcu_get_frequency (void)
{
	return(bcu_frequency_st.word);
}
/**
  * @brief  Возвращает значение мощности с датчика момента
  *
  * @retval мощность
  */
int32_t bcu_get_power (void)
{
	return(bcu_power_st.word);
}

uint32_t bcu_get_Q (void) {
	return (uint32_t)outQ; // расход л/мин * 1000
	//return plsVnew * 1000;
}

/**
  * @brief  Возвращает состояние связи с BCU
  *
  * @retval состояние связи: 1-ошибка связи (нет данных от BCU) 0-связь с BCU исправна
  */
uint8_t bcu_err_link (void) {
	if (timers_get_time_left(bcu_connect_time) == 0) return (1);
	else return(0);
}

