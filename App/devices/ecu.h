#ifndef APP_DEVICES_ECU_H_
#define APP_DEVICES_ECU_H_
#include "arm_math.h"
#include "J1939.h"

#define ECU_CH				7
#define DAC_OUT_MAX			500 // максимальное значение выхода == 5В
#define DAC_FACT			0.060f
#define SPEED_RESOL			0.125f // rpm
#define TORQUE_LIM			125.0f // %
#define FLOW_WEIGHT			0.05f // kg/h per bit
#define RATE_WEIGHT			0.05f // 0.05 L/h per bit
#define F_ECON_WEIGHT		512.0f // 1/512 km/L per bit
#define OIL_P_WEIGHT		4.0f // 4 kPa/bit
#define OIL_T_WEIGHT		0.03125f // 0.03125 deg C/bit
#define AIR_P_WEIGHT		2.0f // 2 kPa/bit
#define AIR_T_WEIGHT		1.0f // 1 deg C/bit
#define TSC1_STOP_RETRY		20

bool ControlState (void);
uint8_t EcuTSC1Control (float32_t spd, float32_t trq);
void EcuPedControl (float32_t out);
void SavePedalPosition (int8_t th);
void SaveEngineTemp (PGN_65262_t* data);
void SaveEngineLP (PGN_65263_t* data);
void SaveFuelRate (PGN_65266_t* data);
void SaveInletExhaust (PGN_65270_t* data);
void SaveAirFlow (int16_t flow);
int32_t EcuPedalPos (void);
uint8_t EcuPedError (void);
int32_t ecu_get_data (uint8_t ch);
#if 0
void SaveFuelLevel (int8_t lev);
void SaveEngineSpeed (int16_t pos);
void SaveRailPressure (int16_t press);
void SaveTorqPercent (int8_t perc);
void SaveEngineHours (PGN_65253_t* data);
#endif

#endif /* APP_DEVICES_ECU_H_ */
