/*
 * pc_link.c
 *
 *  Created on: 11 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "pc_link.h"
#include "wifi_hf.h"
#include "timers.h"
#include "crc.h"
#include <string.h>
//------------------------���������� ��� ������� � ��------------------------------
static pc_link_pack_t pc_link_pack_rx; //����� ������� ������������ ������
static pc_link_pack_t pc_link_pack_tx; //����� ��� ������������ ������������� ������
static uint8_t pc_link_buf[PC_LINK_BUF_SIZE-sizeof(header_t)]; //����� ��� �������� ��������� ��������� ��������� ������
static uint16_t pc_link_rx_size=0; //������ �������� �����
static uint16_t pc_link_point_rx=0; //��������� �� ������� ����������� ����
static stime_t pc_link_tx_time;	//����� ����� ��������� �������� ���������� ������ (����������� �������� ����� ����������)
static stime_t pc_link_rx_time;	//����� �� �������� ������ ������ ����� ������, ����� ����� �� ��������� ���������� (������������ �������� ����� ������� �������)
static uint8_t pc_link_tx_busy=0; 	//���� "���������� �����"
static stime_t pc_link_rx_pack_time; //����� �� �������� ������ ���� �������� ���� ������ (����������� ���������� ����� ����� ������)
static uint8_t pc_link_tx_mode=PC_LINK_LOSS; 	//����� �������� ������ (��� ������������ �������)
static uint8_t pc_link_rx_mode=PC_LINK_LOSS; 	//����� ����� ������ (��� ����������� ������)
static uint16_t pc_link_rx_session=0; //����� ���������� (����������) �� ������� ������ �����
static uint16_t pc_link_tx_session=0;//����� ���������� (����������) ��� ��������
static uint16_t pc_link_rev=PC_LINK_REV; //������ ��������� � ������ ������, ��� ���������� ���������������

/**
  * @brief  ������������� ������ ������ � ��
  */
void pc_link_init(void)
{
	pc_link_reset();
}

/**
  * @brief	����� ������ ������ � �� (�����������������)
  */
void pc_link_reset(void)
{
	pc_link_tx_time=timers_get_finish_time(500);
	pc_link_pack_tx.fld.header.size=0;
	pc_link_rx_size=0; //������ �������� �����
	pc_link_point_rx=0; //��������� �� ������� ����������� ����
	pc_link_tx_time = timers_get_finish_time(0);	//�������� ����� ���������� ������
	pc_link_rx_time=timers_get_finish_time(PC_LINK_MAX_RX_TIMEOUT);	//����������� ���������� �������� ����� ������� �������
	pc_link_tx_mode=PC_LINK_LOSS; 	//����� �������� ������ (��� ������������ �������)
	pc_link_rx_mode=PC_LINK_LOSS; 	//����� ����� ������ (��� ����������� ������)
	wifi_hf_clear_buf();
}

/**
  * @brief  ���������� ��������� ������ ������ ��� ��������� �������� ������
  *
  * @retval ����� ������: �����, ����� �������, ����� ������������� � �.�.
  */
uint8_t pc_link_get_rx_mode(void)
{
	pc_link_step();
	if (pc_link_rx_time.del==0) return(PC_LINK_LOSS);//if ((pc_rx_time==0)&&(pc_tx_version==V_CE3)) return(PC_LINK_LOSS);
	else return(pc_link_rx_mode);
}

/**
  * @brief  ���������� ����� ������ ��� ��������� �������� ������
  *
  * @retval ���������� ����� ������
  */
uint16_t pc_link_get_pc_session(void)
{
	pc_link_step();
	return(pc_link_rx_session);
}

/**
  * @brief  ���������� ��������� ���������� ������ �������� ������ �� ��
  *
  * @retval ���������: 1-���������� ����� � ��������; 0-���������� ����� ��������� ����������� ������
  */
uint8_t pc_link_tx_ready(void)
{
	if ((wifi_hf_get_tx_status()==0)&&(pc_link_tx_busy==0)) return(1); //if ((wifiReadyTx())&&(pc_tx_busy==0)) return(1);
	else return(0);
}

