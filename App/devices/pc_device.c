/*
 * pc_device.c
 *
 *  Created on: 11 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "timers.h"
#include "pc_device.h"
#include "pc_link.h"
#include "rng.h"
#include <string.h>

#include "_signals.h"

static pc_device_session_t pc_device_session[PC_DEVICE_MAX_SESSION];	//список соединений в рамках которых разрешён приём пакетов
static uint8_t pc_device_out_session_rsp=0;					//требуется ответ не в рамках сессии (запрос на соединение)
static uint8_t pc_device_pack_rx[PC_LINK_BUF_SIZE-sizeof(header_t)];		//буфер для копирования принятого пакета
static uint8_t pc_device_pack_tx[PC_LINK_BUF_SIZE-sizeof(header_t)];		//буфер для формирования отправляемого пакета
static uint16_t pc_device_current =0;							//устройство с которым в данный момент ведётся обмен

#ifdef REV //если определена версия данных и соответсвенно структура данных
//--даннное устйроство невозможно уложить в общую логику, т.к. есть нестандартные режимы: сброс и отладка
//--поэтому создавать такие же стурктуры sg_t и sig_cfg_t не имеет смысла, поэтому эти структуры объявлены внешними из модуля _control.h
//--чтобы соблюдать общую концепцию модулей группы devices, нужно обявить переменные для всех входных и выходных данных, но т.к. уже имеются
//--внешние переменные из модуля _control, где есть все описанные переменные, то объявлять новые не имеет смысла
extern sig_cfg_t sig_cfg[SIG_END];  //описание сигналов
extern sg_t sg_st;					//состояние сигналов
#endif

/**
  * @brief  Инициализация устройства ПК
  */
void pc_device_init(void)
{uint8_t cnt;

#ifdef REV //если определена версия данных и соответсвенно структура данных
	pc_link_set_rev(REV);
#endif

	memset(pc_device_session,0,sizeof(pc_device_session));
	for (cnt=0; cnt<PC_DEVICE_MAX_SESSION; cnt++)
		pc_device_session[cnt].tx_time=timers_get_finish_time(0);
}

/**
  * @brief  //Шаг обработки данных устройства ПК
  */
void pc_device_step(void)
{
	pc_link_step(); //выполнить шаг обмена данными

	pc_device_process_rx(); //обработать принятые данные
	pc_device_prepare_tx(); //отправить ответ в случае необходимости
}

/**
  * @brief  //Обабатывает данные полученные для ПК
  */
