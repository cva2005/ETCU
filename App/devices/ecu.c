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

void SaveEngineTemp (PGN_65262_t* data) {
	Data[0] = data->Coolant_T * 1000;
	//set(AO_PC_ECU_08, data->Coolant_T * 1000);
}

void SaveEngineLP (PGN_65263_t* data) {
	Data[1] = data->Oil_P * 1000;
	//set(AO_PC_ECU_09, data->Oil_P * 1000);
}

void SaveFuelEconomy (PGN_65266_t* data) {
}

void SaveInletExhaust (PGN_65270_t* data) {
}

void SaveFuelLevel (int8_t lev) {
	Data[2] = lev * 1000;
	//set(AO_PC_ECU_10, lev * 1000);
}

void SaveAirFlow (int16_t flow) {
	float32_t f = (float32_t)flow * FLOW_WEIGHT;
	Data[3] = (int32_t)(f * 1000.0);
	//set(AO_PC_ECU_11, (int32_t)(f * 1000.0));
}

int32_t ecu_get_data (uint8_t ch) {
	return Data[ch];
}

