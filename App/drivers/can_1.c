/*
 * can_1.c
 *
 *  Created on: 5 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "can_1.h"
#include <string.h>

extern CAN_HandleTypeDef CAN_1;

static CanTxMsgTypeDef can_1_tx;
static CanRxMsgTypeDef can_1_buf[CAN_1_BUF_SIZE_RX];

static uint8_t can_1_buf_overflow;		  //������� ������������ ������
static uint32_t can_1_buf_read_pointer; //��������� � ������ �� ������ ������� ��� �� ����������
static uint32_t can_1_buf_write_pointer; //��������� � ������ �� ������ ���� ����� ������� ��������� �����

static uint8_t can_1_fnumber; //����� ������� �� �������������
static uint8_t can_1_ffifo; //����� FIFO �� ������� ���� ���������������� ������

/**
  * @brief  ������������� ���������� CAN
  *
  * @param prescaler: ���������� ������� ������: CAN_1_SPEED_
  *
  */
void can_1_init(uint8_t prescaler)
{
	CAN_1.Init.Prescaler = prescaler;
	HAL_CAN_Init(&CAN_1);

	can_1_clear_buf(); //�������� ������� �����
	CAN_1.pRxMsg=&can_1_buf[can_1_buf_write_pointer]; //���������� ��������� �� ���� ������ � ������ ������� ������
	CAN_1.pTxMsg=&can_1_tx;							  //���������������� ���������� ��� ������������ ������������� ������
	can_1_ffifo=CAN_FIFO0; can_1_fnumber=0;						  //���������� ��������� �� ������������� ������� ������� FIFO0
	can_1_set_filter32(0,0, 0,0, CAN_FILTERMODE_IDMASK); //������ ���������� �� ���� ���� �������
	can_1_ffifo=CAN_FIFO0; can_1_fnumber=0;						 //������� ��������� �� ������ 0, �.�. ��� ������ ������� ������� ������� ������ ������ ������ ���� �������������������

}

/**
  * @brief  ��������� 32-������� ������� �� ���� (��������������� ����� 2 ������� ��� �����)
  *
  * @param  id: id ��� ��������� � ������� 1
  * 		ide_rtr1: ��� �������1: ���0: �������� �������; ���1:������ � ID=29��� (0-�� ���������; 1-���������)
  *			id_mask: id ��� ��������� � ������� 2 (����� CAN_FILTERMODE_IDLIST) ��� ����� ��� ������ CAN_FILTERMODE_IDMASK
  *			ide_rtr2: ��� �������2: ���0: �������� �������; ���1:������ � ID=29���
  *			type: �����: 2 ������� ��� �����. CAN_FILTERMODE_IDLIST - 2 �������;  CAN_FILTERMODE_IDMASK - �����
  * @retval 0:������ �� ���������� 1:������ ����������
  */
