#include "can_1.h"
#include "J1939.h"
#include "timers.h"
#include "ecu.h"

void j1939Receive (uint8_t* data, uint8_t len, J1939_ID_t* id);
static stime_t err_time, eh_time;
static bool time_out;
static uint8_t mess_cnt;

void canJ1939_init (void) {
	//can_1_set_filter32(PGN_61443, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	//can_1_set_filter32(PGN_61444, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	//can_1_set_filter32(PGN_65243, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	//can_1_set_filter32(PGN_65247, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	//can_1_set_filter32(PGN_65276, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	//can_1_set_filter32(PGN_60928, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	//can_1_set_filter32(PGN_61450, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65253, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65262, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65263, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65266, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65270, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	err_time = timers_get_finish_time(J1939_ERR_TIME);
	eh_time = timers_get_finish_time(E_HOURS_TIME);
}

void J1939_step (void) {
	uint8_t read_st, can_msg[8], can_length;
	uint32_t can_id;
	do {
		read_st = 0;
		if (can_1_get_rx_size()) { // есть пакеты
			read_st = can_1_read_rx_data(&can_id, &can_length, can_msg);
		}
		if (read_st) { //если были прочитаны пакеты
			j1939Receive(can_msg, can_length, ((J1939_ID_t *)&can_id));
			time_out = false;
			err_time = timers_get_finish_time(J1939_ERR_TIME);
		}
	} while (read_st);
	if (timers_get_time_left(err_time) == 0) {
		time_out = true;
	}
	if (timers_get_time_left(eh_time) == 0) {
		eh_time = timers_get_finish_time(E_HOURS_TIME);
		HoursRequest();
	}
}

void j1939Receive (uint8_t* data, uint8_t len, J1939_ID_t* id) {
	switch (id->PGN.FULL) {
	/*case 61450: // Engine Gas Flow Rate (по запросу?)
		SaveAirFlow(((PGN_61450_t *)data)->AirMassFlow);
		break;*/
	case 65253: // Engine Hours, Revolutions (по запросу)
		SaveEngineHours((PGN_65253_t *)data);
		break;
	case 65262: // Engine Temperature 1 (1000 мсек)
		SaveEngineTemp((PGN_65262_t *)data);
		break;
	case 65263: // Engine Fluid Level/Pressure 1 (500 мсек)
		SaveEngineLP((PGN_65263_t *)data);
		break;
	case 65266: // Fuel Economy (Liquid) (100 мсек)
		SaveFuelRate((PGN_65266_t *)data);
		break;
	case 65270: // Inlet/Exhaust Conditions 1 (500 мсек)
		SaveInletExhaust((PGN_65270_t *)data);
		break;
#if 0
	case 61443: // Electronic Engine Controller 2 (50 мсек)
		SavePedalPosition(((PGN_61443_t *)data)->PdPos1);
		break;
	case 61444: // Electronic Engine Controller 1 (25-50 мсек)
		SaveEngineSpeed(((PGN_61444_t *)data)->EngineSpeed);
		break;
	case 65243: // Engine Fluid Level/Pressure 2 (500 мсек)
		SaveRailPressure(((PGN_65243_t *)data)->TimingRail_1_P);
		break;
	case 65247: // Nominal Friction - Percent Torque (250 мсек)
		SaveTorqPercent(((PGN_65247_t *)data)->NominalFriction);
		break;
	case 65276: // Dash Display (1000 мсек)
		SaveFuelLevel(((PGN_65276_t *)data)->Fuel_L1);
		break;
#endif
	}
}

uint8_t j1939Transmit (uint8_t* data, uint8_t len, J1939_ID_t id) {
	return can_1_write_tx_data(*((uint32_t *)&id), len, data);
}

uint8_t HoursRequest (void) {
	J1939_ID_t id;
	id.P = PRIORITY_DEFAULT; // Priority
	id.PGN.FULL = ADDR_CL_REQ;
	//id.PGN.FIELD.PS = ADDR_HOURS;
	id.SA = ADDR_HOURS;
	uint32_t req_pgn = ENG_HOURS;
	return j1939Transmit ((uint8_t*)&req_pgn, 3, id);
}

uint8_t TorqueSpeedControl (int8_t trq, uint16_t spd) {
	PGN_00000_t pgn; J1939_ID_t id; uint8_t cc;
	for (cc = 0; cc < sizeof(pgn); cc++) *((uint8_t *)&pgn + cc) = 0xff;
	id.P = PRIORITY_TSC1; // Priority
	id.PGN.FULL = TSC1; // Torque/Speed Control 1
	id.SA = SRC_ADDR;
	if (spd) {
		pgn.ControlMode = SpeedControl;
		pgn.RequestedSpeed = spd;
		pgn.RequestedTorque = trq;
		pgn.ModePriority = HighestPriority;
		pgn.TransmissionRate = VAL_TX_RATE;
		pgn.ControlPurpose = PURP_OPER;
		if (++mess_cnt > MESS_CNT_MAX) mess_cnt = 0;
		pgn.MessageCounter = mess_cnt;
#ifdef	TSC1_CHECKSUM
		int checksum = 0, i;
		for (i = 0; i < (sizeof(pgn) - 1); i++) {
			checksum += ((uint8_t *)&pgn)[i];
		}
		checksum += mess_cnt;
		for (i = 0; i < sizeof(id); i++) {
			checksum += ((uint8_t *)&id)[i];
		}
		pgn.MessageChecksum = (((checksum >> 6) & 0x03) + (checksum >>3) + checksum) & 0x07;
#else
		//pgn.MessageChecksum = NO_CHECK;
		for (int i = 3; i < sizeof(pgn); i++) {
			((uint8_t *)&pgn)[i] = 0xff;
		}
#endif
		if (trq <= 0) {
			cc = DisDrivelineNonLockup;
		} else if (trq < 25) {
			cc = DisDrivelineNonLockup1;
		} else if (trq < 70) {
			cc = EnDrivelineInLockup;
		} else {
			cc = EnDrivelineInLockup1;
		}
		pgn.ControlConditions = cc; // Условия регулирования скорости (зависит от момента)
	} // else png.ControlMode = OverrideDis;
	return j1939Transmit((uint8_t *)&pgn, sizeof(PGN_00000_t), id);
}

bool J1939_error (void) {
	return time_out;
}
