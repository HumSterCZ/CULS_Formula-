#pragma once
//// ===================INCLUDES===================
#include "mcp_can.h"
#include "sw_pot_wheel_data.h"

//// ===================DEFINES===================
// -----------CAN_ADRESS------------
// SEND
#define CAN_ID_SWITCH_POSITION   0x100
#define CAN_ID_POT1              0x101
#define CAN_ID_POT2              0x102
#define CAN_ID_POT3              0x103
#define CAN_ID_SWITCH_INPUT1     0x110
#define CAN_ID_SWITCH_INPUT2     0x111
#define CAN_ID_SWITCH_INPUT3     0x112
#define CAN_ID_SWITCH_INPUT4     0x113
#define CAN_ID_SOFTWARE_SWITCH   0x120
#define CAN_ID_SHIFT_UP          0x130
#define CAN_ID_SHIFT_DOWN        0x131
#define CAN_ID_LEVE_TLAC         0x132
#define CAN_ID_PRAVE_TLAC        0x133
#define CAN_ID_KILL_SWITCH       0x140

// RECIEVE
#define CAN_ID_GEAR                0x200
#define CAN_ID_RPM                 0x201
#define CAN_ID_WATER_TEMP          0x202
#define CAN_ID_OIL_TEMP            0x203
#define CAN_ID_WATER_PRESSURE      0x204
#define CAN_ID_OIL_PRESSURE        0x205
#define CAN_ID_THROTTLE_POSITION   0x206
#define CAN_ID_FUEL_PRESSURE       0x207
#define CAN_ID_INTAKE_TEMP         0x208
#define CAN_ID_INTAKE_PRESSURE     0x209
#define CAN_ID_AFR                 0x210
#define CAN_ID_SPEED               0x211
#define CAN_ID_BATTERY_LOW         0x212
#define CAN_ID_BATTERY_HYBRID      0x213
#define CAN_ID_TIME                0x214
#define CAN_ID_DATE                0x215

// -----------DEFINES------------
// Modul pro posílání canu MCP2515 CAN nastavení
#define PRIORITY_CAN_CS 49    // Chip Select pro PriorityCAN
#define SECONDARY_CAN_CS 48   // Chip Select pro SecondaryCAN
#define PRIORITY_CAN_INT 47   // Interrupt pro PriorityCAN NENÍ IMPLEMENTOVÁNO
#define SECONDARY_CAN_INT 46  // Interrupt pro SecondaryCAN NENÍ IMPLEMENTOVÁNO
#define CAN_SPEED CAN_1000KBPS  // Rychlost CAN komunikace
#define CAN_CLOCK MCP_8MHZ      // Hodinová frekvence MCP2515

//// ===================VARIABLES===================
#ifndef CAN_BUS_H
#define CAN_BUS_H

struct CANData {
  int gear;
  int rpm;
  int waterTemp;
  int oilTemp;
  float waterPressure;
  float oilPressure;
  float throttlePosition;
  float fuelPressure;
  int intakeTemp;
  float intakePressure;
  float afr;
  float speed;
  float batteryLow;
  float batteryHybrid;
  unsigned long time;
  unsigned long date;
};

extern CANData canData; // Deklarace globální proměnné

#endif // CAN_BUS_H

//// ===================INICIALIZATION===================
void canBusInit();

//// ===================FUNKCTIONS==================
// Pošle všechna získaná data z přístrojové desky a volantu na CAN
void sendAllData();

//Pomocná funkce pro posílání zpráv na CAN 
void sendCANMessage(MCP_CAN &canBus, unsigned int id, int value);

//Funkce pro čtení dat z prioritního canu 
void readPriorityCAN();

// Funkce pro čtení zpráv z CAN sběrnice
void readCAN(MCP_CAN &canBus, const char *canName);

//Funkce pro čtení dat z prioritního canu 
void processCANMessage(long unsigned int id, unsigned char* data, unsigned char len);

// Funkce pro čtení CAN zpráv
void readCANMessages();

/* NENÍ IMPLEMENTOVÁNO
// Přerušovací rutiny PRIO 
void handlePriorityCANInterrupt();

// Přerušovací rutiny SEC
void handleSecondaryCANInterrupt();
*/