/*
 * pc_device.c
 *
 *  Created on: 11 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "timers.h"
#include "pc_device.h"
#include "pc_link.h"
#include "rng.h"
#include <string.h>

#include "_signals.h"

static pc_device_session_t pc_device_session[PC_DEVICE_MAX_SESSION];	//������ ���������� � ������ ������� �������� ���� �������
static uint8_t pc_device_out_session_rsp=0;					//��������� ����� �� � ������ ������ (������ �� ����������)
static uint8_t pc_device_pack_rx[PC_LINK_BUF_SIZE-sizeof(header_t)];		//����� ��� ����������� ��������� ������
static uint8_t pc_device_pack_tx[PC_LINK_BUF_SIZE-sizeof(header_t)];		//����� ��� ������������ ������������� ������
static uint16_t pc_device_current =0;							//���������� � ������� � ������ ������ ������ �����

#ifdef REV //���� ���������� ������ ������ � ������������� ��������� ������
//--������� ���������� ���������� ������� � ����� ������, �.�. ���� ������������� ������: ����� � �������
//--������� ��������� ����� �� ��������� sg_t � sig_cfg_t �� ����� ������, ������� ��� ��������� ��������� �������� �� ������ _control.h
//--����� ��������� ����� ��������� ������� ������ devices, ����� ������� ���������� ��� ���� ������� � �������� ������, �� �.�. ��� �������
//--������� ���������� �� ������ _control, ��� ���� ��� ��������� ����������, �� ��������� ����� �� ����� ������
extern sig_cfg_t sig_cfg[SIG_END];  //�������� ��������
extern sg_t sg_st;					//��������� ��������
#endif

/**
  * @brief  ������������� ���������� ��
  */
void pc_device_init(void)
{uint8_t cnt;

#ifdef REV //���� ���������� ������ ������ � ������������� ��������� ������
	pc_link_set_rev(REV);
#endif

	memset(pc_device_session,0,sizeof(pc_device_session));
	for (cnt=0; cnt<PC_DEVICE_MAX_SESSION; cnt++)
		pc_device_session[cnt].tx_time=timers_get_finish_time(0);
}

/**
  * @brief  //��� ��������� ������ ���������� ��
  */
void pc_device_step(void)
{
	pc_link_step(); //��������� ��� ������ �������

	pc_device_process_rx(); //���������� �������� ������
	pc_device_prepare_tx(); //��������� ����� � ������ �������������
}

/**
  * @brief  //����������� ������ ���������� ��� ��
  */
