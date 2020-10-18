#include <string.h>
#include <stdbool.h>
#include "timers.h"
#include "types.h"
#include "mu110_6U.h"
#include "t46.h"
#include "ecu.h"

int32_t PedalPos = 0;
static int32_t Data[ECU_CH];
static bool TSC1state = false;

/*
spn1483 - Source Address of Controlling Device for Engine Control - The source address of the SAE J1939
device currently controlling the engine. It is used to expand the torque mode parameter (see SPN 899) in cases where control is in
response to an ECU that is not listed in Table SPN899_A. Its value may be the source address of the ECU transmitting the message
(which means that no external SAE J1939 message is providing the active command) or the source address of the SAE J1939 ECU that
is currently providing the active command in a TSC1 (see PGN 0) or similar message. Note that if this parameter value is the same as
the source address of the device transmitting it, the control may be due to a message on a non-J1939 data link such as SAE J1922 or a
proprietary link.
Data Length: 1 byte
Resolution: 1 source address/bit , 0 offset
Data Range: 0 to 255
Operating Range: 0 to 253
Type: Status
Suspect Parameter Number: 1483
Parameter Group Number: [61444]
This section defines the parameter groups for use on the SAE J1939 network. All
undefined bits are to be transmitted with a value of "1." All undefined bits should be received as "don't care" (either masked out or
ignored). This permits them to be defined and used in the future without causing any incompatibilities. Messages that are requesting
control over the receiving device (TSC1, TC1) are transmitted at high rate only during the time when the control is active, but may be
optionally sent at a slow rate as a "heartbeat." For TSC1, it is expected that the transmitting device indicate to the receiving device that
it no longer requests control by sending at least one broadcast with the override control modes set to 00. In the absence of continued
broadcasts from a requesting module, the receiving device shall default to its normal mode after two update periods. The size of the
CAN data field is 8 bytes. Parameter groups that are 0-8 data bytes in length use the services of the Data Link layer (Refer to SAE
J1939-21). Parameter groups that exceed 8 data bytes or parameter group definitions that are variable in length and may exceed 8 data
bytes shall utilize the services of the Transport Protocol. (Refer to 5.10 of SAE J1939-21.)
pgn0 - Torque/Speed Control 1 - TSC1 -
Transmission Repetition Rate: when active; 10 ms to engine - 50 ms to retarder
Data Length: 		8 bytes
Data Page: 			0
PDU Format: 		0
PDU Specific: 		DA
Default Priority: 	3
Parameter Group Number: 0 ( 000000 16 )
Bit Start Position /Bytes Length SPN Description 			SPN
1.1 2 bits 		Override Control Mode 						695
1.3 2 bits 		Requested Speed Control Conditions 			696
1.5 2 bits 		Override Control Mode Priority 				897
2-3 2 bytes 	Requested Speed/Speed Limit 				898
4 1 byte 		Requested Torque/Torque Limit 				518
NOTE - Retarder may be disabled by commanding a torque limit of 0%. Use of the limit mode allows the use of the retarder only up to
the limit specified in the request. This can be used to permit retarding of up to 50%, for example, if that limit is required by some device
such as an EBS, or it can disable the use of the retarder by others, as when an ABS controller detects wheel slip.
Engine Requested Speed Control Conditions: If status 10 is received, a more powerful regulator is used; otherwise a normal regulator
is used.
Override Control Mode Priority: If medium or low priority is received, the request from TSC1 can be de-prioritised
by Accelerator Pedal in the DLN1 message. If highest or high priority is received, the
normal accelerator pedal has no effect.
 */
uint8_t EcuTSC1Control (float32_t spd, float32_t trq) {
	uint16_t sp_w = (uint16_t)(spd / SPEED_RESOL);
	if (sp_w) TSC1state = true;
	else TSC1state = false;
	int8_t tq_b = (int8_t)(trq * TORQUE_LIM);
	return TorqueSpeedControl(tq_b, sp_w);
}

bool ControlState (void) {
	return TSC1state;
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
#ifdef ECU_DEBUG
	Data[4] = 23400; // Todo: ”брать!!!
	float32_t fp = (23.4 * FUEL_DENSITY) / t46_get_power();
	Data[7] = (int32_t)(fp * 1000.0); // ”д. эфф. расход топлива (кг/к¬т*ч)
#endif
}

void SaveAirFlow (int16_t flow) {
	float32_t f = (float32_t)flow * FLOW_WEIGHT;
	Data[3] = (int32_t)(f * 1000.0);
}

void SaveFuelRate (PGN_65266_t* data) {
	float32_t f = (float32_t)data->FuelRate * RATE_WEIGHT;
	Data[4] = (int32_t)(f * 1000.0); // 0 to 3,212.75 L/h (0.05 L/h per bit)
	float32_t fp = (f * FUEL_DENSITY) / t46_get_power();
	Data[7] = (int32_t)(fp * 1000.0); // ”д. эфф. расход топлива (кг/к¬т*ч)
}

void SaveInletExhaust (PGN_65270_t* data) {
	float32_t f = (float32_t)data->AirInlet_P * AIR_P_WEIGHT;
	Data[5] = (int32_t)(f * 1000.0); // 0 to 500 kPa (2 kPa/bit)
	f = (float32_t)data->Manifold_1_T * AIR_T_WEIGHT;
	Data[6] = (int32_t)(f * 1000.0); // -40 to 210 deg C (1 deg C/bit)
}

void SaveEngineHours (PGN_65253_t* data) {
	float32_t h = (float32_t)data->TotalHours * ENGINE_H_WEIGHT;
	Data[8] = (int32_t)(h); // Total Hours
	//Data[8] = 246900; // ToDo:
}

#if 0
void SaveFuelLevel (int8_t lev) {
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