/**
  * @brief  �������� ������ ��� �������� � ������� ��������
  *
  * @param  mode: ����� �������� ������ (��� ������)
  * 		session: ������ � ������ ������� ���� ��������� ������
  * 		p*: ��������� �� ����� � ������� ��� ��������
  * 		size: ������ ������������ ������ (������ ���� ������)
  *
  * @retval ���������� ������������ (������������ � ������� �� ��������) ������
  */
uint16_t pc_link_write_data(uint8_t mode, uint16_t session, uint8_t *p, uint16_t size)
{uint16_t tx_size=0;

	if ((wifi_hf_get_tx_status()==0)&&(pc_link_tx_busy==0))
		{
		if (size<=sizeof(pc_link_pack_tx.fld.data))
			{
			pc_link_tx_mode=mode;
			pc_link_tx_session=session;
			memcpy(pc_link_pack_tx.fld.data, p, size); //����������� ����������� ��� �������� ������
			pc_link_pack_tx.fld.header.size=size;//+sizeof(pc_pack_rx.fld.number)+sizeof(pc_pack_rx.fld.type); //��������� ������ ������
			tx_size=size;
			pc_link_tx_busy=1;
			}
		else tx_size=0;
		}
	pc_link_step();
	return (tx_size);
}

/**
  * @brief  ������������� �������� ���� "������ ���������", ������ ��������� ������� �� ���� ������, ������� ��������������� ������� ��������
  *
  * @param  rev: ������ ���������
  */
void pc_link_set_rev(uint16_t rev)
{
	pc_link_rev=rev;
}

/**
  * @brief  ��������� ��������� �������� ����� � ������ CRC
  *
  * @param  *buf: ��������� �� ����� ��� ����������� ������
  * 		size: ������ ���������������� ������
  *
  * @retval ���������� ������������� (��������) ������
  */
uint16_t pc_link_read_data(void *buf, uint16_t size)
{uint16_t rx_size_tmp;
	if ((size>=pc_link_rx_size)&&(pc_link_rx_size>0))
		{
		pc_link_step();
		memcpy(buf, pc_link_buf, pc_link_rx_size); //����������� ������ � ��������������� �����
		rx_size_tmp=pc_link_rx_size;
		pc_link_rx_size=0;
		pc_link_rx_mode=PC_LINK_LOSS;
		return(rx_size_tmp);
		}
	else
		return 0;
}

/**
  * @brief  ���������� �������� �������: ���������� ��� �� ������ �������, ������ ��������� � ��������� �� CRC
  */