static void pc_device_process_rx(void) //обработка принятых данных
{uint8_t rx_type=0, cnt=0, connect_mode=DT_CONNECTION_MNT;
 uint16_t session=DT_SESSION_BROADCAST, size=0, prt_point=0, prt_cnt=0;
	//-----------обработка принятых пакетов
 	 rx_type=pc_link_get_rx_mode();//get_rx_mode(); //прочитать какой последний пакет был принят
 	 if (rx_type!=PC_LINK_LOSS)
 	 	 {
 		 size=pc_link_read_data(pc_device_pack_rx,sizeof(pc_device_pack_rx));
	 	 session=pc_link_get_pc_session();//get_pc_session(); //прочитать номер соединения
 	 	 }
 	 else return;

	//--------------запрос соединения----------------------
#ifdef PC_RQ_CONNECT
	if (rx_type==PC_RQ_CONNECT) //если прнят запрос на подключение
#else
	if (rx_type==PC_DEVICE_RQ_CONNECT) //если прнят запрос на подключение
#endif
		{
		if (pc_device_pack_rx[FLD_MODE_CONNECTION]==DT_CONNECTION_CTR) //если устйроство требует подключения с возможностью управления
			{
			cnt=0;
			while ((cnt<PC_DEVICE_MAX_SESSION)&&(pc_device_session[cnt].mode!=DT_CONNECTION_CTR)) cnt++; //проверить уже есть устйроство с функцией управления или нет

			if (cnt>=PC_DEVICE_MAX_SESSION) connect_mode=DT_CONNECTION_CTR;
			else
				{
				if (pc_device_session[cnt].session==session) connect_mode=DT_CONNECTION_CTR; //если подключается устйроство уже имеющее права управления
				else connect_mode=DT_CONNECTION_MNT;
				}
			}
		else //иначе
			{
			connect_mode=pc_device_pack_rx[FLD_MODE_CONNECTION]; //записать тип подключения, такой как требует устйроство
			}

		cnt=0;
		if (session==0) //если запрос нового подключения без выдачи сессии запрашивающим устйроством
			while ((cnt<PC_DEVICE_MAX_SESSION)&&(pc_device_session[cnt].session!=0)) cnt++; //поиск свободного канала для подключения
		else //если запрос на изменение типа подключения
			{
			while ((cnt<PC_DEVICE_MAX_SESSION)&&(pc_device_session[cnt].session!=session)) cnt++; //поиск данных о подключении c запрашиваемой сессией
			if (cnt>=PC_DEVICE_MAX_SESSION)												//если запрашиваемая сессия не найдена
				{
				cnt=0;
				while ((cnt<PC_DEVICE_MAX_SESSION)&&(pc_device_session[cnt].session!=0)) cnt++; //поиск свободного канала для подключения
				}
			}

		if (cnt<PC_DEVICE_MAX_SESSION) //если есть свободное место для подключения
			{
			pc_device_session[cnt].mode=connect_mode;  //установить разрешённый режим сессии
			pc_device_session[cnt].session=session;
#ifdef PC_RSP_CONNECT
			pc_device_session[cnt].type=PC_RSP_CONNECT; //установить что устройству необходимо отправить пакет подверждения соединения
#else
			pc_device_session[cnt].type=PC_DEVICE_RSP_CONNECT; //установить что устройству необходимо отправить пакет подверждения соединения
#endif
			pc_device_session[cnt].time=timers_get_finish_time(PC_DEVICE_TIMEOUT_SESSION);//TIMER_GetTick();
			}
		else
			pc_device_out_session_rsp=1;
		}
	else //если это не запрос подключения
		{
		cnt=0;
		while ((cnt<PC_DEVICE_MAX_SESSION)&&(pc_device_session[cnt].session!=session)) cnt++; //поиск данных о подключенини
		if (cnt<PC_DEVICE_MAX_SESSION) //если подключение найдено
			{
			pc_device_session[cnt].type=rx_type;
			//if (pc_device_session[cnt].mode==DT_CONNECTION_CTR)
				{
#ifdef PC_DATA
				if (rx_type==PC_DATA)
					{
					if (sizeof(sg_st.pc.i)<size) size=sizeof(sg_st.pc.i);
					memcpy((uint8_t*)&sg_st.pc.i, pc_device_pack_rx, size);
					pc_device_session[cnt].type=PC_DATA;
					}
#endif
#ifdef PC_SIG
				if (rx_type==PC_SIG)
					{
					if (sizeof(sig_cfg)<size) size=sizeof(sig_cfg);
					memcpy((uint8_t*)&sig_cfg, pc_device_pack_rx, size);
					pc_device_session[cnt].type=PC_SIG;
					}
#endif
#ifdef PC_DEBUG
				if (rx_type==PC_DEBUG)
					{
					if (size>0)
						pc_device_session[cnt].type=PC_DEBUG;
					if (sizeof(sg_st)<size) size=sizeof(sg_st);
					memcpy((uint8_t*)&sg_st, pc_device_pack_rx, size);
					pc_device_session[cnt].type=PC_DEBUG;
					}
#endif
				//----ВЫЗОВ ПРЕДОПРЕДЕЛЁННОЙ ФУНКЦИИ----------------------------------------
				pc_device_rx_callback(pc_device_session[cnt].type, size, pc_device_pack_rx);
				}
			}
		pc_device_session[cnt].time=timers_get_finish_time(PC_DEVICE_TIMEOUT_SESSION);
		}
	//----------------проверка отключившихся сессий-------------------
	for (cnt=0; cnt<PC_DEVICE_MAX_SESSION; cnt++)
		{
		if (pc_device_session[cnt].session!=0)
			{
			if (timers_get_time_left(pc_device_session[cnt].time)==0)
				{
				memset(&pc_device_session[cnt],0,sizeof(pc_device_session[cnt]));
				}

			}
		}
}

/**
  * @brief  //Формирует данные для отправки на ПК
  */
