#include <stdbool.h>
#include "can_1.h"
#include "J1939.h"
#include "timers.h"
#include "ecu.h"

void j1939Receive (uint8_t* data, uint8_t len, J1939_ID_t id);

void canJ1939_init (void) {
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
		if (can_1_get_rx_size()) { // ���� ������
			read_st = can_1_read_rx_data(&can_id, &can_length, can_msg);
		}
		if (read_st) { //���� ���� ��������� ������
			j1939Receive(can_msg, can_length, *((J1939_ID_t *)&can_id));
		}
	} while (read_st);
}

void j1939Receive (uint8_t* data, uint8_t len, J1939_ID_t id) {
	switch (id.PGN) {
	case 61443: // Electronic Engine Controller 2 (50 ����)
		SavePedalPosition(((PGN_61443_t *)data)->PdPos1);
		break;
	case 61444: // Electronic Engine Controller 1 (25-50 ����)
		SaveEngineSpeed(((PGN_61444_t *)data)->EngineSpeed);
		break;
	case 65243: // Engine Fluid Level/Pressure 2 (500 ����)
		SaveRailPressure(((PGN_65243_t *)data)->TimingRail_1_P);
		break;
	case 65247: // Nominal Friction - Percent Torque (250 ����)
		SaveTorqPercent(((PGN_65247_t *)data)->NominalFriction);
		break;
	case 65253: // Engine Hours, Revolutions (�� �������)
		SaveEngineHours((PGN_65253_t *)data);
		break;
	case 65262: // Engine Temperature 1 (1000 ����)
		SaveEngineTemp((PGN_65262_t *)data);
		break;
	case 65263: // Engine Fluid Level/Pressure 1 (500 ����)
		SaveEngineLP((PGN_65263_t *)data);
		break;
	case 65266: // Fuel Economy (Liquid) (100 ����)
		SaveFuelEconomy((PGN_65266_t *)data);
		break;
	case 65270: // Inlet/Exhaust Conditions 1 (500 ����)
		SaveInletExhaust((PGN_65270_t *)data);
		break;
	case 65276: // Dash Display (1000 ����)
		SaveFuelLevel(((PGN_65276_t *)data)->Fuel_L1);
		break;
	}
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