uint8_t can_1_set_filter32(uint32_t id, uint8_t ide_rtr1, uint32_t id_mask, uint8_t ide_rtr2, uint8_t type)
{CAN_FilterConfTypeDef  CAN_FilterInitStructure;

	if (can_1_fnumber>CAN_1_END_FILTER) return(0); //���� ������������ ��� ��������� �������

	CAN_FilterInitStructure.FilterNumber = can_1_fnumber; //����� �������

	if (type==CAN_FILTERMODE_IDMASK)
		CAN_FilterInitStructure.FilterMode = CAN_FILTERMODE_IDMASK;//CAN_FilterInitStructure.FilterMode = CAN_FILTERMODE_IDMASK;
	else
		CAN_FilterInitStructure.FilterMode = CAN_FILTERMODE_IDLIST;

	CAN_FilterInitStructure.FilterScale = CAN_FILTERSCALE_32BIT;
	CAN_FilterInitStructure.FilterIdHigh = ((id&0x07FF)<<5)|((id&0x3E000)>>13); //������ ������� ��� ����� 0-10 STID � ��� ����� 13-17 EXID
	CAN_FilterInitStructure.FilterIdLow = ((id&0x1FFF)<<3)|((ide_rtr1&0x03)<<1); //������ ������� IDE, RTR � ����� 0-12 EXID
	CAN_FilterInitStructure.FilterMaskIdHigh = ((id_mask&0x07FF)<<5)|((id_mask&0x3E000)>>13); //������ ������� ��� ����� 0-10 STID � ��� ����� 13-17 EXID
	CAN_FilterInitStructure.FilterMaskIdLow = ((id_mask&0x1FFF)<<3)|((ide_rtr2&0x03)<<1); //������ ������� IDE, RTR � ����� 0-12 EXID;
	if (can_1_ffifo==CAN_FIFO0)	CAN_FilterInitStructure.FilterFIFOAssignment = CAN_FILTER_FIFO0;//������ ��� FIFO0
	else					CAN_FilterInitStructure.FilterFIFOAssignment = CAN_FILTER_FIFO1;//������ ��� FIFO1
	CAN_FilterInitStructure.FilterActivation = ENABLE;
	#ifdef CAN2
	CAN_FilterInitStructure.BankNumber=CAN_1_END_FILTER+1;
	#else
	CAN_FilterInitStructure.BankNumber=CAN_ALL_FILTERS;
	#endif

	HAL_CAN_ConfigFilter(&CAN_1, &CAN_FilterInitStructure);

	if (can_1_ffifo==CAN_FIFO0) //���� ������������� ������� �� FIFO0
		{
		HAL_CAN_Receive_IT(&CAN_1, CAN_FIFO0);	//�������� ���� ������� �� FIFO0
		CAN_1.State = HAL_CAN_STATE_READY;
		can_1_ffifo=CAN_FIFO1;					//��������� ������ ����� ���������� �� FIFO1
		}
	else
		{
		HAL_CAN_Receive_IT(&CAN_1, CAN_FIFO1);	//�������� ���� ������� �� FIFO1
		CAN_1.State = HAL_CAN_STATE_READY;
		can_1_ffifo=CAN_FIFO0;					//��������� ������ ����� ���������� �� FIFO0
		}
	can_1_fnumber++;

	return(1);

}

/**
  * @brief  ������� ����� �����
  */
void can_1_clear_buf(void)
{
	memset((uint8_t*)can_1_buf, 0, sizeof(can_1_buf));
	can_1_buf_read_pointer=0;
	can_1_buf_write_pointer=0;
	can_1_buf_overflow=0;
}

/**
  * @brief  ���������� ����� ������ � ������
  *
  * @retval �����
  */
void* can_1_get_buf_pointer(void)
{
	return(can_1_buf);
}

/**
  * @brief  ����������  ����� �������� ������ ���� � ����� ������� ��������� ����
  *
  * @retval �������� �� 0 �� CAN_1_BUF_SIZE-1
  */
uint32_t can_1_get_buf_shift(void)
{
	return(can_1_buf_write_pointer);
}

/**
  * @brief  ���������� ���������� ������������ ������
  *
  * @retval ���������� ������������
  */
uint32_t can_1_get_buf_overflow(void)
{
	return(can_1_buf_overflow);
}

/**
  * @brief  �������� �� ������ ��������, �� �� ������������ ������
  *
  * @param  *id: ��������� ���� �������� ������������� ��������� ������
  *			*length: ����������� ���������� ���� �������� ������ ��������� ������
  *			*data: ��������� �� �����, ���� ���� ����������� ������ (����� ������ ���� 8 ����)
  * @retval 0: ��� �������������� ������ 1:������ �����������
  */
uint8_t can_1_read_rx_data(uint32_t* id, uint8_t* length, uint8_t* data)
{uint32_t cpy_size=0, rx_point, buf_point=0;

	rx_point=can_1_get_buf_shift();
	if (rx_point==can_1_buf_read_pointer) return(0);

	if (can_1_buf[can_1_buf_read_pointer].IDE==CAN_ID_STD)
		*id=can_1_buf[can_1_buf_read_pointer].StdId;
	else
		*id=can_1_buf[can_1_buf_read_pointer].ExtId;

	*length=can_1_buf[can_1_buf_read_pointer].DLC;
	memset(data,0,8);
	memcpy(data, can_1_buf[can_1_buf_read_pointer].Data, can_1_buf[can_1_buf_read_pointer].DLC);

	can_1_buf_read_pointer++;
	if (can_1_buf_read_pointer>=CAN_1_BUF_SIZE_RX)  can_1_buf_read_pointer=0;

	return(1);
}

