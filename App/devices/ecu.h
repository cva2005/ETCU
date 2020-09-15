#ifndef APP_DEVICES_ECU_H_
#define APP_DEVICES_ECU_H_
#include "arm_math.h"
#include "J1939.h"

#define DAC_OUT_MAX			500 // максимальное значение выхода == 5В
#define DAC_FACT			0.060f
#define SPEED_RESOL			0.125f // rpm
#define TORQUE_LIM			125.0f // %

uint8_t EcuTSC1Control (float32_t spd, float32_t trq);
void EcuPedControl (float32_t out);
void SaveEngineHours (PGN_65253_t* data);
void SavePedalPosition (int8_t th);
void SaveEngineSpeed (int16_t pos);
void SaveTorqPercent (int8_t perc);
void SaveRailPressure (int16_t press);
void SaveEngineTemp (PGN_65262_t* data);
void SaveEngineLP (PGN_65263_t* data);
void SaveFuelEconomy (PGN_65266_t* data);
void SaveInletExhaust (PGN_65270_t* data);
void SaveFuelLevel (int8_t lev);
int32_t EcuPedalPos (void);
uint8_t EcuPedError (void);

#endif /* APP_DEVICES_ECU_H_ */
