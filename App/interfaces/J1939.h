#ifndef J1939_H
#define J1939_H

#include "types.h"

typedef void (*J1939rxFun_t) (char *data, uint8_t len, uint32_t adr);
extern J1939rxFun_t J1939rxFun;

#pragma pack(1)
typedef struct J1939_ID {
	unsigned SA:	 8;
	unsigned PGN:	18;
	unsigned P:		 3;
	unsigned R:		 3;
} J1939_ID_t;
#pragma pack()

/*
558 ������ ��������� ��������� ���� ������ ������������ Accelerator Pedal 1 Low Idle Switch 50
91 ��������� ������ ������������ Accelerator Pedal Position 1
559 ������ ������� ������ ������������ Accelerator Pedal Kickdown Switch
92 ������� �������� �� ������� �������� Engine Percent Load At Current Speed
*/
#pragma pack(1)
typedef struct PGN_61443 { // Electronic Engine Controller 2 (50 ����)
    uint8_t  PdLowIdleSw1:	2; // 558 Accelerator Pedal 1 Low Idle Switch
    uint8_t  PdKickdownSw1: 2; // 559 Accelerator Pedal Kickdown Switch
    uint8_t  SpeedLimitStatus: 2; // 1437 Road Speed Limit Status
    uint8_t  PdLowIdleSw2: 2; // 2970 Accelerator Pedal 2 Low Idle Switch
    uint8_t  PdPos1; // 91 Accelerator Pedal Position 1
    uint8_t  PercLoadSpeed; // 92 Engine Percent Load At Current Speed
    uint8_t RemotePdPos; // 974 Remote Accelerator Pedal Position
    uint8_t  PdPos2; // 29 Accelerator Pedal Position 2
    uint8_t  AccLimitSt: 4; // 2979 Vehicle Acceleration Rate Limit Status
    uint8_t  MaxPowerEn: 4; // 5021 Momentary Engine Maximum Power Enable Feedback
    uint8_t  MaxPercTorque; // 3357 Actual Maximum Available Engine - Percent Torque
    uint8_t  res8;
} PGN_61443_t;
#pragma pack()

/*
899 ����� ���������� �������� �������� ��������� Engine Torque Mode 25-50
512 ��������� ������ � ������� ��������� ������� Driver's Demand Engine - Percent Torque
513 ����������� ������ � ������� ��������� �������, ������� ���. ������ Actual Engine - Percent Torque
190 ������� �������� ��������� Engine Speed
1483 ������� ����� ����������, ������������ ���������� Source Address of Controlling Device for Engine Control
1675 ����� ������� ��������� Engine Starter Mode
*/
#pragma pack(1)
typedef struct PGN_61444 { // Electronic Engine Controller 1 (25-50 ����)
    uint8_t  TorqueMode:	4; // 899 Engine Torque Mode
    uint8_t  PercentTorqueHi: 4; // 4154 Actual Engine - Percent Torque High Resolution
    uint8_t  DriverPercentTorque; // 512 Driver's Demand Engine - Percent Torque
    uint8_t  ActualPercentTorque; // 513 Actual Engine - Percent Torque
    uint16_t EngineSpeed; // 190 Engine Speed
    uint8_t  SourceAddress; // 1483 Source Address of Controlling Device for Engine Control
    uint8_t  StarterMode: 4; // 1675 Engine Starter Mode
    uint8_t  DemandPercentTorque; // 2432 Engine Demand � Percent Torque
} PGN_61444_t;
#pragma pack()

/*
156 �������� ������� � ����� Engine Injector Timing Rail 1 Pressure
*/
#pragma pack(1)
typedef struct PGN_65243 { // Engine Fluid Level/Pressure 2 (500 ����)
    uint16_t InjectionControl_P; // 164 Engine Injection Control Pressure
    uint16_t MeteringRail_1_P; // 157 Engine Injector Metering Rail 1 Pressure
    uint16_t TimingRail_1_P; // 156 Engine Injector Timing Rail 1 Pressure
    uint16_t MeteringRail_2_P; // 1349 Engine Injector Metering Rail 2 Pressure
} PGN_65243_t;
#pragma pack()

/*
514 ����������� ������ � ������� ��������� ������� Nominal Friction - Percent Torque
*/
#pragma pack(1)
typedef struct PGN_65247 { // Nominal Friction - Percent Torque (250 ����)
    uint8_t  NominalFriction; // 514 Nominal Friction - Percent Torque
    uint16_t DesiredOpSpeed; // 515 Engine's Desired Operating Speed
    uint8_t  DriverPercentTorque; // 512 Driver's Demand Engine - Percent Torque
    uint8_t  ActualPercentTorque; // 513 Actual Engine - Percent Torque
    uint16_t GasMassFlow1; // 3236 Aftertreatment 1 Exhaust Gas Mass Flow
    uint8_t  IntakeDewPoint1: 2; // 3237 Aftertreatment 1 Intake Dew Point
    uint8_t  ExhaustDewPoint1: 2; // 3238 Aftertreatment 1 Exhaust Dew Point
    uint8_t  IntakeDewPoint2: 2; // 3239 Aftertreatment 2 Intake Dew Point
    uint8_t  ExhausDewPoint2: 2; // 3240 Aftertreatment 2 Exhaust Dew Point
} PGN_65247_t;
#pragma pack()

