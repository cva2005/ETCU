/*
 * bcu.c
 *
 *  Created on: 10 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "bcu.h"
#include "canopen.h"
#include "timers.h"
#include <string.h>
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

static uint32_t bcu_k_pressure=BCU_MAX_PRESSURE/(BCU_I_MAX_PRESSURE-BCU_I_MIN_PRESSURE); //K - коэффициент пересчёта датчика давления
static uint32_t bcu_b_pressure=BCU_I_MIN_PRESSURE; 										//B - смещение дачтика давления

/**
  * @brief Инициализация устйроства управления гидротормозом (BCU)
  *
  * @param  node_id: адрес устйроства на шине CanOpen (NODE_ID)
  */
void bcu_init(uint8_t node_id)
{uint8_t cnt=0;
extern CanOpen_rx_object_t CanOpen_rx_object[MAX_DEV_CANOPEN]; //указатели на функции обработчики пакетов от устройств CanOpen

	memset(&bcu_tx_data.byte[0], 0, sizeof(bcu_tx_data));

	bcu_t_st.word=5000;
	bcu_p_st.word=0;
	bcu_position_st.word=0;
	bcu_in_st.byte=0;
	bcu_torque_st.word=0;
	bcu_frequency_st.word=0;
	bcu_power_st.word=0;

	bcu_err.byte = 0;

	if (node_id<=32)
		{
		while ((CanOpen_rx_object[cnt]!=NULL)&&(cnt<MAX_DEV_CANOPEN))	cnt++; //найти свободный указатель
		if (cnt<MAX_DEV_CANOPEN)
			{
			CanOpen_rx_object[cnt]=bcu_update_data; //указать обработчик принатых пакетов
			bcu_node_id=node_id;				  //сохранить адрес
			bcu_tx_time=timers_get_finish_time(BCU_DATA_TX_INIT); 	   //установить время отправки следующего пакета
			bcu_connect_time=timers_get_finish_time(BCU_CONNECT_TIME); //Установить ограничение времени когда должен быть принят пакет PDO от slave устйроства
			}
		}
}

/**
  * @brief  Обработчик пакетов принятых по CanOpen от BCU
  *
  * @param  *data: указатель на данные принятые по CanOpen
  * 		len: длина принятых данных
  * 		adr: COB ID пакета (CAN адрес)
  */
void bcu_update_data (char *data, uint8_t len, uint32_t adr)
{uint16_t object;

	if (CanOpen_get_nodeid(adr)==bcu_node_id)
		{
		bcu_connect_time=timers_get_finish_time(BCU_CONNECT_TIME); //установить счётчик ожидания подключения
		object=CanOpen_get_object(adr);
		if (object==PDO1_TX_SLAVE)
			{
			if (len>=2)
				{
				bcu_t_st.byte[0]=data[0];
				bcu_t_st.byte[1]=data[1];
				}
			if (len>=4)
				{
				bcu_p_st.byte[0]=data[2];
				bcu_p_st.byte[1]=data[3];
				}
			if (len>=6)
				{
				bcu_position_st.byte[0]=data[4];
				bcu_position_st.byte[1]=data[5];
				}
			if (len==8)
				{
				bcu_in_st.byte=data[7];
				}
			}
		if (object==PDO2_TX_SLAVE)
			{
			if (len>=2)
				{
				bcu_torque_st.byte[0]=data[0];
				bcu_torque_st.byte[1]=data[1];
				}
			if (len>=4)
				{
				bcu_frequency_st.byte[0]=data[2];
				bcu_frequency_st.byte[1]=data[3];
				}
			if (len>=6)
				{
				bcu_power_st.byte[0]=data[4];
				bcu_power_st.byte[1]=data[5];
				}
			}
		if (object==EMERGENCY)
			{
			if (len>=1)
				{
				bcu_err.byte=data[0];
				}
			}
//		if (object==SDO_TX_SLAVE)
//			{
//			if (len==8)
//				{
//				if ((CanOpen_get_index(data)==0x6200)&&(CanOpen_get_subindex(data)==0x00))
//					{
//					bcu_mask_st.byte[0]=data[4];
//					}
//				}
//			}
		}
}

/**
  * @brief  Шаг обработки данных устйроства управления гидротормозом (BCU)
  */
void bcu_step(void)
{
	CanOpen_step();

	if (timers_get_time_left(bcu_tx_time)==0)			//если пришло время отправки пакета
		{
		if (timers_get_time_left(bcu_connect_time)==0)	//если истекло время ожидания данных от ППУ
			{
			if (CanOpen_tx_nmt(NODE_START, bcu_node_id))	//отправить команду старта
				{
				bcu_tx_time=timers_get_finish_time(BCU_DATA_TX_INIT);	//в случае успешной отправки, установить новое время следующей отправки
				}
			}
		else
			{
			if (CanOpen_tx_pdo(1, bcu_node_id, &bcu_tx_data.byte[0], 8))		  //отправить пакет данных
				{
				bcu_tx_time=timers_get_finish_time(BCU_DATA_TX_TIME); //установить время отправки следующего пакета
				}
			}
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

/**
  * @brief  Возвращает состояние связи с BCU
  *
  * @retval состояние связи: 1-ошибка связи (нет данных от BCU) 0-связь с BCU исправна
  */
uint8_t bcu_err_link (void)
{
	bcu_step();
	if (timers_get_time_left(bcu_connect_time)==0) return(1);
	else return(0);
}
