/*
 * adc.c
 *
 *  Created on: 9 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "adc.h"
#include <string.h>
#include "types.h"

extern ADC_HandleTypeDef ADC_INTERNAL_1;
extern ADC_HandleTypeDef ADC_INTERNAL_2;
extern ADC_HandleTypeDef ADC_INTERNAL_3;

static uint16_t adc_1_buf[ADC_MAX_FILTERS][ADC_1_CHAN_NUM];
static uint16_t adc_2_buf[ADC_MAX_FILTERS][ADC_2_CHAN_NUM];
static uint16_t adc_3_buf[ADC_MAX_FILTERS][ADC_3_CHAN_NUM];

static uint8_t adc_max_filters=ADC_DEF_FILTERS;						//размер фильтра
static uint32_t adc_1_buf_size=0, adc_2_buf_size=0, adc_3_buf_size=0; //размер буфера для DMA

//uint32_t adc_seq[ADC_1_CHAN_NUM+ADC_2_CHAN_NUM+ADC_3_CHAN_NUM]={ADC_SEQ};

/**
  * @brief  Инициализация встроенного АЦП
  */
void adc_init(void)
{ADC_ChannelConfTypeDef sConfig;
 uint8_t rank=1, cnt;

	memset(adc_1_buf, 0, sizeof(adc_1_buf));
	memset(adc_2_buf, 0, sizeof(adc_2_buf));
	memset(adc_3_buf, 0, sizeof(adc_3_buf));

	adc_1_buf_size=ADC_1_CHAN_NUM*adc_max_filters;
	adc_2_buf_size=ADC_2_CHAN_NUM*adc_max_filters;
	adc_3_buf_size=ADC_3_CHAN_NUM*adc_max_filters;

//	sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
//	sConfig.Rank = 1;
//	for (cnt=0; cnt<(ADC_1_CHAN_NUM+ADC_2_CHAN_NUM+ADC_3_CHAN_NUM); cnt++)
//		{
//		sConfig.Channel=adc_seq[cnt];
//		if ((cnt==ADC_1_CHAN_NUM)||(cnt==ADC_2_CHAN_NUM)) sConfig.Rank=1;
//		if (cnt<ADC_1_CHAN_NUM)	HAL_ADC_ConfigChannel(&ADC_INTERNAL_1, &sConfig);
//		else
//			if (cnt<(ADC_1_CHAN_NUM+ADC_2_CHAN_NUM))	HAL_ADC_ConfigChannel(&ADC_INTERNAL_2, &sConfig);
//			else HAL_ADC_ConfigChannel(&ADC_INTERNAL_3, &sConfig);
//
//		sConfig.Rank++;
//		}

	adc_1_reinit();
	adc_2_reinit();
	adc_3_reinit();
}

/**
  * @brief  Возвращает номер элмента в массиве для указанного канала АЦП
  *
  * @param  chan: номер канала АЦП
  *
  * @retval номер элемента в буфере: ADC_VBAT, ADC_CHAN0 .....
  */
uint8_t agc_get_chan_code(uint8_t chan)
{
	switch (chan)
		{
#ifdef ADC_CHAN_0
		case 0: return(ADC_CHAN_0);
#endif
#ifdef ADC_CHAN_1
		case 1: return(ADC_CHAN_1);
#endif
#ifdef ADC_CHAN_2
		case 2: return(ADC_CHAN_2);
#endif
#ifdef ADC_CHAN_3
		case 3: return(ADC_CHAN_3);
#endif
#ifdef ADC_CHAN_4
		case 4: return(ADC_CHAN_4);
#endif
#ifdef ADC_CHAN_5
		case 5: return(ADC_CHAN_5);
#endif
#ifdef ADC_CHAN_6
		case 6: return(ADC_CHAN_6);
#endif
#ifdef ADC_CHAN_7
		case 7: return(ADC_CHAN_7);
#endif
#ifdef ADC_CHAN_8
		case 8: return(ADC_CHAN_8);
#endif
#ifdef ADC_CHAN_9
		case 9: return(ADC_CHAN_9);
#endif
#ifdef ADC_CHAN_10
		case 10: return(ADC_CHAN_10);
#endif
#ifdef ADC_CHAN_11
		case 11: return(ADC_CHAN_11);
#endif
#ifdef ADC_CHAN_12
		case 12: return(ADC_CHAN_12);
#endif
#ifdef ADC_CHAN_13
		case 13: return(ADC_CHAN_13);
#endif
#ifdef ADC_CHAN_14
		case 14: return(ADC_CHAN_14);
#endif
#ifdef ADC_CHAN_15
		case 15: return(ADC_CHAN_15);
#endif
#ifdef ADC_VBAT
		case 16: return(ADC_VBAT);
#endif
		default: return(0);
		}
}