/**
  * @brief  ���������� ���������� ��������, �� �� ������������ ������� �������� �������
  *
  * @retval ���������� �������
  */
uint32_t can_1_get_rx_size(void)
{uint32_t sz=0, rx_point, pt;

	rx_point=can_1_get_buf_shift();
	pt=can_1_buf_read_pointer;

	if (rx_point<can_1_buf_read_pointer) //���� ��������� ����� ��������� ����� ����� ������
		{
		sz=CAN_1_BUF_SIZE_RX-can_1_buf_read_pointer;
		pt=0;
		}
	sz+=(rx_point-pt);

	return(sz);
}

/**
  * @brief  �������� � ����� �������� ������ � ���������� � CAN
  *
  * @param  id: �������������
  *			length: ������ ������ ������� ���� ��������� (�� ������ 8)
  *			data: ��������� �� ����� � �������, ������� ���� ���������
  * @retval 0: ����� �� ���������, 1: ����� ���������
  */
uint8_t can_1_write_tx_data(uint32_t id, uint8_t length, uint8_t* data)
{
	if (length>8) length=8; //���� ������� �������� ������ ��� 8 ����, �� ���������� ����� 8-� �������

	if (id>0x7FF)	CAN_1.pTxMsg->IDE = CAN_ID_EXT; //���� ������ ������ ������ 11 ���, �� ����������� ID = 29���
	else			CAN_1.pTxMsg->IDE = CAN_ID_STD; //����� ID 11���

	CAN_1.pTxMsg->RTR=0;	//������ �������� �������� �� �������������� (�� �������������� ��������� �������� ��������, �.�. ��� ��������� �� ������� �������������� � �������������� �� ��������� �����������)

	CAN_1.pTxMsg->DLC = length;
	CAN_1.pTxMsg->StdId = id;
	CAN_1.pTxMsg->ExtId = id;
	memcpy(CAN_1.pTxMsg->Data, data, length);

	if (HAL_CAN_Transmit(&CAN_1, CAN1_TX_TIMEOUT)==HAL_OK) return(1); //���� ����� ���������
	else return(0);
}

/**
  * @brief ����������� ��������� � �������� ������ �� ��������� ��������� �����
  *
  */
void can_1_buf_point_move(void)
{
	can_1_buf_write_pointer++;
	if (can_1_buf_write_pointer>=CAN_1_BUF_SIZE_RX)
		{
		can_1_buf_write_pointer=0;
		can_1_buf_overflow++;
		}
	CAN_1.pRxMsg=&can_1_buf[can_1_buf_write_pointer];

}

/**
  * @brief  ���������� ���������� CAN1 ������� FIFO0
  *
  *  @param  *hcan: ��������� �� ��������� � ��������� CAN
  *
  */
void can_1_rx0_irq(CAN_HandleTypeDef *hcan)
{
	can_1_buf_point_move();
	//CAN_1.State = HAL_CAN_STATE_READY;
	//HAL_CAN_Receive_IT(&CAN_1, CAN_FIFO0);
	__HAL_CAN_ENABLE_IT(hcan, CAN_IT_FMP0);
}

/**
  * @brief  ���������� ���������� CAN1 ������� FIFO1
  *
  *  @param  *hcan: ��������� �� ��������� � ��������� CAN
  *
  */
void can_1_rx1_irq(CAN_HandleTypeDef *hcan)
{
	can_1_buf_point_move();
	//CAN_1.State = HAL_CAN_STATE_READY;
	//HAL_CAN_Receive_IT(&CAN_1, CAN_FIFO1);
	__HAL_CAN_ENABLE_IT(hcan, CAN_IT_FMP1);
}