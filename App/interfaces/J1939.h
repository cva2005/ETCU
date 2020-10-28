#ifndef J1939_H
#define J1939_H

#include <stdbool.h>
#include "types.h"

typedef void (*J1939rxFun_t) (char *data, uint8_t len, uint32_t adr);
extern J1939rxFun_t J1939rxFun;

/*
 * After power on, a node should internally set the “availability bits”
 * of received parameters as not available and operate with default
 * values until valid data is received.
 * When transmitting, undefined bytes should be sent as
 * 255 (FF16) and undefined bits should be sent as 1.
 */
#pragma pack(1)
typedef struct J1939_ID {
	unsigned SA:	 		8;
	union {
		struct {
		unsigned PS:		8;
		unsigned PF:		8;
		unsigned DP:		1;
		unsigned EDP:		1;
		} FIELD;
		unsigned FULL:		18;
	} PGN;
} J1939_ID_t;
#pragma pack()

/*
695 Режим управления двигателем
696 Запрошенные условия регулирования скорости
897 Приоритет режима управления
898 Запрошенная скорость вращения (Error indicator FExx, Not available FFxx)
518 Запрошенный крутящий момент
 */
#pragma pack(1)//  (To engine: Control Purpose dependent or 10 ms To retarder: 50 ms)
typedef struct PGN_00000 { // TSC1 Torque Speed Control 1
    uint8_t  ControlMode:		2;	// 695 Engine Override Control Mode
    uint8_t  ControlConditions:	2;	// 696 Engine Requested Speed Control Conditions
    uint8_t  ModePriority: 		2;	// 897 Override Control Mode Priority
    uint8_t	 Unused_1: 			2;
    uint16_t RequestedSpeed; 		// 898 Engine Requested Speed/Speed Limit
    uint8_t  RequestedTorque; 		// 518 Engine Requested Torque/Torque Limit
    uint8_t  TransmissionRate: 	3; 	// 3349 TSC1 Transmission Rate
    uint8_t  ControlPurpose: 	5;	// 3350 TSC1 Control Purpose
    uint8_t  TorqueHighResol: 	4;	// 4191 Engine Requested Torque -  High Resolution
    uint8_t  Unused_2: 			4;
    uint8_t  Unused_3;
    uint8_t  MessageCounter: 	4; 	// 4206 Message Counter
    uint8_t  MessageChecksum: 	4; 	// 4207 Message Checksum
} PGN_00000_t;
#pragma pack()

//#define TSC1_CHECKSUM
#define TSC1_TX_RATE		50
#if		TSC1_TX_RATE == 1000
	#define VAL_TX_RATE		0
#elif	TSC1_TX_RATE == 750
	#define VAL_TX_RATE		1
#elif	TSC1_TX_RATE == 500
	#define VAL_TX_RATE		2
#elif	TSC1_TX_RATE == 250
	#define VAL_TX_RATE		3
#elif	TSC1_TX_RATE == 100
	#define VAL_TX_RATE		4
#elif	TSC1_TX_RATE == 50
	#define VAL_TX_RATE		5
#elif	TSC1_TX_RATE == 17	// 20 ms
	#define VAL_TX_RATE		6
#elif	TSC1_TX_RATE == 8	// 10 ms
	#define VAL_TX_RATE		7
#else
	#error Rate Not Defined!
#endif

#define PURP_OPER			0
#define PURP_CRUISE			1
#define PURP_PTO			2
#define MESS_CNT_MAX		7
#define NO_CHECK			0x0f

/*
	Engine Override Control Mode
Override Disabled 			00
Speed Control 				01
Torque Control 				10
Speed/Torque Limit Control	11
*/
typedef enum {
	OverrideDis		= 0,
	SpeedControl	= 1,
	TorqueControl	= 2,
	LimitControl	= 3
} ControlMode_t;

/*
	Engine Requested Speed Control Conditions
Transient Optimized for driveline disengaged and
non-lockup condition 		00
Stability Optimized for driveline disengaged and
non-lockup condition 1		01
Stability Optimized for driveline engaged and/or
in lockup condition 2		10
Stability Optimized for driveline engaged and/or
in lockup condition			11
*/
typedef enum {
	DisDrivelineNonLockup	= 0,
	DisDrivelineNonLockup1	= 1,
	EnDrivelineInLockup		= 2,
	EnDrivelineInLockup1	= 3
} ControlCond_t;

/*
	Override Control Mode Priority
Highest Priority			00
High Priority				01
Medium Priority				10
Low Priority				11
*/
typedef enum {
	HighestPriority	= 0,
	HighPriority	= 1,
	MediumPriority	= 2,
	LowPriority	= 3
} ModePriority_t;

/*
	Engine Requested Torque/Torque Limit (1% -125 to +125 %)
Error indicator 			FE
Not available 				FF
*/
typedef enum {
	ErrorIndicator	= 0xFE,
	NotAvailable	= 0xFF
} TorqueLimit_t;

