/*
 * pulse_in.h
 *
 *  Created on: 14 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_DRIVERS_PULSE_IN_H_
#define APP_DRIVERS_PULSE_IN_H_
#include "stm32f4xx_hal.h"
/* ----ПРАВИЛЬНАЯ НАСТРОЙКА ТАЙМЕРОВ В STM32CubeMX----
 *	1: BASE_MODE==1: таймер работает от внешней CLK (clk импульсного входа), производится подсчёт количества импульсов на входе за период работы базового таймера
 *		Slave Mode: External Clock Mode 1
 *		Trigger Source: TI1_ED или ETR1 или TI1FP1 (зависит от ножки куда подключён импульсный вход)
 *		Остальное: Disable
 *		Prescaler=1 (для TI1_ED) Prescaler=0 (для ETR1)
 *		Counter Period = 0xFFFF
 *		IRQ - отключено
 *	2: BASE_MODE==0: таймер работает в режиме "input compare" - по переднему фронту сохраняет значение таймера и перезагружается (измерение периода между импульсами)
 *		Slave Mode: Reset Mode
 *		Trigger Source: TI1FP1 или TI2FP2 (зависит от ножки куда подключён импульсный вход)
 *		Clock Source: Internal Clock
 *		Channel1 или Channel2 (зависит от Trigger Source): Input Capture direct mode
 *		Остальное: Disable
 *		Prescaler = 0 - зависит от максимальной частоты на входе, чем  больше Prescaler, тем меньше максимум измеряемой частоты, но при этом реже срабатывает IRQ
 *		Counter Period = 0xFFFF
 *		Polarity: Rising Edge
 *		IC Selection: Direct
 *		Input Filter: 0
 *		Остальное: по умолчанию
 */

#define SYS_CLOCK_APB1 84	//частота APB1:значение взять из STM32CubeMX
#define SYS_CLOCK_APB2 168	//частота APB2:значение взять из STM32CubeMX

#define PULSE_IN_NUMBER 3	//количество импульсных входов

#if PULSE_IN_NUMBER>0		//настройка импульсного входа 1
	#define PULSE_IN_1 htim3		//таймер настроеный на импульсный вход 1
	#define PULSE_IN_1_BASE_MODE 1	//режим работы 1 - таймера работает в режиме счёта от импульсного входа (измерения относительно базового таймера) 0 - работа в режиме измерения периода (input compare)
#endif
#if PULSE_IN_NUMBER>1		//настройка импульсного входа 2
	#define PULSE_IN_2 htim4		//таймер настроеный на импульсный вход 2
	#define PULSE_IN_2_BASE_MODE 1	//режим работы 1 (с базовым таймером) или 0 (измерение периода)
#endif
#if PULSE_IN_NUMBER>2		//настройка импульсного входа 3
	#define PULSE_IN_3 htim9		//таймер настроеный на импульсный вход 3
	#define PULSE_IN_3_BASE_MODE 0	//режим работы 1 (с базовым таймером) или 0 (измерение периода)
#endif
#if PULSE_IN_NUMBER>3		//настройка импульсного входа 4
	#define PULSE_IN_4 htim		//таймер настроеный на импульсный вход 4
	#define PULSE_IN_4_BASE_MODE 0	//режим работы 1 (с базовым таймером) или 0 (измерение периода)
#endif
#if PULSE_IN_NUMBER>4		//настройка импульсного входа 5
	#define PULSE_IN_5 htim		//таймер настроеный на импульсный вход 5
	#define PULSE_IN_5_BASE_MODE 0	//режим работы 1 (с базовым таймером) или 0 (измерение периода)
#endif
#if PULSE_IN_NUMBER>5		//настройка импульсного входа 6
	#define PULSE_IN_6 htim		//таймер настроеный на импульсный вход 6
	#define PULSE_IN_6_BASE_MODE 0	//режим работы 1 (с базовым таймером) или 0 (измерение периода)
#endif

#if PULSE_IN_1_BASE_MODE||PULSE_IN_2_BASE_MODE||PULSE_IN_3_BASE_MODE\
	||PULSE_IN_4_BASE_MODE||PULSE_IN_5_BASE_MODE||PULSE_IN_6_BASE_MODE\
	||PULSE_IN_7_BASE_MODE||PULSE_IN_8_BASE_MODE||PULSE_IN_9_BASE_MODE\
	||PULSE_IN_10_BASE_MODE||PULSE_IN_11_BASE_MODE||PULSE_IN_12_BASE_MODE\
	||PULSE_IN_13_BASE_MODE||PULSE_IN_14_BASE_MODE

