#include "can_1.h"
#include "J1939.h"
#include "timers.h"
#include "ecu.h"

void j1939Receive (uint8_t* data, uint8_t len, J1939_ID_t* id);
static stime_t err_time, eh_time, sa_time;
static bool time_out = false;
static const int8_t AddrTabl[] = {
		ADDR_TECU_1, ADDR_TECU_2, ADDR_CUMMINS, ADDR_BR_CNTR, ADDR_CRUISE,
		ADDR_VOLVO, ADDR_DETROIT, ADDR_SA_232, ADDR_SA_233, ADDR_VOLVO_M,
		ADDR_SA_235, ADDR_SA_236, ADDR_SA_237, ADDR_SA_238, ADDR_SA_239,
		ADDR_SA_240, ADDR_SA_241, ADDR_SA_242, ADDR_SA_243, ADDR_SA_244,
		ADDR_SA_245, ADDR_SA_246, ADDR_SA_247};
#define ADDR_MAX sizeof(AddrTabl) / sizeof(typeof(AddrTabl[0]))
static uint32_t SrcAddr = 0;
static bool CrcUse = false;
static bool AddrSel = false;
static bool CheckAddr = false;
static uint8_t MessCnt;

void canJ1939_init (void) {
	//can_1_set_filter32(PGN_61443, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	//can_1_set_filter32(PGN_65243, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	//can_1_set_filter32(PGN_65247, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	//can_1_set_filter32(PGN_65276, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	//can_1_set_filter32(PGN_60928, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	//can_1_set_filter32(PGN_61450, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_61444, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65253, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65262, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65263, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65266, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	can_1_set_filter32(PGN_65270, 0, 0xffff000, 1, CAN_FILTERMODE_IDMASK);
	err_time = timers_get_finish_time(J1939_ERR_TIME);
	eh_time = timers_get_finish_time(E_HOURS_TIME);
}

bool j1939TSC1active (void) {
	return ((SrcAddr != 0) && AddrSel);
}

bool j1939TSC1error (void) {
	return ((SrcAddr == 0) && AddrSel);
}

void j1939TSC1reset (void) {
	SrcAddr = 0;
	AddrSel = false;
	CrcUse = false;
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
	if (!AddrSel && CheckAddr) {
		if (timers_get_time_left(sa_time) == 0) {
			CheckAddr = false;
			int8_t addr = GetCurrSA();
			if (AddrTabl[SrcAddr] == addr) {
				SrcAddr = addr;
				AddrSel = true;
			} else {
				SrcAddr++;
				if (SrcAddr == ADDR_MAX) {
					SrcAddr = 0; // SA not found address!
					if (CrcUse == false) {
						CrcUse = true;
					} else {
						AddrSel = true; // SA not found sign!
					}
				}
			}
		}
	}
}

void j1939Receive (uint8_t* data, uint8_t len, J1939_ID_t* id) {
	switch (id->PGN.FULL) {
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
	case 61444: // Electronic Engine Controller 1 (25-50 мсек)
		SaveEngineCtr1((PGN_61444_t *)data);
		break;
#if 0
	case 61450: // Engine Gas Flow Rate (по запросу?)
		SaveAirFlow(((PGN_61450_t *)data)->AirMassFlow);
		break;
	case 61443: // Electronic Engine Controller 2 (50 мсек)
		SavePedalPosition(((PGN_61443_t *)data)->PdPos1);
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

uint8_t HoursRequest (void) {
	J1939_ID_t id;
	id.PGN.FULL = ADDR_CL_REQ;
	id.SA = ADDR_HOURS;
	uint32_t id_32 = *(uint32_t *)&id;
	id_32 |= PRIORITY_DEFAULT << 26;
	uint32_t req_pgn = ENG_HOURS;
	return can_1_write_tx_data(id_32, 3, (uint8_t *)&req_pgn);
}

uint8_t TorqueSpeedControl (uint8_t trq, uint16_t spd) {
	PGN_00000_t pgn; J1939_ID_t id; uint8_t cc;
	id.PGN.FULL = TSC1; // Torque/Speed Control 1
	if (!AddrSel) {
		if (!CheckAddr) {
			sa_time = timers_get_finish_time(SA_VALID_TIME);
			CheckAddr = true;
		}
		id.SA = AddrTabl[SrcAddr];
	} else id.SA = SrcAddr;
	uint32_t id_32 = *((uint32_t *)&id);
	id_32 |= PRIORITY_TSC1 << 26;
	pgn.ControlMode = SpeedControl;
	pgn.RequestedSpeed = spd;
	pgn.RequestedTorque = trq;
	pgn.ModePriority = HighestPriority;
	pgn.TransmissionRate = VAL_TX_RATE;
	pgn.ControlPurpose = PURP_OPER;
	if (++MessCnt > MESS_CNT_MAX) MessCnt = 0;
	pgn.MessageCounter = MessCnt;
	if (CrcUse) {
		int checksum = 0, i;
		for (i = 0; i < (sizeof(pgn) - 1); i++) {
			checksum += ((uint8_t *)&pgn)[i];
		}
		checksum += MessCnt;
		for (i = 0; i < sizeof(id); i++) {
			checksum += ((uint8_t *)&id)[i];
		}
		pgn.MessageChecksum = (((checksum >> 6) & 0x03) + (checksum >>3) + checksum) & 0x07;
	} else {
		for (int i = 5; i < sizeof(pgn); i++) {
			((uint8_t *)&pgn)[i] = 0xff;
		}
	}
	trq -= TORQUE_OFFSET;
	if (trq == 0) {
		cc = DisDrivelineNonLockup;
	} else if (trq < 25) {
		cc = DisDrivelineNonLockup1;
	} else if (trq < 70) {
		cc = EnDrivelineInLockup;
	} else {
		cc = EnDrivelineInLockup1;
	}
	pgn.ControlConditions = cc; // Условия регулирования скорости (зависит от момента)
	return can_1_write_tx_data(id_32, sizeof(PGN_00000_t), (uint8_t *)&pgn);
}

bool J1939_error (void) {
	return time_out;
}