static void pc_link_rx_data(void)
{uint32_t rx_size;
udata32_t crc;

	if (pc_link_point_rx<sizeof(pc_link_pack_rx.fld.header)) //���� ��������� ������ ��� �� ��������
		{
		rx_size=wifi_hf_get_rx_size(); //��������� ������� ���� ��������� �������
		if (rx_size>=(sizeof(pc_link_pack_rx.fld.header)-pc_link_point_rx)) //���� ��������� ��� ������, �� ��������� ���
			{
			pc_link_point_rx+=wifi_hf_read_rx_data(&pc_link_pack_rx.fld.header.start+pc_link_point_rx, (sizeof(pc_link_pack_rx.fld.header)-pc_link_point_rx));
			}
		}

	if (pc_link_point_rx>=sizeof(pc_link_pack_rx.fld.header)) //���� ��������� ������ ��������
		{
		rx_size=wifi_hf_get_rx_size(); //��������� ������� ���� ������ �������
		if (rx_size>=(pc_link_pack_rx.fld.header.size+4)) //���� ����� �������� ���� ������ ����� �������� � ���� ������ � ������� CRC (+4)
			{
			pc_link_point_rx+=wifi_hf_read_rx_data(pc_link_pack_rx.fld.data, (pc_link_pack_rx.fld.header.size+4));
			}
		}

	if (pc_link_point_rx>=(sizeof(pc_link_pack_rx.fld.header)+pc_link_pack_rx.fld.header.size+4)) //���� ������ ���� �����
		{
		pc_link_point_rx=0;  //��������� ���������� �� ���� ��������� �������
		crc.dword=crc32_ether(&pc_link_pack_rx.byte[0], (pc_link_pack_rx.fld.header.size+sizeof(pc_link_pack_rx.fld.header)), 1);
		if (memcmp(crc.byte, &pc_link_pack_rx.byte[pc_link_pack_rx.fld.header.size+sizeof(pc_link_pack_rx.fld.header)], 4)!=0)
			{
			pc_link_rx_size=0;
			pc_link_rx_mode=PC_LINK_LOSS;
			return;
			}
		pc_link_rx_time=timers_get_finish_time(PC_LINK_MAX_RX_TIMEOUT); //������� ������� ���������� ������ � ������� ��������� �������
		if (pc_link_pack_rx.fld.header.version==pc_link_rev)
			{
			pc_link_rx_mode=pc_link_pack_rx.fld.header.type;
			pc_link_rx_session=pc_link_pack_rx.fld.header.session;
			pc_link_rx_size=pc_link_pack_rx.fld.header.size;//-sizeof(pc_pack_rx.fld.number)-sizeof(pc_pack_rx.fld.type);
			if (pc_link_pack_rx.fld.header.size<sizeof(pc_link_buf))
			memcpy(pc_link_buf, pc_link_pack_rx.fld.data, pc_link_pack_rx.fld.header.size);//(pc_pack_rx.fld.size-sizeof(pc_pack_rx.fld.number)-sizeof(pc_pack_rx.fld.type)));
			}
		else
			{
			pc_link_rx_size=0;
			pc_link_rx_mode=PC_LINK_LOSS;
			}
		}
}

/**
  * @brief  ��������� ��������� ������ � ���������� ������ �� ������ (���������� pc_link_write_data)
  */
static void pc_link_tx_data(void)
{uint16_t pack_size=0;
 udata32_t crc;

 	if (pc_link_tx_busy) //���� ����� �� �������� �� ���� (���� ����� �� ��������)
 		{
 		if (wifi_hf_get_tx_status()==0) //���� ���������� ��������
			{
 			pc_link_pack_tx.fld.header.start=0x5A;
 			pc_link_pack_tx.fld.header.number++;
 			pc_link_pack_tx.fld.header.type=pc_link_tx_mode;
 			pc_link_pack_tx.fld.header.session=pc_link_tx_session;
 			pc_link_pack_tx.fld.header.version=pc_link_rev;

 			pack_size=pc_link_pack_tx.fld.header.size+sizeof(pc_link_pack_tx.fld.header); // + �����-���� + ������ ������
 			crc.dword=crc32_ether(&pc_link_pack_tx.byte[0], pack_size, 1);
 			memcpy(&pc_link_pack_tx.byte[pack_size], crc.byte, 4);

 			if (wifi_hf_write_tx_data(pc_link_pack_tx.byte, (pack_size+4))) //��������� � ������ 4 ����� CRC32
 				{
 				pc_link_tx_time=timers_get_finish_time(PC_LINK_MIN_TX_TIME);
 				pc_link_tx_busy=0;
 				}
			}
 		}
}

/**
  * @brief  //��� ������ pc_link: ���������� �������� � �������� ����
  */
void pc_link_step(void)
{uint16_t cnt;

	if (timers_get_time_left(pc_link_tx_time)==0) //������� ����������� ����� ����� ���������� �������
		{
		pc_link_tx_data();						  //��������� ����� (���� ���� ����� ��� ��������)
		}

	pc_link_rx_data();							 //���������� �������� �����
	if (pc_link_point_rx==0) //���� ��� ����� ������
		{
		pc_link_rx_pack_time=timers_get_finish_time(PC_LINK_MAX_RX_PACK_TIME); //���������� ����� �����, ����� ������ ���������� ���� ������
		}
	else					//���� ��� ���� ������
		{
		if (timers_get_time_left(pc_link_rx_pack_time)==0)	//���� ����� �� ���� ������ �������, �� ��� ���� ����� ������ �� �������, ������ � ������ "�����"
			wifi_hf_clear_buf();							//�������� �����
		}

}
