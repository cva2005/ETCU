/*
 * pulse_in.c
 *
 *  Created on: 14 янв. 2016 г.
 *      Author: Перчиц А.Н.
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
static uint8_t pulse_in_base_init=0; //флаг: "базовый таймер инициализирован"
#endif

static uint32_t pulse_in_ovr_tmp[PULSE_IN_NUMBER];	//временный счётчик переполнений (в процессе измерения)
static uint32_t pulse_in_ovr[PULSE_IN_NUMBER];		//основной счётчик переполнений таймера
static uint32_t pulse_in_cmp[PULSE_IN_NUMBER];		//значение входа сравнения (в режиме input compare) или число импульсов на входе за время работы базового таймера
static uint8_t pulse_in_base_mode[PULSE_IN_NUMBER];	//режимы работы импульсного входа (с базовым таймером или нет)
static TIM_HandleTypeDef *pulse_in_tim_point[PULSE_IN_NUMBER]; //указательи на структуры с описанием таймеров поключённых к импульсным входам
static uint32_t pulse_in_calc_var[PULSE_IN_NUMBER];	//переменные необходимые для расчёта частоты (индивидуальные для каждого таймера и типа входа)
static uint32_t pulse_in_base=1000;		//период срабатывания  базового таймера (по умолчанию каждые 1000 мс.)


/**
  * @brief  Инициалицирует импульсный вход
  *
  * @param  chan: номер импульсного входа (канала): от 1 до PULSE_IN_NUMBER
  */