/*
558 Датчик положения холостого хода педали акселератора Accelerator Pedal 1 Low Idle Switch 50
91 Положение педали акселератора Accelerator Pedal Position 1
559 Датчик кикдаун педали акселератора Accelerator Pedal Kickdown Switch
92 Процент нагрузки на текущих оборотах Engine Percent Load At Current Speed
*/
#pragma pack(1)
typedef struct PGN_61443 { // Electronic Engine Controller 2 (50 мсек)
    uint8_t  PdLowIdleSw1:	2; // 558 Accelerator Pedal 1 Low Idle Switch
    uint8_t  PdKickdownSw1: 2; // 559 Accelerator Pedal Kickdown Switch
    uint8_t  SpeedLimitStatus: 2; // 1437 Road Speed Limit Status
    uint8_t  PdLowIdleSw2: 2; // 2970 Accelerator Pedal 2 Low Idle Switch
    uint8_t  PdPos1; // 91 Accelerator Pedal Position 1
    uint8_t  PercLoadSpeed; // 92 Engine Percent Load At Current Speed
    uint8_t  RemotePdPos; // 974 Remote Accelerator Pedal Position
    uint8_t  PdPos2; // 29 Accelerator Pedal Position 2
    uint8_t  AccLimitSt: 4; // 2979 Vehicle Acceleration Rate Limit Status
    uint8_t  MaxPowerEn: 4; // 5021 Momentary Engine Maximum Power Enable Feedback
    uint8_t  MaxPercTorque; // 3357 Actual Maximum Available Engine - Percent Torque
    uint8_t  res8;
} PGN_61443_t;
#pragma pack()

/*
899 Режим управления крутящим моментом двигателя Engine Torque Mode 25-50
512 Требуемый момент – процент крутящего момента Driver's Demand Engine - Percent Torque
513 Действующий момент – процент крутящего момента, включая мех. потери Actual Engine - Percent Torque
190 Частота вращения двигателя Engine Speed
1483 Сетевой адрес устройства, управляющего двигателем Source Address of Controlling Device for Engine Control
1675 Режим запуска двигателя Engine Starter Mode
*/
#pragma pack(1)
typedef struct PGN_61444 { // Electronic Engine Controller 1 (25-50 мсек)
    uint8_t  TorqueMode:	4; // 899 Engine Torque Mode
    uint8_t  PercentTorqueHi: 4; // 4154 Actual Engine - Percent Torque High Resolution
    uint8_t  DriverPercentTorque; // 512 Driver's Demand Engine - Percent Torque
    uint8_t  ActualPercentTorque; // 513 Actual Engine - Percent Torque
    uint16_t EngineSpeed; // 190 Engine Speed
    uint8_t  SourceAddress; // 1483 Source Address of Controlling Device for Engine Control
    uint8_t  StarterMode: 4; // 1675 Engine Starter Mode
    uint8_t  DemandPercentTorque; // 2432 Engine Demand – Percent Torque
} PGN_61444_t;
#pragma pack()

/*
2659 Flow rate of gas through the EGR system. Flow rate of the exhaust
gas being recirculated into the combustion air.
0 to 3212.75 kg/h
0.05 kg/h per bit
132 Mass flow rate of fresh air entering the engine air intake, before any EGR mixer, if used.
Flow rate of fresh air conducted to the engine cylinders to support combustion.
0 to 3212.75 kg/h
0.05 kg/h per bit
*/
#pragma pack(1)
typedef struct PGN_61450 { // Engine Gas Flow Rate (50 мсек)
    uint16_t RecircFlow; // 2659 Engine Exhaust Gas Recirculation (EGR) Mass Flow Rate
    uint16_t AirMassFlow; // 132 Engine Inlet Air Mass Flow Rate
    uint32_t Unused;
} PGN_61450_t;
#pragma pack()

/*
156 Давление топлива в рейле Engine Injector Timing Rail 1 Pressure
*/
#pragma pack(1)
typedef struct PGN_65243 { // Engine Fluid Level/Pressure 2 (500 мсек)
    uint16_t InjectionControl_P; // 164 Engine Injection Control Pressure
    uint16_t MeteringRail_1_P; // 157 Engine Injector Metering Rail 1 Pressure
    uint16_t TimingRail_1_P; // 156 Engine Injector Timing Rail 1 Pressure
    uint16_t MeteringRail_2_P; // 1349 Engine Injector Metering Rail 2 Pressure
} PGN_65243_t;
#pragma pack()

