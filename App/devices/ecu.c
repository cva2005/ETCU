#include <string.h>
#include "timers.h"
#include "types.h"
#include "_signals.h"
#include "_control.h"
#include "mu110_6U.h"
#include "ecu.h"

extern sg_t sg_st; //состояние сигналов
int32_t ServoPos = 0;
#if 0
//датчики температуры ds18b20
sg_st.etcu.i.a[ETCU_AI_TEMP1]=ds18b20_get_temp(1);
sg_st.etcu.i.a[ETCU_AI_TEMP2]=ds18b20_get_temp(2);
sg_st.etcu.i.a[ETCU_AI_TEMP3]=ds18b20_get_temp(3);
sg_st.etcu.i.a[ETCU_AI_TEMP4]=ds18b20_get_temp(4);
sg_st.etcu.i.a[ETCU_AI_TEMP5]=ds18b20_get_temp(5);
sg_st.etcu.i.a[ETCU_AI_TEMP6]=ds18b20_get_temp(6);
sg_st.etcu.i.a[ETCU_AI_TEMP7]=ds18b20_get_temp(7);
sg_st.etcu.i.a[ETCU_AI_TEMP8]=ds18b20_get_temp(8);
#endif

void EcuControl (float32_t out) {
	uint16_t data = (uint16_t)(out * DAC_FACT);
	if (data > DAC_OUT_MAX) data = DAC_OUT_MAX; // 0...5В
	mu6u_set_out(data); // уст. выходы DAC0, DAC1
	ServoPos = data * (1000 / DAC_OUT_MAX) * 100; // положение сервопривода %
}

uint8_t EcuGetError (void) {
	return mu6u_err_link();
}

int32_t EcuServoPos (void) {
	return ServoPos;
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
	sg_st.etcu.i.a[ETCU_AI_TEMP1] = data->Coolant_T * 1000;
}

void SaveEngineLP (PGN_65263_t* data) {
	sg_st.etcu.i.a[ETCU_AI_TEMP2] = data->Oil_P * 1000;
}

void SaveFuelEconomy (PGN_65266_t* data) {
}

void SaveInletExhaust (PGN_65270_t* data) {
}

void SaveFuelLevel (int8_t lev) {
	sg_st.etcu.i.a[ETCU_AI_TEMP3] = lev * 1000;
}
