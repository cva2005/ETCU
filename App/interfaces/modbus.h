/*
 * modbus.h
 *
 *  Created on: 29 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */

#ifndef APP_INTERFACES_MODBUS_H_
#define APP_INTERFACES_MODBUS_H_
#include "types.h"

typedef void (*modbus_rx_t) (char *data, uint8_t len, uint8_t adr, uint8_t function); //описание процедуры - обработчика принятых команд ModBus
typedef uint32_t (*rx_data_t) (void* buf, uint32_t size);// Принимает данные из RS485
typedef uint32_t (*rx_size_t) (void);  // Возвращает количество байт данных
typedef uint32_t (*tx_data_t) (void* buf, uint32_t size); // Отправляет данные в RS485
typedef enum {
	Low_pr = 0,
	Hi_pr = 1
} pr_t;

#define CH1						0 // раздельных сетей (мастеров) ModBus
#define CH2						1 // раздельных сетей (мастеров) ModBus
#define MB_NET					2 // раздельных сетей (мастеров) ModBus
#define MB_BL					256 // размер буфера приема
#define MODBUS_MAX_DEV			6 // устройств ModBus 1
#define MODBUS2_MAX_DEV 		6 // устройств ModBus 2
#define MODBUS_MAX_WAIT_TIME 	1000 // время ожидания ответа от slave устройства
#define MODBUS2_MAX_WAIT_TIME 	200 // время ожидания ответа от slave устройства
#define MODBUS_BYTE_RX_PAUSE 	5 // допустимая пауза между принятыми байтами
#define MB_TX_BUFF				20

void modbus_init (void); //Инициализация интерфейса ModBus
void modbus_step (void); //шаг обмена ModBus: необходимо добавить в основной цикл
uint8_t modbus_rd_in_reg (uint8_t ch, uint8_t adr, uint16_t reg, uint16_t number); // Чтение одного или нескольких входных регистров
uint8_t modbus_rd_hold_reg (uint8_t ch, uint8_t adr, uint16_t reg, uint16_t number); // Чтение одного или нескольких регистров хранения
uint8_t modbus_wr_1reg (uint8_t ch, uint8_t adr, uint16_t reg, uint16_t val); //Запись регистра хранения
uint8_t modbus_wr_mreg (uint8_t ch, uint8_t adr, uint16_t reg, uint16_t num, uint8_t* data); //Запись регистров хранения
uint8_t modbus_user_function (uint8_t ch, uint8_t adr, uint8_t func, uint8_t ln, uint8_t *data);//Функция: Пользовательская функция
uint8_t modbus_get_busy (uint8_t ch, uint8_t adr, pr_t pr); //Возвращает готовность ModBus к передаче данных
extern modbus_rx_t* pf_rx[MB_NET]; //указатели на функции обработчики пакетов от устйроств ModBus

typedef enum
{
	MODBUS_READ_COIL_STATUS=1,
	MODBUS_READ_INPUT_STATUS=2,
	MODBUS_READ_HOLDING_REGISTERS=3,
	MODBUS_READ_INPUTS_REGISTERS=4,
	MODBUS_FORCE_SINGLE_COIL=5,
	MODBUS_FORCE_SINGLE_REGISTER=6,
	MODBUS_READ_EXCEPTION_STATUS=7,
	MODBUS_LOOPBACK_DIAGNOSTIC_TEST=8,
	MODBUS_FETCH_EVENT_COUNTER=11,
	MODBUS_FETCH_COMMUNICATION=12,
	MODBUS_PROGRAM=13,
	MODBUS_POLL_PROGRAM_COMPLETE=14,
	MODBUS_FORCE_MULTIPLE_COILS=15,
	MODBUS_FORCE_MULTIPLE_REGISTERS=16,
	MODBUS_REPORT_SLAVE=17,
	MODBUS_RESET_COMMUNACATION_LINK=19
}modbus_function_t;

#define GET_UINT16(addr)      (((uint16_t)((uint8_t *)(addr))[0] << 8) | ((uint8_t *)(addr))[1])
#define SET_UINT16(addr, val) do { ((uint8_t *)(addr))[0] = ((val) >> 8) & 0xFF; \
                                   ((uint8_t *)(addr))[1] = (val) & 0xFF; } while (0)
#define SWAP16(x)             ( (((x) & 0xFF) << 8) | (((x) >> 8) & 0xFF) )

#endif /* APP_INTERFACES_MODBUS_H_ */