#define PULSE_IN_BASE_NUMBER 6 //номер базового таймера
#endif

#define PULSE_MAX_OVERLOAD 65535 //максимальное допустимое число переполнений таймера в режиме измерения периода (input compare)

void pulse_in_init(uint8_t channel);	//Инициалицирует импульсный вход
uint32_t pulse_in_sec(uint8_t chan);	//Возвращает количество импульсов в секунду (Гц)
uint32_t pulse_in_min(uint8_t chan);	//Возвращает количество импульсов в миниуту (среднее значение)
void pulse_in_capture_irq(TIM_HandleTypeDef *htim);//Обработчик прерываний для таймеров на входах со сравнением (input compare - в режиме имзерения периода между импульсами)
void pulse_in_base_irq(void);			//Обработчик прерываний базового таймера (для работы входов в режиме с базовым таймером)

//----------------------------------------------------------------------------------------------------------
#define TIM1_IRQ_PRE pulse_in_capture_irq(&htim1)
#define TIM2_IRQ_PRE pulse_in_capture_irq(&htim2)
#define TIM3_IRQ_PRE pulse_in_capture_irq(&htim3)
#define TIM4_IRQ_PRE pulse_in_capture_irq(&htim4)
#define TIM5_IRQ_PRE pulse_in_capture_irq(&htim5)
#define TIM6_IRQ_PRE pulse_in_capture_irq(&htim6)
#define TIM7_IRQ_PRE pulse_in_capture_irq(&htim7)
#define TIM8_IRQ_PRE pulse_in_capture_irq(&htim8)
#define TIM9_IRQ_PRE pulse_in_capture_irq(&htim9)
#define TIM10_IRQ_PRE pulse_in_capture_irq(&htim10)
#define TIM11_IRQ_PRE pulse_in_capture_irq(&htim11)
#define TIM12_IRQ_PRE pulse_in_capture_irq(&htim12)
#define TIM13_IRQ_PRE pulse_in_capture_irq(&htim13)
#define TIM14_IRQ_PRE pulse_in_capture_irq(&htim14)
#define TIM15_IRQ_PRE pulse_in_capture_irq(&htim15)
#define TIM16_IRQ_PRE pulse_in_capture_irq(&htim16)

#if PULSE_IN_1_BASE_MODE||PULSE_IN_2_BASE_MODE||PULSE_IN_3_BASE_MODE||PULSE_IN_4_BASE_MODE||PULSE_IN_5_BASE_MODE||PULSE_IN_6_BASE_MODE
	#if PULSE_IN_BASE_NUMBER==1
	#define PULSE_IN_BASE htim1
	#define TIM1_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM1
	#elif PULSE_IN_BASE_NUMBER==2
	#define PULSE_IN_BASE htim2
	#define TIM2_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM2
	#elif PULSE_IN_BASE_NUMBER==3
	#define PULSE_IN_BASE htim3
	#define TIM3_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM3
	#elif PULSE_IN_BASE_NUMBER==4
	#define PULSE_IN_BASE htim5
	#define TIM4_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM4
	#elif PULSE_IN_BASE_NUMBER==5
	#define PULSE_IN_BASE htim5
	#define TIM5_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM5
	#elif PULSE_IN_BASE_NUMBER==6
	#define PULSE_IN_BASE htim6
	#define TIM6_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM6
	#elif PULSE_IN_BASE_NUMBER==7
	#define PULSE_IN_BASE htim7
	#define TIM7_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM7
	#elif PULSE_IN_BASE_NUMBER==8
	#define PULSE_IN_BASE htim8
	#define TIM8_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM8
	#elif PULSE_IN_BASE_NUMBER==9
	#define PULSE_IN_BASE htim9
	#define TIM9_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM9
	#elif PULSE_IN_BASE_NUMBER==10
	#define PULSE_IN_BASE htim10
	#define TIM10_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM10
	#elif PULSE_IN_BASE_NUMBER==11
	#define PULSE_IN_BASE htim11
	#define TIM11_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM11
	#elif PULSE_IN_BASE_NUMBER==12
	#define PULSE_IN_BASE htim12
	#define TIM12_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM12
	#elif PULSE_IN_BASE_NUMBER==13
	#define PULSE_IN_BASE htim13
	#define TIM13_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM13
	#elif PULSE_IN_BASE_NUMBER==14
	#define PULSE_IN_BASE htim14
	#define TIM14_IRQ_POST 		pulse_in_base_irq() //обработчик прерывания TIM14
	#endif
#endif

#endif /* APP_DRIVERS_PULSE_IN_H_ */
