#ifndef LA10PWM_H_
#define LA10PWM_H_

typedef enum {
	LA10P_POWERED  = 0,
	LA10P_STOP_ERR = 1,
	LA10P_NOT_INIT = 2,
	LA10P_INIT_RUN = 3,
	LA10P_READY    = 4
} la10p_st;

typedef enum {
	CH_FORW,
	CH_REVR
} pwm_ch;

/*
 * Линейный привод: 	LA10P 24В
 * Датчик положения: 	10кОм
 * Working duty: 		25%
 * Непрерывный Ток: 	4A
 * Выходная мощность: 	48W
 * Скорость линейная: 	20[mm/s]
 * Нагрузка: 			450N = 45кг
 * 150 мм 6-дюймовый ход
 * Рабочая температура -26 ~ + 65 градусов
 * IP Max IP 65
 */
#define ST_LENGT			150 // длина штока
#define ST_MOVE				20  // скорость перемещения штока [мм/сек]
#if MODEL_NO_SERVO
	#define STATE_SENS()	StateVal
#else
	#define STATE_SENS()	(float32_t)st(AI_T_OIL_OUT)
#endif
#define LA10P_FULL_TIME		(ST_LENGT * 1000 / ST_MOVE + 2000) // ms
#define LA10P_ERR_TIME		LA10P_FULL_TIME + 1000
#define LA10P_MIN_TIME		(LA10P_ERR_TIME - (LA10P_FULL_TIME / 4))
#define LA10P_MUL			6.50f
#define STEP_TIME			100 // время шага опроса привода, мс
#define CHECK_TIME			STEP_TIME * 20
#define CURR_LIMIT			1.20f
#define RELE_TIME			200 // время шага управления реле, мс
#define SENS_MAX_VAL		3000.0f // максимальное выходное значение, мВ
#define SENS_ERR_VAL		3300.0f // максимальное выходное значение, мВ
#define SENS_MIN_VAL		700.0f // минимальное выходное значение, мВ
#define SENS_I_Om			0.200f // output current sensor from the DRV8816
#define SENS_I_Gain			5.00f // Current-proportional output Gain DRV8816
#define CURR_SENS_mV		(float32_t)st(AI_P_MANIFOLD)
#ifdef MODEL_OBJ
#define CURR_VIEW(x)		set(AO_PC_P_MANIFOLD, (int32_t)x)
#else
#define CURR_VIEW(x)
#endif
#define CURR_SENS_mA		(CURR_SENS_mV / (SENS_I_Om * SENS_I_Gain))
#define SENS_I_MAX			100.0f

#define pwm_tim				htim1
#define TIM_INST			TIM1
#define GPIO_AF1_INST		GPIO_AF1_TIM1
#define TIM_CLK_ENABLE()	__TIM1_CLK_ENABLE();
#define PWM_R_PIN			GPIO_PIN_13 // reveres pin
#define PWM_F_PIN			GPIO_PIN_14 // forward pin
#define PWM_PORT			GPIOE
#define PWM_CH_FORW			TIM_CHANNEL_4 // (PE14 pin)
#define PWM_CH_REVR			TIM_CHANNEL_3 // (PE13 pin)
#define PWM_PRD_VAL			16384
#define FORW_DUTY			TIM_INST->CCR4
#define REVR_DUTY			TIM_INST->CCR3
#define DUTY_MIN			0.12f
#define DUTY_MAX			1.00f
#define ZONE_DEAD			0.01
#define FORW_MAX			DUTY_MAX
#define REVR_MAX			(-DUTY_MAX)
#define STOP_MOV			0.0f
#define FLOAT_MIN			0.0001
#define FORW_MOV			get_PWM_out() > FLOAT_MIN
#define REVR_MOV			get_PWM_out() < -FLOAT_MIN
#define FORW_LED(x)			set(DO_OIL_HEATER, x)
#define REVR_LED(x)			set(DO_COOLANT_HEATER, x)
#define SAFE_VAL			0.10f

extern float32_t StSens_K;
void la10p_init(void);
void la10p_step(void);
float32_t la10p_get_pos(void);
void la10p_set_out(float32_t pid_out);
la10p_st la10p_state(void);

#endif /* LA10PWM_H_ */