/*
247 ����� ����� ������ Engine Total Hours of Operation
249 ����� ����� �������� ��������� Engine Total Revolutions
*/
#pragma pack(1)
typedef struct PGN_65253 { // Engine Hours, Revolutions (�� �������)
    uint32_t TotalHours; // 247 Engine Total Hours of Operation
    uint32_t TotalRevolutions; // 249 Engine Total Revolutions
} PGN_65253_t;
#pragma pack()

/*
110 ����������� ����������� �������� Engine Coolant Temperature
174 ����������� ������� Engine Fuel Temperature 1
175 ����������� ����� Engine Oil Temperature 1
*/
#pragma pack(1)
typedef struct PGN_65262 { // Engine Temperature 1 (1000 ����)
    uint8_t  Coolant_T; // 110 Engine Coolant Temperature
    uint8_t  Fuel_T; // 174 Engine Fuel Temperature 1
    uint16_t Oil_T; // 175 Engine Oil Temperature 1
    uint16_t TurbochargerOil_T; // 176 Engine Turbocharger Oil Temperature
    uint8_t  Intercooler_T; // 52 Engine Intercooler Temperature
    uint8_t  IntercoolerThermostatOpening; // 1134 Engine Intercooler Thermostat Opening
} PGN_65262_t;
#pragma pack()

/*
94 �������� ������� ����� ������� Engine Fuel Delivery Pressure
98 ������� ����� Engine Oil Level
100 �������� ����� Engine Oil Pressure
109 �������� ����������� �������� Engine Coolant Pressure
111 ������� ����������� �������� Engine Coolant Level
*/
#pragma pack(1)
typedef struct PGN_65263 { // Engine Fluid Level/Pressure 1 (500 ����)
    uint8_t  FuelDelivery_P; // 94 Engine Fuel Delivery Pressure
    uint8_t  CrankcaseBlow_P; // 22 Engine Extended Crankcase Blow-by Pressure
    uint8_t  Oil_L; // 98 Engine Oil Level
    uint8_t  Oil_P; // 100 Engine Oil Pressure
    uint16_t Crankcase_P; // 101 Engine Crankcase Pressure
    uint8_t  Coolant_P; // 109 Engine Coolant Pressure
    uint8_t  Coolant_L; // 111 Engine Coolant Level
} PGN_65263_t;
#pragma pack()

/*
183 ������ ������� Engine Fuel Rate
184 ���������� ������ ������� Engine Instantaneous Fuel Economy
51 ��������� ������ Engine Throttle Valve 1 Position
*/
#pragma pack(1)
typedef struct PGN_65266 { // Fuel Economy (Liquid) (100 ����)
    uint16_t FuelRate; // 183 Engine Fuel Rate
    uint16_t InstantaneousEconomy; // 184 Engine Instantaneous Fuel Economy
    uint16_t AverageEconomy; // 185 Engine Average Fuel Economy
    uint8_t  ThrottlePos; // 51 Engine Throttle Position
    uint8_t  ThrottlePos2; // 3673 Engine Throttle 2 Position
} PGN_65266_t;
#pragma pack()

/*
102 �������� ������� Engine Intake Manifold #1 Pressure
105 ����������� ������� �� ������ Engine Intake Manifold 1 Temperature
106 �������� ������� �� ������ Engine Air Intake Pressure
173 ����������� ��������� ����� Engine Exhaust Gas Temperature
*/
#pragma pack(1)
typedef struct PGN_65270 { // Inlet/Exhaust Conditions 1 (500 ����)
    uint8_t  FilterInlet_P; // 81 Engine Diesel Particulate Filter Inlet Pressure
    uint8_t  Manifold_1_P; // 102 Engine Intake Manifold #1 Pressure
    uint8_t  Manifold_1_T; // 105 Engine Intake Manifold 1 Temperature
    uint8_t  AirInlet_P; // 106 Engine Air Inlet Pressure
    uint8_t  AirFlt1Diff_P; // 107 Engine Air Filter 1 Differential Pressure
    uint16_t ExhaustGas_T; // 173 Engine Exhaust Gas Temperature
    uint8_t  CoolantFltDiff_P; // 112 Engine Coolant Filter Differential Pressure
} PGN_65270_t;
#pragma pack()

/*
96 ������� ������� Fuel_Level
*/
#pragma pack(1)
typedef struct PGN_65276 { // Dash Display (1000 ����)
    uint8_t  WasheFluid_L; // 80 Washer Fluid Level
    uint8_t  Fuel_L1; // 96 Fuel Level 1
    uint8_t  FuelFltDiff_P; // 95 Engine Fuel Filter Differential Pressure
    uint8_t  OilFltDiff_P; // 99 Engine Oil Filter Differential Pressure
    uint16_t CargoAmbient_T; // 169 Cargo Ambient Temperature
    uint8_t  Fuel_L2; // 38 Fuel Level 2
    uint8_t  res8;
} PGN_65276_t;
#pragma pack()

#define PGN_61443 61443 << 8
#define PGN_61444 61444 << 8
#define PGN_65243 65243 << 8
#define PGN_65247 65247 << 8
#define PGN_65253 65253 << 8
#define PGN_65262 65262 << 8
#define PGN_65263 65263 << 8
#define PGN_65266 65266 << 8
#define PGN_65270 65270 << 8
#define PGN_65276 65276 << 8

void canJ1939_init(void);
void J1939_step (void);

#endif // #ifndef J1939_H