/*
 * pulse_in.c
 *
 *  Created on: 14 ���. 2016 �.
 *      Author: ������ �.�.
 */
#include "pulse_in.h"
//#include <string.h>

#ifdef PULSE_IN_1
extern TIM_HandleTypeDef PULSE_IN_1;
#endif
#ifdef PULSE_IN_2
extern TIM_HandleTypeDef PULSE_IN_2;
#endif
#ifdef PULSE_IN_3
extern TIM_HandleTypeDef PULSE_IN_3;
#endif
#ifdef PULSE_IN_4
extern TIM_HandleTypeDef PULSE_IN_4;
#endif
#ifdef PULSE_IN_5
extern TIM_HandleTypeDef PULSE_IN_5;
#endif
#ifdef PULSE_IN_6
extern TIM_HandleTypeDef PULSE_IN_6;
#endif
#ifdef PULSE_IN_BASE
extern TIM_HandleTypeDef PULSE_IN_BASE;
static uint8_t pulse_in_base_init=0; //����: "������� ������ ���������������"
#endif

static uint32_t pulse_in_ovr_tmp[PULSE_IN_NUMBER];	//��������� ������� ������������ (� �������� ���������)
static uint32_t pulse_in_ovr[PULSE_IN_NUMBER];		//�������� ������� ������������ �������
static uint32_t pulse_in_cmp[PULSE_IN_NUMBER];		//�������� ����� ��������� (� ������ input compare) ��� ����� ��������� �� ����� �� ����� ������ �������� �������
static uint8_t pulse_in_base_mode[PULSE_IN_NUMBER];	//������ ������ ����������� ����� (� ������� �������� ��� ���)
static TIM_HandleTypeDef *pulse_in_tim_point[PULSE_IN_NUMBER]; //���������� �� ��������� � ��������� �������� ����������� � ���������� ������
static uint32_t pulse_in_calc_var[PULSE_IN_NUMBER];	//���������� ����������� ��� ������� ������� (�������������� ��� ������� ������� � ���� �����)
static uint32_t pulse_in_base=1000;		//������ ������������  �������� ������� (�� ��������� ������ 1000 ��.)


/**
  * @brief  �������������� ���������� ����
  *
  * @param  chan: ����� ����������� ����� (������): �� 1 �� PULSE_IN_NUMBER
  */
