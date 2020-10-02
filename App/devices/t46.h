#ifndef APP_DEVICES_T46_H_
#define APP_DEVICES_T46_H_
#include "arm_math.h"
#include "modbus.h"

#define T46_TX_TIME		300
#define T46_MAX_ERR_SEND		3	// Максимально допустимое количество запросов без ответа
#define RESULT_ALL		0 // 5 регистров: момент, частота, температура
//#define RES_REG_NUM		5 // 5 регистров: момент, частота, температура
#define RES_REG_NUM		4 // 4 регистра: момент, частота
#define CONFIG_ADDR		0 // адрес регистра "Флаги конфигурации"
#define CONFIG_WORD		0x0009 // слово с регистрами флагов: StartStop + UsingFloat
#define POWER_FACTOR	9.549 // коэффициент для вычисления мощности

void t46_init (uint8_t ch, uint8_t adr); // Инициализация датчика
void t46_step (void);				// Шаг обработки данных датчика
uint8_t t46_err_link (void);		// состояние связи
int32_t t46_get_torque (void);		// значение крутящего момента
int32_t t46_get_temperature (void);	// значение температуры
int32_t t46_get_freq (void);		// значение оборотов
int32_t t46_get_power (void);		// значение мощность
void t46_update_data (char *data, uint8_t len, uint8_t adr, uint8_t function); // Обработчик пакетов принятых по ModBus
extern float32_t Speed_Out, Torque_Out;

typedef struct {
	int32_t torque;
	int32_t freq;
	int32_t temperature;
	int32_t power;
} t46_rx_data_t;

typedef enum {
	READ_COMPLEX = MODBUS_READ_INPUTS_REGISTERS,
	START_MEASURING = MODBUS_FORCE_SINGLE_REGISTER,
} t46_modbus_function_t;

uint8_t t46_start_measure(void);

#endif /* APP_DEVICES_T46_H_ */