static void pc_device_process_rx(void) //��������� �������� ������
{uint8_t rx_type=0, cnt=0, connect_mode=DT_CONNECTION_MNT;
 uint16_t session=DT_SESSION_BROADCAST, size=0, prt_point=0, prt_cnt=0;
	//-----------��������� �������� �������
 	 rx_type=pc_link_get_rx_mode();//get_rx_mode(); //��������� ����� ��������� ����� ��� ������
 	 if (rx_type!=PC_LINK_LOSS)
 	 	 {
 		 size=pc_link_read_data(pc_device_pack_rx,sizeof(pc_device_pack_rx));
	 	 session=pc_link_get_pc_session();//get_pc_session(); //��������� ����� ����������
 	 	 }
 	 else return;

	//--------------������ ����������----------------------
#ifdef PC_RQ_CONNECT
	if (rx_type==PC_RQ_CONNECT) //���� ����� ������ �� �����������
#else
	if (rx_type==PC_DEVICE_RQ_CONNECT) //���� ����� ������ �� �����������
#endif
		{
		if (pc_device_pack_rx[FLD_MODE_CONNECTION]==DT_CONNECTION_CTR) //���� ���������� ������� ����������� � ������������ ����������
			{
			cnt=0;
			while ((cnt<PC_DEVICE_MAX_SESSION)&&(pc_device_session[cnt].mode!=DT_CONNECTION_CTR)) cnt++; //��������� ��� ���� ���������� � �������� ���������� ��� ���

			if (cnt>=PC_DEVICE_MAX_SESSION) connect_mode=DT_CONNECTION_CTR;
			else
				{
				if (pc_device_session[cnt].session==session) connect_mode=DT_CONNECTION_CTR; //���� ������������ ���������� ��� ������� ����� ����������
				else connect_mode=DT_CONNECTION_MNT;
				}
			}
		else //�����
			{
			connect_mode=pc_device_pack_rx[FLD_MODE_CONNECTION]; //�������� ��� �����������, ����� ��� ������� ����������
			}

		cnt=0;
		if (session==0) //���� ������ ������ ����������� ��� ������ ������ ������������� �����������
			while ((cnt<PC_DEVICE_MAX_SESSION)&&(pc_device_session[cnt].session!=0)) cnt++; //����� ���������� ������ ��� �����������
		else //���� ������ �� ��������� ���� �����������
			{
			while ((cnt<PC_DEVICE_MAX_SESSION)&&(pc_device_session[cnt].session!=session)) cnt++; //����� ������ � ����������� c ������������� �������
			if (cnt>=PC_DEVICE_MAX_SESSION)												//���� ������������� ������ �� �������
				{
				cnt=0;
				while ((cnt<PC_DEVICE_MAX_SESSION)&&(pc_device_session[cnt].session!=0)) cnt++; //����� ���������� ������ ��� �����������
				}
			}

		if (cnt<PC_DEVICE_MAX_SESSION) //���� ���� ��������� ����� ��� �����������
			{
			pc_device_session[cnt].mode=connect_mode;  //���������� ����������� ����� ������
			pc_device_session[cnt].session=session;
#ifdef PC_RSP_CONNECT
			pc_device_session[cnt].type=PC_RSP_CONNECT; //���������� ��� ���������� ���������� ��������� ����� ������������ ����������
#else
			pc_device_session[cnt].type=PC_DEVICE_RSP_CONNECT; //���������� ��� ���������� ���������� ��������� ����� ������������ ����������
#endif
			pc_device_session[cnt].time=timers_get_finish_time(PC_DEVICE_TIMEOUT_SESSION);//TIMER_GetTick();
			}
		else
			pc_device_out_session_rsp=1;
		}
	else //���� ��� �� ������ �����������
		{
		cnt=0;
		while ((cnt<PC_DEVICE_MAX_SESSION)&&(pc_device_session[cnt].session!=session)) cnt++; //����� ������ � ������������
		if (cnt<PC_DEVICE_MAX_SESSION) //���� ����������� �������
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
				//----����� ����������˨���� �������----------------------------------------
				pc_device_rx_callback(pc_device_session[cnt].type, size, pc_device_pack_rx);
				}
			}
		pc_device_session[cnt].time=timers_get_finish_time(PC_DEVICE_TIMEOUT_SESSION);
		}
	//----------------�������� ������������� ������-------------------
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
  * @brief  //��������� ������ ��� �������� �� ��
  */
