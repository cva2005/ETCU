/*
 * bcu.c
 *
 *  Created on: 10 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "bcu.h"
#include "canopen.h"
#include "timers.h"
#include <string.h>
#include "t_auto.h"

//--������������ ������--
bcu_tx_data_t bcu_tx_data;			//���1, ���2, ���������, ������
//--����������� ������--
static udata16_t bcu_t_st; 			//�������� ������� �����������
static udata16_t bcu_p_st; 			//�������� ������� ��������
static udata16_t bcu_position_st;	//��������� ������������
static udata8_t  bcu_in_st;			//��������� �������
static udata16_t bcu_torque_st;		//�������� ������
static udata16_t bcu_frequency_st;	//������� �������� ������� �������
static udata16_t bcu_power_st;		//�������� �� ������� �������

static udata8_t bcu_err;			//������ BCU

static uint8_t bcu_node_id=0;		//NOD ID ���������� BCU
static stime_t bcu_connect_time;	//������ �������� �������
static stime_t bcu_tx_time;		//������ �������� �������

static uint32_t bcu_k_pressure=BCU_MAX_PRESSURE/(BCU_I_MAX_PRESSURE-BCU_I_MIN_PRESSURE); //K - ����������� ��������� ������� ��������
static uint32_t bcu_b_pressure=BCU_I_MIN_PRESSURE; 										//B - �������� ������� ��������

/**
  * @brief ������������� ���������� ���������� ������������� (BCU)
  *
  * @param  node_id: ����� ���������� �� ���� CanOpen (NODE_ID)
  */
void bcu_init(uint8_t node_id)
{uint8_t cnt=0;
extern CanOpen_rx_object_t CanOpen_rx_object[MAX_DEV_CANOPEN]; //��������� �� ������� ����������� ������� �� ��������� CanOpen

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
		while ((CanOpen_rx_object[cnt]!=NULL)&&(cnt<MAX_DEV_CANOPEN))	cnt++; //����� ��������� ���������
		if (cnt<MAX_DEV_CANOPEN)
			{
			CanOpen_rx_object[cnt]=bcu_update_data; //������� ���������� �������� �������
			bcu_node_id=node_id;				  //��������� �����
			bcu_tx_time=timers_get_finish_time(BCU_DATA_TX_INIT); 	   //���������� ����� �������� ���������� ������
			bcu_connect_time=timers_get_finish_time(BCU_CONNECT_TIME); //���������� ����������� ������� ����� ������ ���� ������ ����� PDO �� slave ����������
			}
		}
}

/**
  * @brief  ���������� ������� �������� �� CanOpen �� BCU
  *
  * @param  *data: ��������� �� ������ �������� �� CanOpen
  * 		len: ����� �������� ������
  * 		adr: COB ID ������ (CAN �����)
  */
void bcu_update_data (char *data, uint8_t len, uint32_t adr)
{uint16_t object;

	if (CanOpen_get_nodeid(adr)==bcu_node_id)
		{
		bcu_connect_time=timers_get_finish_time(BCU_CONNECT_TIME); //���������� ������� �������� �����������
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
  * @brief  ��� ��������� ������ ���������� ���������� ������������� (BCU)
  */
void bcu_step(void)
{
	CanOpen_step();

	if (timers_get_time_left(bcu_tx_time)==0)			//���� ������ ����� �������� ������
		{
		if (timers_get_time_left(bcu_connect_time)==0)	//���� ������� ����� �������� ������ �� ���
			{
			if (CanOpen_tx_nmt(NODE_START, bcu_node_id))	//��������� ������� ������
				{
				bcu_tx_time=timers_get_finish_time(BCU_DATA_TX_INIT);	//� ������ �������� ��������, ���������� ����� ����� ��������� ��������
				}
			}
		else
			{
			if (CanOpen_tx_pdo(1, bcu_node_id, &bcu_tx_data.byte[0], 8))		  //��������� ����� ������
				{
				bcu_tx_time=timers_get_finish_time(BCU_DATA_TX_TIME); //���������� ����� �������� ���������� ������
				}
			}
		}
}

/**
  * @brief  ������������� �������� ���1
  *
  * @param  data: �������� ���1 � �% �� 0 �� 100000
  */
void bcu_set_pwm1 (int32_t data)
{
	data/=100;
	bcu_tx_data.fld.pwm1=data;
}
/**
  * @brief  ������������� �������� ���2
  *
  * @param  data: �������� ���2 � �% �� 0 �� 100000
  */
void bcu_set_pwm2 (int32_t data)
{
	data/=100;
	bcu_tx_data.fld.pwm2=data;
}
/**
  * @brief  ������������� ��������� ������������
  *
  * @param  data: ��������� ������������
  */
void bcu_set_position (int32_t data)
{
	bcu_tx_data.fld.position=data;
}
/**
  * @brief  ������������� �������� ���������� �������
  *
  * @param  data: ������� ����, ������������ ������ 4 ������� ����, �����1 - ���0, �����4 - ���3
  */
void bcu_set_out (uint8_t data)
{
	bcu_tx_data.fld.out=(data&0x0F);
}

/**
  * @brief  ���������� ��������� ���������� ������
  *
  * @retval �������� ���������� ������ (������� ����)
  */
uint8_t bcu_get_in (void)
{
	return(bcu_in_st.byte);
}
/**
  * @brief  ���������� �������� ������� �����������
  *
  * @retval �����������: �C
  */
int32_t bcu_get_t (void)
{
	return(t_auto_convert_r_to_val(bcu_t_st.word));
	//return(bcu_t_st.word);
}
/**
  * @brief  ���������� �������� ������� ��������
  *
  * @retval �������� � ����
  */
int32_t bcu_get_p (void)
{
	return((bcu_p_st.word-bcu_b_pressure)*bcu_k_pressure);
}
/**
  * @brief  ���������� ��������� ������������
  *
  * @retval ��������� ������������ � �������� ��������
  */
int32_t bcu_get_position (void)
{
	return(bcu_position_st.word);
}
/**
  * @brief  ���������� �������� ��������� ������� � ������� �������
  *
  * @retval �������� ������
  */
int32_t bcu_get_torque(void)
{
	return(bcu_torque_st.word);
}
/**
  * @brief  ���������� �������� ������� � ������� �������
  *
  * @retval �������� ������� � ��/���
  */
int32_t bcu_get_frequency (void)
{
	return(bcu_frequency_st.word);
}
/**
  * @brief  ���������� �������� �������� � ������� �������
  *
  * @retval ��������
  */
int32_t bcu_get_power (void)
{
	return(bcu_power_st.word);
}

/**
  * @brief  ���������� ��������� ����� � BCU
  *
  * @retval ��������� �����: 1-������ ����� (��� ������ �� BCU) 0-����� � BCU ��������
  */
uint8_t bcu_err_link (void)
{
	//bcu_step();
	if (timers_get_time_left(bcu_connect_time)==0) return(1);
	else return(0);
}