void pulse_in_init(uint8_t chan)
{
	if (chan>PULSE_IN_NUMBER) chan=PULSE_IN_NUMBER;
	if (chan>0) chan--;
	switch (chan)
		{
		case 0:
#ifdef PULSE_IN_1 // ToDo: ���������� �������?
			pulse_in_tim_point[chan]=&PULSE_IN_1; //htim=&PULSE_IN_1;
			pulse_in_base_mode[chan]=PULSE_IN_1_BASE_MODE;
#endif
			break;
		case 1:
#ifdef PULSE_IN_2
			pulse_in_tim_point[chan]=&PULSE_IN_2;//htim=&PULSE_IN_2;
			pulse_in_base_mode[chan]=PULSE_IN_2_BASE_MODE;
#endif
			break;
		case 2:
#ifdef PULSE_IN_3
			pulse_in_tim_point[chan]=&PULSE_IN_3;//htim=&PULSE_IN_3;
			pulse_in_base_mode[chan]=PULSE_IN_3_BASE_MODE;
#endif
			break;
		case 3:
#ifdef PULSE_IN_4
			pulse_in_tim_point[chan]=&PULSE_IN_3;//htim=&PULSE_IN_3;
			pulse_in_base_mode[chan]=PULSE_IN_3_BASE_MODE;
#endif
			break;
		case 4:
#ifdef PULSE_IN_5
			pulse_in_tim_point[chan]=&PULSE_IN_3;//htim=&PULSE_IN_3;
			pulse_in_base_mode[chan]=PULSE_IN_3_BASE_MODE;
#endif
			break;
		case 5:
#ifdef PULSE_IN_6
			pulse_in_tim_point[chan]=&PULSE_IN_3;//htim=&PULSE_IN_3;
			pulse_in_base_mode[chan]=PULSE_IN_3_BASE_MODE;
#endif
			break;
		}

	pulse_in_ovr_tmp[chan]=0;
	pulse_in_ovr[chan]=0;
	pulse_in_cmp[chan]=0;
	pulse_in_calc_var[chan]=1;

	if (pulse_in_base_mode[chan])		//���� ���������� ���� �������� � ������� ��������
		{
#ifdef PULSE_IN_BASE
		HAL_TIM_Base_Start(pulse_in_tim_point[chan]);	//��������� ������ ����������� �����
		if (pulse_in_base_init==0)						//���� ������� ������ ��� �� ���������������
			{
			pulse_in_base_init=1;						//���������� ������� ��� ������� ������ �������
			if ((PULSE_IN_BASE.Instance==TIM1)||(PULSE_IN_BASE.Instance==TIM8)||(PULSE_IN_BASE.Instance==TIM9)
			 ||(PULSE_IN_BASE.Instance==TIM10)||(PULSE_IN_BASE.Instance==TIM11))	//���� ������ ������ ����������� �� APB2
				pulse_in_base=SYS_CLOCK_APB2*1000000;
			else																	//���� ������ ������ ����������� �� APB1
				pulse_in_base=SYS_CLOCK_APB1*1000000;
			pulse_in_base/=(PULSE_IN_BASE.Init.Prescaler+1);	//��������� ���� ������� ������� (����� ������ �����) � ���.
			pulse_in_base=pulse_in_base*1000/(PULSE_IN_BASE.Init.Period+1); //���������� ������ ������������ �������� ������� � ��.
			HAL_TIM_Base_Start_IT(&PULSE_IN_BASE);		//��������� ������� ������
			}
#endif
		}
	else								//���� ���������� ���� �������� � ������ ��������� ������� ����� ����������
		{
		if ((pulse_in_tim_point[chan]->Instance==TIM1)||(pulse_in_tim_point[chan]->Instance==TIM8)||(pulse_in_tim_point[chan]->Instance==TIM9)
		  ||(pulse_in_tim_point[chan]->Instance==TIM10)||(pulse_in_tim_point[chan]->Instance==TIM11))	//���� ������ ����������� �� APB2
			pulse_in_calc_var[chan]=(SYS_CLOCK_APB2*1000000)/(pulse_in_tim_point[chan]->Init.Prescaler+1); //���������� ������� �������
		else																							//���� ������ ����������� �� APB1
			pulse_in_calc_var[chan]=(SYS_CLOCK_APB1*1000000)/(pulse_in_tim_point[chan]->Init.Prescaler+1); //���������� ������� �������

		pulse_in_ovr[chan]=PULSE_MAX_OVERLOAD; //���������� �������� ������ �����: ������� ��� ��������� ���
		//HAL_TIM_IC_Start_IT(pulse_in_tim_point[chan], TIM_CHANNEL_1);		//��������� ������
		HAL_TIM_IC_Start(pulse_in_tim_point[chan], TIM_CHANNEL_1);		//��������� ������
		__HAL_TIM_ENABLE_IT(pulse_in_tim_point[chan], TIM_IT_UPDATE);	//���������� ���������� �� ������������ �������
		}
}

/**
  * @brief  ���������� ���������� ��������� � ������� (��)
  *
  * @param  chan: ����� ����������� ����� (������): �� 1 �� PULSE_IN_NUMBER
  *
  * @retval ���������� ����� ��������� � �������
  */
uint32_t pulse_in_sec(uint8_t chan)
{uint64_t cnt_tim;

	if (chan>0) chan--;

	if(pulse_in_base_mode[chan]==0)
		{
		cnt_tim=(uint64_t)pulse_in_ovr[chan]*PULSE_IN_3.Init.Period+pulse_in_cmp[chan]; //���������� ���������� ����������������� ������� ����� ����������
		return((uint64_t)pulse_in_calc_var[chan]/cnt_tim);	//���������� ������� � ��: ������� �������/�������� ������� (����� ������������ �������)

		}
	else
		return (pulse_in_cmp[chan]*1000/pulse_in_base);		//���������� ������� � ��: ����� ��������� *1000��/������ ������������ �������� �������
}