void pulse_in_init(uint8_t chan)
{
	if (chan>PULSE_IN_NUMBER) chan=PULSE_IN_NUMBER;
	if (chan>0) chan--;
	switch (chan)
		{
		case 0:
#ifdef PULSE_IN_1
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

	if (pulse_in_base_mode[chan])		//если импульсный вход работает с базовым таймером
		{
#ifdef PULSE_IN_BASE
		HAL_TIM_Base_Start(pulse_in_tim_point[chan]);	//запустить таймер импульсного входа
		if (pulse_in_base_init==0)						//если базовый таймер ещё не инициализирован
			{
			pulse_in_base_init=1;						//установить признак что базовый таймер запущен
			if ((PULSE_IN_BASE.Instance==TIM1)||(PULSE_IN_BASE.Instance==TIM8)||(PULSE_IN_BASE.Instance==TIM9)
			 ||(PULSE_IN_BASE.Instance==TIM10)||(PULSE_IN_BASE.Instance==TIM11))	//если базовй таймер тактируется от APB2
				pulse_in_base=SYS_CLOCK_APB2*1000000;
			else																	//если базовй таймер тактируется от APB1
				pulse_in_base=SYS_CLOCK_APB1*1000000;
			pulse_in_base/=(PULSE_IN_BASE.Init.Prescaler+1);	//расчитать цену деления таймера (время одного счёта) в сек.
			pulse_in_base=pulse_in_base*1000/(PULSE_IN_BASE.Init.Period+1); //рассчитать период срабатывания базового таймера в мс.
			HAL_TIM_Base_Start_IT(&PULSE_IN_BASE);		//запустить базовый таймер
			}
#endif
		}
	else								//если импульсный вход работает в режиме измерения периода между импульсами
		{
		if ((pulse_in_tim_point[chan]->Instance==TIM1)||(pulse_in_tim_point[chan]->Instance==TIM8)||(pulse_in_tim_point[chan]->Instance==TIM9)
		  ||(pulse_in_tim_point[chan]->Instance==TIM10)||(pulse_in_tim_point[chan]->Instance==TIM11))	//если таймер тактируется от APB2
			pulse_in_calc_var[chan]=(SYS_CLOCK_APB2*1000000)/(pulse_in_tim_point[chan]->Init.Prescaler+1); //определить частоту таймера
		else																							//если таймер тактируется от APB1
			pulse_in_calc_var[chan]=(SYS_CLOCK_APB1*1000000)/(pulse_in_tim_point[chan]->Init.Prescaler+1); //определить частоту таймера

		pulse_in_ovr[chan]=PULSE_MAX_OVERLOAD; //естановить заведомо больше число: считаем что импульсов нет
		//HAL_TIM_IC_Start_IT(pulse_in_tim_point[chan], TIM_CHANNEL_1);		//запустить таймер
		HAL_TIM_IC_Start(pulse_in_tim_point[chan], TIM_CHANNEL_1);		//запустить таймер
		__HAL_TIM_ENABLE_IT(pulse_in_tim_point[chan], TIM_IT_UPDATE);	//установить прерывание на перезагрузку таймера
		}
}

/**
  * @brief  Возвращает количество импульсов в секунду (Гц)
  *
  * @param  chan: номер импульсного входа (канала): от 1 до PULSE_IN_NUMBER
  *
  * @retval количество целых импульсов в секунду
  */
uint32_t pulse_in_sec(uint8_t chan)
{uint64_t cnt_tim;

	if (chan>0) chan--;

	if(pulse_in_base_mode[chan]==0)
		{
		cnt_tim=(uint64_t)pulse_in_ovr[chan]*PULSE_IN_3.Init.Period+pulse_in_cmp[chan]; //рассчитать количество инкриментирований таймера между импульсами
		return((uint64_t)pulse_in_calc_var[chan]/cnt_tim);	//рассчитать частоту в Гц: частота таймера/значение таймера (число срабатываний таймера)

		}
	else
		return (pulse_in_cmp[chan]*1000/pulse_in_base);		//рассчитать частоту в Гц: число импульсов *1000мс/период срабатывания базового таймера
}

/**
  * @brief  Возвращает количество импульсов в миниуту (среднее значение)
  *
  * @param  chan: номер импульсного входа (канала): от 1 до PULSE_IN_NUMBER
  *
  * @retval количество целых импульсов в минуту
  */
uint32_t pulse_in_min(uint8_t chan)
{uint64_t cnt_tim;
 uint32_t result;
 uint8_t cnt;

	if (chan>0) chan--;

	if(pulse_in_base_mode[chan]==0)
		{
		cnt_tim=(uint64_t)pulse_in_ovr[chan]*PULSE_IN_3.Init.Period+pulse_in_cmp[chan]; //рассчитать количество инкриментирований таймера между импульсами
		result=((uint64_t)pulse_in_calc_var[chan]*60ULL)/cnt_tim;
		return(result);
		}
	else
		{
		cnt_tim=(uint64_t)pulse_in_cmp[chan]*1000*60/pulse_in_base;
		if (pulse_in_ovr_tmp[chan]>0) //если слишком низкая частота импульсов
			result=cnt_tim/(pulse_in_ovr_tmp[chan]-1); //умножить на 60 секунд /количество импульсов
		else						//если частота достаточная
			result=cnt_tim;

		return (result);
		}
}

/**
  * @brief  Обработчик прерываний для таймеров на входах со сравнением (input compare - в режиме имзерения периода между импульсами)
  *
  * @param  *htim: указатель на описание таймера вызвавшего прерывание
  */
void pulse_in_capture_irq(TIM_HandleTypeDef *htim)
{uint8_t chan;

	__HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE); //сброс флага перезагрузки счётного регистра, т.к. это не работает нормально в HAL драйвере

	chan=0;
	while ((htim!=pulse_in_tim_point[chan])&&(chan<PULSE_IN_NUMBER)) chan++; //ищем номер таймера вызвавшего прерывание

	if ((chan>=PULSE_IN_NUMBER)||(pulse_in_base_mode[chan]==1)) return; //если не нашли нужного канала или найденный канал настройен на режим базового таймера

	if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC1)==0) //если прерывание произошло из-за переполнениия счётчика (флаг сравнения ==0)
		{
		if (pulse_in_ovr_tmp[chan]<PULSE_MAX_OVERLOAD) pulse_in_ovr_tmp[chan]++;	//увеличить временный счётчик переполнений
		if (pulse_in_ovr_tmp[chan]>pulse_in_ovr[chan]) //если счётчик переполнений превысил предыдущие значение, значит частота уменшилась
			{
			pulse_in_ovr[chan]=pulse_in_ovr_tmp[chan]; //обновить значение счётчика переполнений
			pulse_in_cmp[chan]=0; //обнулить остаток счётчика
			}
		}
	else	//если прерывание произошло из-за input compare (обнаружен фронт импульса): измерение закончено
		{
		pulse_in_cmp[chan]=__HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1); 	//перизаписать остаток счётчика
		pulse_in_ovr[chan]=pulse_in_ovr_tmp[chan];						//перезаписать счётчик переполнений
		pulse_in_ovr_tmp[chan]=0;										//обнулить временный счётчик переполнений
		}

	__HAL_TIM_CLEAR_IT(htim, TIM_IT_CC1);
}

