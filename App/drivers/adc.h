/*
 * adc.h
 *
 *  Created on: 9 ���. 2016 �.
 *      Author: ������ �.�.
 */

#ifndef APP_DRIVERS_ADC_H_
#define APP_DRIVERS_ADC_H_
#include "stm32f4xx_hal.h"

#define ADC_MAX_VAL 4095	//������������ �������� ��� (���������� ���)
#define ADC_REF_VAL 3308	//������� ���������� ��� (� ��)

#define ADC_INTERNAL_1 hadc1
#define ADC_INTERNAL_2 hadc2
#define ADC_INTERNAL_3 hadc3

#define DMA_ADC1_IRQ_PRE 	if(hdma_adc1.Instance->NDTR==0) adc_1_stop()
#define DMA_ADC1_IRQ_POST 	if(hdma_adc1.Instance->NDTR==0) adc_1_reinit()
#define DMA_ADC2_IRQ_PRE	if(hdma_adc2.Instance->NDTR==0) adc_2_stop()
#define DMA_ADC2_IRQ_POST	if(hdma_adc2.Instance->NDTR==0) adc_2_reinit()
#define DMA_ADC3_IRQ_PRE	if(hdma_adc3.Instance->NDTR==0) adc_3_stop()
#define DMA_ADC3_IRQ_POST	if(hdma_adc3.Instance->NDTR==0) adc_3_reinit()

#define ADC_DEF_FILTERS	64	//������ ������ ��� ������� �� ���������
#define ADC_MAX_FILTERS	255	//����������� ������ ������ ��� �������

#define ADC_MAX_INPUT 16 //������������ ����� ������ ��� ��� ������� ��
//------------��� ��������� ������ ����� ��������������� ������������ � STM32CubeMX-----------------
#define ADC_1_CHAN_NUM	7 //����� ������� ������� ����� ������������ ���1
#define ADC_2_CHAN_NUM	5 //����� ������� ������� ����� ������������ ���2
#define ADC_3_CHAN_NUM	4 //����� ������� ������� ����� ������������ ���3

//������������������ ��������� �������: ����� ������� �������������� ������ ��� ���1, ���2, ���3
//TEMP, VREF, VBAT ������ ������ ���� �� ���1, �� ���3 ������ ���� ������ ������: 0,1,2,3,10,11,12,13

#define ADC_VBAT		0
#define ADC_CHAN_0		1
#define ADC_CHAN_5		2
#define ADC_CHAN_8		3
#define ADC_CHAN_9		4
#define ADC_CHAN_14		5
#define ADC_CHAN_15		6

#define ADC_CHAN_1		7
#define ADC_CHAN_2		8
#define ADC_CHAN_3		9
#define ADC_CHAN_6		10
#define ADC_CHAN_7		11

#define ADC_CHAN_10		12
#define ADC_CHAN_11		13
#define ADC_CHAN_12		14
#define ADC_CHAN_13		15
//--------------------------------------------------------------------------------------------------

void adc_init(void);						//������������� ����������� ���
uint8_t agc_get_chan_code(uint8_t chan);	//���������� ����� ������� � ������� ��� ���������� ������ ���
uint16_t adc_get_average(uint8_t element);	//���������� �������� ��� ����� ������� (������� ��������) � ���� ���
int32_t adc_get_u(uint8_t chan);			//���������� �������� ���������� �� ����� ���
int32_t adc_get_calc(uint8_t chan, int32_t k, int32_t b, uint8_t d, uint8_t base); //������������� �������� ��� � ������ ��������
void adc_set_filter(uint8_t filter_size);	//������������� ������ ������ ��� �������

void adc_1_reinit(void); 					//����������������� ���1
void adc_2_reinit(void); 					//����������������� ���2
void adc_3_reinit(void); //����������������� ���3
void adc_1_stop(void);						//������������� ���1
void adc_2_stop(void);						//������������� ���2
void adc_3_stop(void);						//������������� ���3

//----------------------------------------------------------------------------------------------------------------------------------
#if ADC_CHAN_4>=(ADC_1_CHAN_NUM+ADC_2_CHAN_NUM)
#error ������ ������������ ���: ����� ADC_CHAN_4 ���������� �� ������������ ���3
#endif
#if ADC_CHAN_5>=(ADC_1_CHAN_NUM+ADC_2_CHAN_NUM)
#error ������ ������������ ���: ����� ADC_CHAN_5 ���������� �� ������������ ���3
#endif
#if ADC_CHAN_6>=(ADC_1_CHAN_NUM+ADC_2_CHAN_NUM)
#error ������ ������������ ���: ����� ADC_CHAN_6 ���������� �� ������������ ���3
#endif
#if ADC_CHAN_7>=(ADC_1_CHAN_NUM+ADC_2_CHAN_NUM)
#error ������ ������������ ���: ����� ADC_CHAN_7 ���������� �� ������������ ���3
#endif
#if ADC_CHAN_8>=(ADC_1_CHAN_NUM+ADC_2_CHAN_NUM)
#error ������ ������������ ���: ����� ADC_CHAN_8 ���������� �� ������������ ���3
#endif
#if ADC_CHAN_9>=(ADC_1_CHAN_NUM+ADC_2_CHAN_NUM)
#error ������ ������������ ���: ����� ADC_CHAN_9 ���������� �� ������������ ���3
#endif
#if ADC_CHAN_14>=(ADC_1_CHAN_NUM+ADC_2_CHAN_NUM)
#error ������ ������������ ���: ����� ADC_CHAN_14 ���������� �� ������������ ���3
#endif
#if ADC_CHAN_15>=(ADC_1_CHAN_NUM+ADC_2_CHAN_NUM)
#error ������ ������������ ���: ����� ADC_CHAN_15 ���������� �� ������������ ���3
#endif
#if ADC_TEMP>=(ADC_1_CHAN_NUM)
#error ������ ������������ ���: ����� ADC_TEMP ���������� �� �� ���1
#endif
#if ADC_VREF>=(ADC_1_CHAN_NUM)
#error ������ ������������ ���: ����� ADC_VREF ���������� �� �� ���1
#endif
#if ADC_VBAT>=(ADC_1_CHAN_NUM)
#error ������ ������������ ���: ����� ADC_BAT ���������� �� �� ���1
#endif
#if (ADC_DEF_FILTERS>ADC_MAX_FILTERS)
#error ������ ������ ��� ������� ������ �����������-�����������
#endif

#endif /* APP_DRIVERS_ADC_H_ */