static void pc_device_prepare_tx(void)
{uint16_t size, session_tmp, prt_point=0;//, prt_cnt=0;
uint16_t pc_tx_tmp;

	pc_tx_tmp=pc_device_current; //запомнить номер устйроства
	while (timers_get_time_left(pc_device_session[pc_device_current].tx_time)!=0) //искать устйроства которым надо отправить данные
		{
		pc_device_current++;					//взять следующее усттройство
		if (pc_device_current>=PC_DEVICE_MAX_SESSION) pc_device_current=0;
		if (pc_device_current==pc_tx_tmp) return; //если прошли по кругу, а устйроств которым надо отправить данные не нашли, значит выйти из процедуры
		}

	if (pc_link_tx_ready())
		{
		//------ответ на запрос соединения без установки сессии--------
		if (pc_device_out_session_rsp)
			{
			pc_device_pack_tx[FLD_MODE_CONNECTION]=DT_CONNECTION_ERR;
			pc_device_pack_tx[FLD_ERR_CONNECTION]=0;
			pc_device_pack_tx[FLD_SESSION]=(uint8_t)(DT_SESSION_BROADCAST&0x00FF);
			pc_device_pack_tx[FLD_SESSION+1]=(uint8_t)((DT_SESSION_BROADCAST>>8)&0x00FF);
#ifdef PC_RSP_CONNECT
			pc_link_write_data(PC_RSP_CONNECT, 0, pc_device_pack_tx, 4); //поставить пакет в очередь на отправку
#else
			pc_link_write_data(PC_DEVICE_RSP_CONNECT, 0, pc_device_pack_tx, 4); //поставить пакет в очередь на отправку
#endif

			pc_device_session[pc_device_current].tx_time=timers_get_finish_time(PC_DEVICE_TX_TIME);
			pc_device_out_session_rsp=0;
			return;
			}
		//--------------ответ на запрос соединения----------------------
#ifdef PC_RSP_CONNECT
		if (pc_device_session[pc_device_current].type==PC_RSP_CONNECT) //если требуется ответ на запрос соединения
#else
		if (pc_device_session[pc_device_current].type==PC_DEVICE_RSP_CONNECT) //если требуется ответ на запрос соединения
#endif
			{
			pc_device_pack_tx[FLD_MODE_CONNECTION]=pc_device_session[pc_device_current].mode;	//записать режим: управление или мониторинг
			pc_device_pack_tx[FLD_ERR_CONNECTION]=0;									//ошибок нет

			size=4;
			if (pc_device_session[pc_device_current].mode==DT_CONNECTION_ERR)			//если отправляли ответ: "ошибка подключения"
				{
				pc_device_session[pc_device_current].type=0;							//установить "нет необходимости данному устйройтву слать пакеты"
				session_tmp=DT_SESSION_BROADCAST;
				}
			else
				{
				if (pc_device_session[pc_device_current].session==DT_SESSION_BROADCAST) //если ещё не присвоен номер сессии
					session_tmp=rng_rnd16_soft(); //присвоить номер сессии
				else //если номер сессии уже был
					session_tmp=pc_device_session[pc_device_current].session;
				}

			pc_device_pack_tx[FLD_SESSION]=(uint8_t)(session_tmp&0x00FF);		//номер соединения
			pc_device_pack_tx[FLD_SESSION+1]=(uint8_t)((session_tmp>>8)&0x00FF);
			pc_link_write_data(pc_device_session[pc_device_current].type, pc_device_session[pc_device_current].session, pc_device_pack_tx, size); //поставить пакет в очередь на отправку
			pc_device_session[pc_device_current].tx_time=timers_get_finish_time(PC_DEVICE_TX_TIME);
			pc_device_session[pc_device_current].session=session_tmp;
			}
#ifdef PC_DATA
		//--------------пакет чтения данных----------------------
		if (pc_device_session[pc_device_current].type==PC_DATA) //если требуется отправить пакет с данными
			{
			pc_link_write_data(pc_device_session[pc_device_current].type, pc_device_session[pc_device_current].session,  (uint8_t*) (&sg_st.pc.o), sizeof(sg_st.pc.o));
			pc_device_session[pc_device_current].tx_time=timers_get_finish_time(PC_DEVICE_TX_TIME);
			}
#endif
#ifdef PC_SIG
		//--------------пакет c таблицей сигналов----------------------
		if (pc_device_session[pc_device_current].type==PC_SIG) //если требуется отправить пакет с данными
			{
			pc_link_write_data(pc_device_session[pc_device_current].type, pc_device_session[pc_device_current].session,  (uint8_t*) (&sig_cfg), sizeof(sig_cfg));
			pc_device_session[pc_device_current].tx_time=timers_get_finish_time(PC_DEVICE_TX_TIME);
			}
#endif
#ifdef PC_DEBUG
		//--------------пакет системных данных----------------------
		if (pc_device_session[pc_device_current].type==PC_DEBUG) //если требуется отправить пакет с данными
			{
			pc_link_write_data(pc_device_session[pc_device_current].type, pc_device_session[pc_device_current].session,  (uint8_t*) (&sg_st), sizeof(sg_st));
			pc_device_session[pc_device_current].tx_time=timers_get_finish_time(PC_DEVICE_TX_TIME);
			}
#endif

		pc_device_current++;
		if (pc_device_current>=PC_DEVICE_MAX_SESSION) pc_device_current=0;
		}
}

/**
  * @brief  Приём: предопределённая функция вызова обработчика верхнего упровня: обработчик приложения или обработчик алгоритма управления
  *
  * @param  type: тип принятого пакета: функция вызывается только ля неизвестных типов пакетов (все пакеты за исключением пакетов соединения)
  * 		size: размер принятого пакета
  * 		*pack: указатель на пакет
  */
__weak void pc_device_rx_callback(uint8_t type, uint8_t size, void *pack)
{
}
