#ifndef APP_DEVICES_ECU_H_
#define APP_DEVICES_ECU_H_
#include "arm_math.h"
#include "J1939.h"

#define SPEED_LOOP_TIME		TSC1_TX_RATE
#define ECU_CH				10
#define DAC_OUT_NULL		65  // минимальное значение выхода: 5В * 13%
#define DAC_OUT_MIN			120  // значение выхода в режиме ХХ: 5В * 25%
#define DAC_OUT_MAX			445 // максимальное значение выхода: 5В * 89%
#define DAC_OUT_DIFF		(DAC_OUT_MAX - DAC_OUT_NULL)
#define DAC_FACT			0.0456f
#define SPEED_RESOL			0.125f // rpm
#define TORQUE_LIM			125.0f // %
#define FLOW_WEIGHT			0.05f // kg/h per bit
#define RATE_WEIGHT			0.05f // 0.05 L/h per bit
#define F_ECON_WEIGHT		512.0f // 1/512 km/L per bit
#define OIL_P_WEIGHT		4.0f // 4 kPa/bit
#define OIL_T_WEIGHT		0.03125f // 0.03125 deg C/bit
#define OIL_T_OFFSET		273.0f
#define AIR_P_WEIGHT		2.0f // 2 kPa/bit
#define AIR_T_WEIGHT		1.0f // 1 deg C/bit
#define AIR_T_OFFSET		40.0f
#define TSC1_STOP_RETRY		20
#define FUEL_DENSITY		0.850f // плотность ДТ
#define ENGINE_H_WEIGHT		0.05f // 0.05 hr/bit
#define OIL_C_OFFSET		40
#define TORQUE_OFFSET		125
#define SPD_MAX				2850.0f
#define SPD_MIN				700.0f
#define SPD_DIFF			(SPD_MAX - SPD_MIN)

void EcuInit (void);
bool ControlState (void);
uint8_t EcuTSC1Control (float32_t spd, float32_t trq);
void EcuPedControl (float32_t out, bool start);
void SavePedalPosition (int8_t th);
void SaveEngineCtr1 (PGN_61444_t * data);
void SaveEngineTemp (PGN_65262_t* data);
void SaveEngineLP (PGN_65263_t* data);
void SaveFuelRate (PGN_65266_t* data);
void SaveInletExhaust (PGN_65270_t* data);
void SaveAirFlow (int16_t flow);
void SaveEngineHours (PGN_65253_t* data);
int32_t EcuPedalPos (void);
uint8_t EcuPedError (void);
int32_t ecu_get_data (uint8_t ch);
void SaveEngineSpeed (int16_t pos);
float32_t GetEcuSpeed (void);
int8_t GetCurrSA (void);

#if 0
void SaveFuelLevel (int8_t lev);
void SaveRailPressure (int16_t press);
void SaveTorqPercent (int8_t perc);
#endif

#endif /* APP_DEVICES_ECU_H_ */