/**
  * @brief  Обработчик прерываний базового таймера (для работы входов в режиме с базовым таймером): сохраняет значение таймеров входов и перезагружает их
  */
void pulse_in_base_irq(void)
{
#ifdef PULSE_IN_BASE
uint8_t chan;

	for(chan=0; chan<PULSE_IN_NUMBER; chan++) //перебираем все каналы
		{
		if (pulse_in_base_mode[chan])		//если канал работает в режиме "с базовым таймером"
			{
			pulse_in_cmp[chan]=__HAL_TIM_GET_COUNTER(pulse_in_tim_point[chan]); //сохранить количество импульсов
			__HAL_TIM_SET_COUNTER(pulse_in_tim_point[chan], 0);					//обнулить счётчик импульсов на входе
			if (pulse_in_cmp[chan]<3) //если частота базового таймера меньше частоты импульсов менее чем в 2 раза
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
  * @brief  Предопределённая функция обработки прерывания по перезагрузки регистра счёта для всех таймеров
  * 		НЕ РАБОТАЕТ НОРМАЛЬНО, Т,К, ЕСТЬ ОШИБКА В HAL ИЗ-ЗА КОТОРОЙ НЕ ВСЕГДА СБРАСЫВАЕТСЯ ФЛАГ В РЕГИСТРЕ SR.
  * 		Поэтому сделано на обработчиках прерывания
  *
  * @param  *htim: указатель на описание таймера вызвавшего прерывание
  */
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{uint8_t chan=0;
//
//	__HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
//
//#ifdef PULSE_IN_BASE
//	if (htim==&PULSE_IN_BASE)
//		{
//		for(chan=0; chan<PULSE_IN_NUMBER; chan++) //перебираем все каналы
//			{
//			if (pulse_in_base_mode[chan])		//если канал работает в режиме "с базовым таймером"
//				{
//				pulse_in_cmp[chan]=__HAL_TIM_GET_COUNTER(pulse_in_tim_point[chan]); //сохранить количество импульсов
//				__HAL_TIM_SET_COUNTER(pulse_in_tim_point[chan], 0);					//обнулить счётчик импульсов на входе
//				if (pulse_in_cmp[chan]<3) //если частота базового таймера меньше частоты импульсов менее чем в 2 раза
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
//	while ((htim!=pulse_in_tim_point[chan])&&(chan<PULSE_IN_NUMBER)) chan++; //ищем номер таймера вызвавшего прерывание
//
//	if ((chan>=PULSE_IN_NUMBER)||(pulse_in_base_mode[chan]==1)) return; //если не нашли нужного канала или найденный канал настройен на режим базового таймера
//
//	if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC1)==0) //если прерывание произошло из-за переполнениия счётчика (флаг сравнения ==0)
//		{
//		pulse_in_ovr_tmp[chan]++;	//увеличить временный счётчик переполнений
//		if (pulse_in_ovr_tmp[chan]>pulse_in_ovr[chan]) //если счётчик переполнений превысил предыдущие значение, значит частота уменшилась
//			{
//			pulse_in_ovr[chan]=pulse_in_ovr_tmp[chan]; //обновить значение счётчика переполнений
//			pulse_in_cmp[chan]=0; //обнулить остаток счётчика
//			}
//		}
//	else	//если прерывание произошло из-за input compare (обнаружен фронт импульса): измерение закончено
//		{
//		pulse_in_cmp[chan]=__HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1); 	//перизаписать остаток счётчика
//		pulse_in_ovr[chan]=pulse_in_ovr_tmp[chan];						//перезаписать счётчик переполнений
//		pulse_in_ovr_tmp[chan]=0;										//обнулить временный счётчик переполнений
//		}
//
//	__HAL_TIM_CLEAR_IT(htim, TIM_IT_CC1);
//}