/*
514 Номинальное трение – Процент крутящего момента Nominal Friction - Percent Torque
*/
#pragma pack(1)
typedef struct PGN_65247 { // Nominal Friction - Percent Torque (250 мсек)
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
247 Общее время работы Engine Total Hours of Operation
249 Общее число оборотов двигателя Engine Total Revolutions
*/
#pragma pack(1)
typedef struct PGN_65253 { // Engine Hours, Revolutions (по запросу)
    uint32_t TotalHours; // 247 Engine Total Hours of Operation
    uint32_t TotalRevolutions; // 249 Engine Total Revolutions
} PGN_65253_t;
#pragma pack()

/*
110 Температура охлаждающей жидкости Engine Coolant Temperature
174 Температура топлива Engine Fuel Temperature 1
175 Температура масла Engine Oil Temperature 1
*/
#pragma pack(1)
typedef struct PGN_65262 { // Engine Temperature 1 (1000 мсек)
    uint8_t  Coolant_T; // 110 Engine Coolant Temperature
    uint8_t  Fuel_T; // 174 Engine Fuel Temperature 1
    uint16_t Oil_T; // 175 Engine Oil Temperature 1
    uint16_t TurbochargerOil_T; // 176 Engine Turbocharger Oil Temperature
    uint8_t  Intercooler_T; // 52 Engine Intercooler Temperature
    uint8_t  IntercoolerThermostatOpening; // 1134 Engine Intercooler Thermostat Opening
} PGN_65262_t;
#pragma pack()

/*
94 Давление топлива после фильтра Engine Fuel Delivery Pressure
98 Уровень масла Engine Oil Level
100 Давление масла Engine Oil Pressure
109 Давление охлаждающей жидкости Engine Coolant Pressure
111 Уровень охлаждающей жидкости Engine Coolant Level
*/
#pragma pack(1)
typedef struct PGN_65263 { // Engine Fluid Level/Pressure 1 (500 мсек)
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
183 Расход топлива Engine Fuel Rate
184 Мгновенный расход топлива Engine Instantaneous Fuel Economy
51 Положение педали Engine Throttle Valve 1 Position
*/
#pragma pack(1)
typedef struct PGN_65266 { // Fuel Economy (Liquid) (100 мсек)
    uint16_t FuelRate; // 183 Engine Fuel Rate
    uint16_t InstantaneousEconomy; // 184 Engine Instantaneous Fuel Economy
    uint16_t AverageEconomy; // 185 Engine Average Fuel Economy
    uint8_t  ThrottlePos; // 51 Engine Throttle Position
    uint8_t  ThrottlePos2; // 3673 Engine Throttle 2 Position
} PGN_65266_t;
#pragma pack()

/*
102 Давление наддува Engine Intake Manifold #1 Pressure
105 Температура воздуха на впуске Engine Intake Manifold 1 Temperature
106 Давление воздуха на впуске Engine Air Intake Pressure
173 Температура выхлопных газов Engine Exhaust Gas Temperature
*/
#pragma pack(1)
typedef struct PGN_65270 { // Inlet/Exhaust Conditions 1 (500 мсек)
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
96 Уровень топлива Fuel_Level
*/
#pragma pack(1)
typedef struct PGN_65276 { // Dash Display (1000 мсек)
    uint8_t  WasheFluid_L; // 80 Washer Fluid Level
    uint8_t  Fuel_L1; // 96 Fuel Level 1
    uint8_t  FuelFltDiff_P; // 95 Engine Fuel Filter Differential Pressure
    uint8_t  OilFltDiff_P; // 99 Engine Oil Filter Differential Pressure
    uint16_t CargoAmbient_T; // 169 Cargo Ambient Temperature
    uint8_t  Fuel_L2; // 38 Fuel Level 2
    uint8_t  res8;
} PGN_65276_t;
#pragma pack()

#define J1939_ERR_TIME		500 // , мс
#define PGN_60928 60928 << 8
#define PGN_61443 61443 << 8
#define PGN_61444 61444 << 8
#define PGN_61450 61450 << 8
#define PGN_65243 65243 << 8
#define PGN_65247 65247 << 8
#define PGN_65253 65253 << 8
#define PGN_65262 65262 << 8
#define PGN_65263 65263 << 8
#define PGN_65266 65266 << 8
#define PGN_65270 65270 << 8
#define PGN_65276 65276 << 8
#define TSC1			00000
#define ENG_HOURS		65253
#define ADDR_CL_REQ		59904
#define ADDR_CL_RES		60928
#define ENG_GAS			61450

// Source Address ID
#define Engine1				0
#define Engine2				1
#define Turbocharger		2
#define Transmission1		3
#define Transmission2		4
#define ShiftConsole1		5
#define ShiftConsole2		6
#define PowerTakeOff		7
#define AxleSteering		8
#define AxleDrive1			9
#define AxleDrive2			10
#define SRC_ADDR			0x0b

#define E_HOURS_TIME		1000 // EngineHours request interval, ms
#define E_AIR_TIME			100 // Engine Gas Flow Rate request interval, ms
#define ADDR_GLOBAL			0xff
#define ADDR_NOT_CLAIMED 	0xfe
#define ADDR_NULL			0xef
#define ADDR_HOURS			0x21

#define PRIORITY_HIGH 		0
#define PRIORITY_TSC1 		3
#define PRIORITY_DEFAULT	6
#define PRIORITY_LOW		7

void canJ1939_init(void);
void J1939_step (void);
uint8_t TorqueSpeedControl (int8_t trq, uint16_t spd);
bool J1939_error (void);
uint8_t HoursRequest (void);

#endif // #ifndef J1939_H