/**
  * @brief  ���������� ���������� ��������� � ������� (������� ��������)
  *
  * @param  chan: ����� ����������� ����� (������): �� 1 �� PULSE_IN_NUMBER
  *
  * @retval ���������� ����� ��������� � ������
  */
uint32_t pulse_in_min(uint8_t chan)
{uint64_t cnt_tim;
 uint32_t result;
 uint8_t cnt;

	if (chan>0) chan--;

	if(pulse_in_base_mode[chan]==0)
		{
		cnt_tim=(uint64_t)pulse_in_ovr[chan]*PULSE_IN_3.Init.Period+pulse_in_cmp[chan]; //���������� ���������� ����������������� ������� ����� ����������
		result=((uint64_t)pulse_in_calc_var[chan]*60ULL)/cnt_tim;
		return(result);
		}
	else
		{
		cnt_tim=(uint64_t)pulse_in_cmp[chan]*1000*60/pulse_in_base;
		if (pulse_in_ovr_tmp[chan]>0) //���� ������� ������ ������� ���������
			result=cnt_tim/(pulse_in_ovr_tmp[chan]-1); //�������� �� 60 ������ /���������� ���������
		else						//���� ������� �����������
			result=cnt_tim;

		return (result);
		}
}

/**
  * @brief  ���������� ���������� ��� �������� �� ������ �� ���������� (input compare - � ������ ��������� ������� ����� ����������)
  *
  * @param  *htim: ��������� �� �������� ������� ���������� ����������
  */
void pulse_in_capture_irq(TIM_HandleTypeDef *htim)
{uint8_t chan;

	__HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE); //����� ����� ������������ �������� ��������, �.�. ��� �� �������� ��������� � HAL ��������

	chan=0;
	while ((htim!=pulse_in_tim_point[chan])&&(chan<PULSE_IN_NUMBER)) chan++; //���� ����� ������� ���������� ����������

	if ((chan>=PULSE_IN_NUMBER)||(pulse_in_base_mode[chan]==1)) return; //���� �� ����� ������� ������ ��� ��������� ����� ��������� �� ����� �������� �������

	if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC1)==0) //���� ���������� ��������� ��-�� ������������� �������� (���� ��������� ==0)
		{
		if (pulse_in_ovr_tmp[chan]<PULSE_MAX_OVERLOAD) pulse_in_ovr_tmp[chan]++;	//��������� ��������� ������� ������������
		if (pulse_in_ovr_tmp[chan]>pulse_in_ovr[chan]) //���� ������� ������������ �������� ���������� ��������, ������ ������� ����������
			{
			pulse_in_ovr[chan]=pulse_in_ovr_tmp[chan]; //�������� �������� �������� ������������
			pulse_in_cmp[chan]=0; //�������� ������� ��������
			}
		}
	else	//���� ���������� ��������� ��-�� input compare (��������� ����� ��������): ��������� ���������
		{
		pulse_in_cmp[chan]=__HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1); 	//������������ ������� ��������
		pulse_in_ovr[chan]=pulse_in_ovr_tmp[chan];						//������������ ������� ������������
		pulse_in_ovr_tmp[chan]=0;										//�������� ��������� ������� ������������
		}

	__HAL_TIM_CLEAR_IT(htim, TIM_IT_CC1);
}

/**
  * @brief  ���������� ���������� �������� ������� (��� ������ ������ � ������ � ������� ��������): ��������� �������� �������� ������ � ������������� ��
  */
