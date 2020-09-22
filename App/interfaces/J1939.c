#include "can_1.h"
#include "J1939.h"
#include "timers.h"
#include "ecu.h"
#include "_control.h"

void j1939Receive (uint8_t* data, uint8_t len, J1939_ID_t* id);
static stime_t err_time;
static bool time_out;

void canJ1939_init (void) {
	//can_1_set_filter32(PGN_00000, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_61443, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_61444, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65243, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65247, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65253, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65262, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65263, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65266, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65270, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65276, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
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
		for (uint32_t i = 0; i < AO_PC_ECU_05 - AO_PC_ECU_01; i++)
			set(AO_PC_ECU_01 + i, ERROR_CODE);
	}
}

void j1939Receive (uint8_t* data, uint8_t len, J1939_ID_t* id) {
	switch (id->PGN) {
	case 61443: // Electronic Engine Controller 2 (50 мсек)
		SavePedalPosition(((PGN_61443_t *)data)->PdPos1);
		break;
	case 61444: // Electronic Engine Controller 1 (25-50 мсек)
		SaveEngineSpeed(((PGN_61444_t *)data)->EngineSpeed);
		break;
	case 61450: // Electronic Engine Controller 1 (50 мсек)
		SaveAirFlow(((PGN_61450_t *)data)->AirMassFlow);
		break;
	case 65243: // Engine Fluid Level/Pressure 2 (500 мсек)
		SaveRailPressure(((PGN_65243_t *)data)->TimingRail_1_P);
		break;
	case 65247: // Nominal Friction - Percent Torque (250 мсек)
		SaveTorqPercent(((PGN_65247_t *)data)->NominalFriction);
		break;
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
		SaveFuelEconomy((PGN_65266_t *)data);
		break;
	case 65270: // Inlet/Exhaust Conditions 1 (500 мсек)
		SaveInletExhaust((PGN_65270_t *)data);
		break;
	case 65276: // Dash Display (1000 мсек)
		SaveFuelLevel(((PGN_65276_t *)data)->Fuel_L1);
		break;
	}
}

uint8_t j1939Transmit (uint8_t* data, uint8_t len, J1939_ID_t id) {
	return can_1_write_tx_data(*((uint32_t *)&id), len, data);
}

uint8_t TorqueSpeedControl (int8_t trq, uint16_t spd) {
	PGN_00000_t png; J1939_ID_t id; uint8_t cc;
	for (cc = 0; cc < sizeof(png); cc++) *((uint8_t *)&png + cc) = 0;
	id.P = 03; // Priority
	id.R = 0; // Should always be set to 0 when transmitting messages
	id.PGN = TSC1_PGN; // Torque/Speed Control 1
	id.SA = Transmission1;
	png.ControlMode = SpeedControl;
	png.RequestedSpeed = spd;
	png.RequestedTorque = trq;
	png.ModePriority = HighestPriority;
	if (trq <= 0) {
		cc = DisDrivelineNonLockup;
	} else if (trq < 25) {
		cc = DisDrivelineNonLockup1;
	} else if (trq < 70) {
		cc = EnDrivelineInLockup;
	} else {
		cc = EnDrivelineInLockup1;
	}
	png.ControlConditions = cc; // ToDo: зависит от нагрузки
	return j1939Transmit ((uint8_t *)&png, sizeof(PGN_00000_t), id);
}

bool J1939_error (void) {
	return time_out;
}

#if 0
bool j1939PeerToPeer (uint32_t lPGN) {
	if (lPGN > 0 && lPGN <= 0xEFFF)
		return true;
	if (lPGN > 0x10000 && lPGN <= 0x1EFFF)
		return true;
	return false;
}

uint8_t j1939Transmit (uint32_t lPGN, uint8_t nPriority, uint8_t nSrcAddr,
					   uint8_t nDestAddr, uint8_t* nData, uint8_t nDataLen) {
	uint32_t lID = ((uint32_t)nPriority << 26) + (lPGN << 8) + (uint32_t)nSrcAddr;

	// If PGN represents a peer-to-peer, add destination address to the ID
	if (j1939PeerToPeer(lPGN) == true) {
		lID = lID & 0xFFFF00FF;
		lID = lID | ((uint32_t)nDestAddr << 8);
	}
	return canTransmit(lID, nDataLen, nData);
}

uint8_t j1939Receive (uint32_t* lPGN, uint8_t* nPriority, uint8_t* nSrcAddr,
					  uint8_t* nDestAddr, uint8_t* nData, uint8_t* nDataLen) {
	uint8_t nRetCode = 1;
	uint32_t lID;
	*nSrcAddr = 255;
	*nDestAddr = 255;
	if (canReceive(&lID, nData, nDataLen) == 0) {
		uint8_t lPriority = lID & 0x1C000000;
		*nPriority = (uint32_t)(lPriority >> 26);
		 *lPGN = lID & 0x00FFFF00;
		 *lPGN = *lPGN >> 8;
		 lID = lID & 0x000000FF;
		 *nSrcAddr = (uint32_t)lID;
		 if (j1939PeerToPeer(*lPGN) == true) {
			 *nDestAddr = (uint32_t)(*lPGN & 0xFF);
			 *lPGN = *lPGN & 0x01FF00;
		 }
		 nRetCode = 0;
	 }
	 return nRetCode;
}
#endif