/**
  * @brief  Возвращает значение АЦП после фильтра (среднее значение)
  *
  * @param  element: номер элемента в буфере: ADC_VBAT, ADC_CHAN0 .....
  *
  * @retval значение АЦП
  */
uint16_t adc_get_average(uint8_t element)
{uint16_t cnt;
uint64_t result=0;
uint16_t pt;

	if (element<ADC_1_CHAN_NUM)
		{
		pt=element;
		for (cnt=0; cnt<adc_max_filters; cnt++)
			result+=adc_1_buf[cnt][pt];
		}
	else
		{
		if (element<(ADC_1_CHAN_NUM+ADC_2_CHAN_NUM))
			{
			pt=element-ADC_1_CHAN_NUM;
			for (cnt=0; cnt<adc_max_filters; cnt++)
				result+=adc_2_buf[cnt][pt];
			}
		else
			{
			pt=element-ADC_1_CHAN_NUM-ADC_2_CHAN_NUM;
			for (cnt=0; cnt<adc_max_filters; cnt++)
				{
				result+=adc_3_buf[cnt][pt];
				}
			}
		}

	result/=adc_max_filters;

 	return((uint16_t)result);
}

/**
  * @brief  Возвращает значение напряжения на входе АЦП
  *
  * @param  sens: номер входа АЦП (от 0 до ADC_MAX_INPUT-1)
  *
  * @retval напряжение "мВ".
  */
int32_t adc_get_u(uint8_t chan)
{uint32_t adc_val;

	adc_val=adc_get_average(agc_get_chan_code(chan));
	if (ADC_MAX_VAL!=0)
		adc_val=(adc_val*ADC_REF_VAL)/ADC_MAX_VAL;

	return(adc_val);
}

/**
  * @brief  Пересчитывает значение АЦП в нужное значение
  *
  * @param  sens: номер канала АЦП (от 0 до ADC_MAX_INPUT-1)
  * 		k: (x-b)*K/10^d
  * 		b: (x-B)*k/10^d
  * 		d: (x-b)*k/10^D
  * 		base: множитель результата: 1-Х, 1000-мХ, 1000000-мкХ и т.д.
  *
  * @retval значение параметра в измеряемой величине (мА, мВ, м°С и т.д.)
  */
int32_t adc_get_calc(uint8_t chan, int32_t k, int32_t b, uint8_t d, uint8_t base)
{int64_t m;
int8_t cnt;
int32_t x;

	//x=adc_get_average(agc_get_chan_code(chan));
extern uint8_t jpo;

	x=adc_get_u(chan);
	m=(x-b)*k;

	if (d>base)
		for (cnt=0; cnt<(d-base); cnt++) {m/=10;}
	if (d<base)
		for (cnt=0; cnt<d; cnt++) {m*=10UL;}

	return ((int32_t)m);
}

/**
  * @brief  Устанавливает размер буфера для фильтра
  *
  * @param  filter_size: размер буфера от 1 до ADC_MAX_FILTERS
  */
void adc_set_filter(uint8_t filter_size)
{
	if (filter_size>ADC_MAX_FILTERS) filter_size=ADC_MAX_FILTERS;
	if (filter_size==0) filter_size=1;

	adc_max_filters=filter_size;
}

/**
  * @brief  Переинициализация АЦП1
  */
void adc_1_reinit(void)
{
	HAL_ADC_Start_DMA(&ADC_INTERNAL_1, (uint32_t*) adc_1_buf, adc_1_buf_size);
}

/**
  * @brief  Переинициализация АЦП2
  */
void adc_2_reinit(void)
{
	HAL_ADC_Start_DMA(&ADC_INTERNAL_2, (uint32_t*) adc_2_buf, adc_2_buf_size);
}

/**
  * @brief  Переинициализация АЦП3
  */
void adc_3_reinit(void)
{
	HAL_ADC_Start_DMA(&ADC_INTERNAL_3, (uint32_t*) adc_3_buf, adc_3_buf_size);
}

/**
  * @brief  Останавливает АЦП1
  */
void adc_1_stop(void)
{
	HAL_ADC_Stop_DMA(&ADC_INTERNAL_1);
}

/**
  * @brief  Останавливает АЦП2
  */
void adc_2_stop(void)
{
	HAL_ADC_Stop_DMA(&ADC_INTERNAL_2);
}

/**
  * @brief  Останавливает АЦП3
  */
void adc_3_stop(void)
{
	HAL_ADC_Stop_DMA(&ADC_INTERNAL_3);
}