static void pc_device_prepare_tx(void)
{uint16_t size, session_tmp, prt_point=0;//, prt_cnt=0;
uint16_t pc_tx_tmp;

	pc_tx_tmp=pc_device_current; //��������� ����� ����������
	while (timers_get_time_left(pc_device_session[pc_device_current].tx_time)!=0) //������ ���������� ������� ���� ��������� ������
		{
		pc_device_current++;					//����� ��������� �����������
		if (pc_device_current>=PC_DEVICE_MAX_SESSION) pc_device_current=0;
		if (pc_device_current==pc_tx_tmp) return; //���� ������ �� �����, � ��������� ������� ���� ��������� ������ �� �����, ������ ����� �� ���������
		}

	if (pc_link_tx_ready())
		{
		//------����� �� ������ ���������� ��� ��������� ������--------
		if (pc_device_out_session_rsp)
			{
			pc_device_pack_tx[FLD_MODE_CONNECTION]=DT_CONNECTION_ERR;
			pc_device_pack_tx[FLD_ERR_CONNECTION]=0;
			pc_device_pack_tx[FLD_SESSION]=(uint8_t)(DT_SESSION_BROADCAST&0x00FF);
			pc_device_pack_tx[FLD_SESSION+1]=(uint8_t)((DT_SESSION_BROADCAST>>8)&0x00FF);
#ifdef PC_RSP_CONNECT
			pc_link_write_data(PC_RSP_CONNECT, 0, pc_device_pack_tx, 4); //��������� ����� � ������� �� ��������
#else
			pc_link_write_data(PC_DEVICE_RSP_CONNECT, 0, pc_device_pack_tx, 4); //��������� ����� � ������� �� ��������
#endif

			pc_device_session[pc_device_current].tx_time=timers_get_finish_time(PC_DEVICE_TX_TIME);
			pc_device_out_session_rsp=0;
			return;
			}
		//--------------����� �� ������ ����������----------------------
#ifdef PC_RSP_CONNECT
		if (pc_device_session[pc_device_current].type==PC_RSP_CONNECT) //���� ��������� ����� �� ������ ����������
#else
		if (pc_device_session[pc_device_current].type==PC_DEVICE_RSP_CONNECT) //���� ��������� ����� �� ������ ����������
#endif
			{
			pc_device_pack_tx[FLD_MODE_CONNECTION]=pc_device_session[pc_device_current].mode;	//�������� �����: ���������� ��� ����������
			pc_device_pack_tx[FLD_ERR_CONNECTION]=0;									//������ ���

			size=4;
			if (pc_device_session[pc_device_current].mode==DT_CONNECTION_ERR)			//���� ���������� �����: "������ �����������"
				{
				pc_device_session[pc_device_current].type=0;							//���������� "��� ������������� ������� ���������� ����� ������"
				session_tmp=DT_SESSION_BROADCAST;
				}
			else
				{
				if (pc_device_session[pc_device_current].session==DT_SESSION_BROADCAST) //���� ��� �� �������� ����� ������
					session_tmp=rng_rnd16_soft(); //��������� ����� ������
				else //���� ����� ������ ��� ���
					session_tmp=pc_device_session[pc_device_current].session;
				}

			pc_device_pack_tx[FLD_SESSION]=(uint8_t)(session_tmp&0x00FF);		//����� ����������
			pc_device_pack_tx[FLD_SESSION+1]=(uint8_t)((session_tmp>>8)&0x00FF);
			pc_link_write_data(pc_device_session[pc_device_current].type, pc_device_session[pc_device_current].session, pc_device_pack_tx, size); //��������� ����� � ������� �� ��������
			pc_device_session[pc_device_current].tx_time=timers_get_finish_time(PC_DEVICE_TX_TIME);
			pc_device_session[pc_device_current].session=session_tmp;
			}
#ifdef PC_DATA
		//--------------����� ������ ������----------------------
		if (pc_device_session[pc_device_current].type==PC_DATA) //���� ��������� ��������� ����� � �������
			{
			pc_link_write_data(pc_device_session[pc_device_current].type, pc_device_session[pc_device_current].session,  (uint8_t*) (&sg_st.pc.o), sizeof(sg_st.pc.o));
			pc_device_session[pc_device_current].tx_time=timers_get_finish_time(PC_DEVICE_TX_TIME);
			}
#endif
#ifdef PC_SIG
		//--------------����� c �������� ��������----------------------
		if (pc_device_session[pc_device_current].type==PC_SIG) //���� ��������� ��������� ����� � �������
			{
			pc_link_write_data(pc_device_session[pc_device_current].type, pc_device_session[pc_device_current].session,  (uint8_t*) (&sig_cfg), sizeof(sig_cfg));
			pc_device_session[pc_device_current].tx_time=timers_get_finish_time(PC_DEVICE_TX_TIME);
			}
#endif
#ifdef PC_DEBUG
		//--------------����� ��������� ������----------------------
		if (pc_device_session[pc_device_current].type==PC_DEBUG) //���� ��������� ��������� ����� � �������
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
  * @brief  ����: ��������������� ������� ������ ����������� �������� �������: ���������� ���������� ��� ���������� ��������� ����������
  *
  * @param  type: ��� ��������� ������: ������� ���������� ������ �� ����������� ����� ������� (��� ������ �� ����������� ������� ����������)
  * 		size: ������ ��������� ������
  * 		*pack: ��������� �� �����
  */
__weak void pc_device_rx_callback(uint8_t type, uint8_t size, void *pack)
{
}