void pulse_in_base_irq(void)
{
#ifdef PULSE_IN_BASE
uint8_t chan;

	for(chan=0; chan<PULSE_IN_NUMBER; chan++) //���������� ��� ������
		{
		if (pulse_in_base_mode[chan])		//���� ����� �������� � ������ "� ������� ��������"
			{
			pulse_in_cmp[chan]=__HAL_TIM_GET_COUNTER(pulse_in_tim_point[chan]); //��������� ���������� ���������
			__HAL_TIM_SET_COUNTER(pulse_in_tim_point[chan], 0);					//�������� ������� ��������� �� �����
			if (pulse_in_cmp[chan]<3) //���� ������� �������� ������� ������ ������� ��������� ����� ��� � 2 ����
				{
				pulse_in_ovr[chan]+=pulse_in_cmp[chan];
				pulse_in_ovr_tmp[chan]++;
				}
			else
				{
				pulse_in_ovr[chan]=0;
				pulse_in_ovr_tmp[chan]=0;
				}
			}
		}
#endif
}


/**
  * @brief  ��������������� ������� ��������� ���������� �� ������������ �������� ����� ��� ���� ��������
  * 		�� �������� ���������, �,�, ���� ������ � HAL ��-�� ������� �� ������ ������������ ���� � �������� SR.
  * 		������� ������� �� ������������ ����������
  *
  * @param  *htim: ��������� �� �������� ������� ���������� ����������
  */
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{uint8_t chan=0;
//
//	__HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
//
//#ifdef PULSE_IN_BASE
//	if (htim==&PULSE_IN_BASE)
//		{
//		for(chan=0; chan<PULSE_IN_NUMBER; chan++) //���������� ��� ������
//			{
//			if (pulse_in_base_mode[chan])		//���� ����� �������� � ������ "� ������� ��������"
//				{
//				pulse_in_cmp[chan]=__HAL_TIM_GET_COUNTER(pulse_in_tim_point[chan]); //��������� ���������� ���������
//				__HAL_TIM_SET_COUNTER(pulse_in_tim_point[chan], 0);					//�������� ������� ��������� �� �����
//				if (pulse_in_cmp[chan]<3) //���� ������� �������� ������� ������ ������� ��������� ����� ��� � 2 ����
//					{
//					pulse_in_ovr[chan]+=pulse_in_cmp[chan];
//					pulse_in_ovr_tmp[chan]++;
//					}
//				else
//					{
//					pulse_in_ovr[chan]=0;
//					pulse_in_ovr_tmp[chan]=0;
//					}
//				}
//			}
//		return;
//		}
//#endif
//
//	while ((htim!=pulse_in_tim_point[chan])&&(chan<PULSE_IN_NUMBER)) chan++; //���� ����� ������� ���������� ����������
//
//	if ((chan>=PULSE_IN_NUMBER)||(pulse_in_base_mode[chan]==1)) return; //���� �� ����� ������� ������ ��� ��������� ����� ��������� �� ����� �������� �������
//
//	if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC1)==0) //���� ���������� ��������� ��-�� ������������� �������� (���� ��������� ==0)
//		{
//		pulse_in_ovr_tmp[chan]++;	//��������� ��������� ������� ������������
//		if (pulse_in_ovr_tmp[chan]>pulse_in_ovr[chan]) //���� ������� ������������ �������� ���������� ��������, ������ ������� ����������
//			{
//			pulse_in_ovr[chan]=pulse_in_ovr_tmp[chan]; //�������� �������� �������� ������������
//			pulse_in_cmp[chan]=0; //�������� ������� ��������
//			}
//		}
//	else	//���� ���������� ��������� ��-�� input compare (��������� ����� ��������): ��������� ���������
//		{
//		pulse_in_cmp[chan]=__HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1); 	//������������ ������� ��������
//		pulse_in_ovr[chan]=pulse_in_ovr_tmp[chan];						//������������ ������� ������������
//		pulse_in_ovr_tmp[chan]=0;										//�������� ��������� ������� ������������
//		}
//
//	__HAL_TIM_CLEAR_IT(htim, TIM_IT_CC1);
//}