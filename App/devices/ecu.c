#include <string.h>
#include "timers.h"
#include "types.h"
#include "mu110_6U.h"
#include "ecu.h"

int32_t PedalPos = 0;
static int32_t Data[ECU_CH];

uint8_t EcuTSC1Control (float32_t spd, float32_t trq) {
	uint16_t sp_w = (uint16_t)(spd / SPEED_RESOL);
	int8_t tq_b = (int8_t)(trq * TORQUE_LIM);
	return TorqueSpeedControl(tq_b, sp_w);
}

void EcuPedControl (float32_t out) {
	uint16_t data = (uint16_t)(out * DAC_FACT);
	if (data > DAC_OUT_MAX) data = DAC_OUT_MAX; // 0...5¬
	mu6u_set_out(data); // уст. выходы DAC0, DAC1
	PedalPos = data * ((1000 / DAC_OUT_MAX) * 100); // положение сервопривода %
}

uint8_t EcuPedError (void) {
	return mu6u_err_link();
}

int32_t EcuPedalPos (void) {
	return PedalPos;
}

void SaveEngineTemp (PGN_65262_t* data) {
	float32_t f = (float32_t)data->Oil_T * OIL_T_WEIGHT;
	Data[0] =  (int32_t)(f * 1000.0); // -273 to 1734.96875 deg C (0.03125 deg C/bit)
}

void SaveEngineLP (PGN_65263_t* data) {
	float32_t f = (float32_t)data->Oil_P * OIL_P_WEIGHT;
	Data[1] = (int32_t)(f * 1000.0); // 0 to 1000 kPa (4 kPa/bit)
	f = (float32_t)data->FuelDelivery_P * OIL_P_WEIGHT;
	Data[2] = (int32_t)(f * 1000.0); // 0 to 1000 kPa (4 kPa/bit)

}

void SaveAirFlow (int16_t flow) {
	float32_t f = (float32_t)flow * FLOW_WEIGHT;
	Data[3] = (int32_t)(f * 1000.0);
}

void SaveFuelRate (PGN_65266_t* data) {
	float32_t f = (float32_t)data->FuelRate * RATE_WEIGHT;
	Data[4] = (int32_t)(f * 1000.0); // 0 to 3,212.75 L/h (0.05 L/h per bit)
	f = (float32_t)data->InstantaneousEconomy / F_ECON_WEIGHT;
	Data[5] = (int32_t)(f * 1000.0); // 0 to 125.5 km/L (1/512 km/L per bit)
}

void SaveInletExhaust (PGN_65270_t* data) {
	float32_t f = (float32_t)data->AirInlet_P * AIR_P_WEIGHT;
	Data[6] = (int32_t)(f * 1000.0); // 0 to 500 kPa (2 kPa/bit)
	f = (float32_t)data->Manifold_1_T * AIR_T_WEIGHT;
	Data[7] = (int32_t)(f * 1000.0); // -40 to 210 deg C (1 deg C/bit)

}

#if 0
void SaveFuelLevel (int8_t lev) {
}

void SaveEngineHours (PGN_65253_t* data) {
}

void SavePedalPosition (int8_t pos) {
}

void SaveEngineSpeed (int16_t pos) {
}

void SaveTorqPercent (int8_t perc) {
}

void SaveRailPressure (int16_t press) {
}
#endif

int32_t ecu_get_data (uint8_t ch) {
	return Data[ch];
}

