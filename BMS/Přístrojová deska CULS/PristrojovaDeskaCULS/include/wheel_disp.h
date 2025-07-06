#pragma once
//// ===================INCLUDES===================
#include "can_bus.h"
#include <SoftwareSerial.h>

//// ===================DEFINES===================
// Definování UART komunikace
#define RX_PIN 0 // Arduino přijímací pin
#define TX_PIN 1 // Arduino vysílací pin

// Definice komponent na Nextion displeji
#define NEXTION_GEAR           "gear.val"      // Komponenta pro převodový stupeň
#define NEXTION_RPM            "rpm.val"       // Komponenta pro otáčky
#define NEXTION_WATER_TEMP     "waterTemp.val" // Komponenta pro teplotu vody
#define NEXTION_OIL_TEMP       "oilTemp.val"   // Komponenta pro teplotu oleje
#define NEXTION_WATER_PRESSURE "waterPress.val" // Komponenta pro tlak vody
#define NEXTION_OIL_PRESSURE   "oilPress.val"  // Komponenta pro tlak oleje
#define NEXTION_THROTTLE_POS   "throttle.val"  // Komponenta pro pozici plynu
#define NEXTION_FUEL_PRESSURE  "fuelPress.val" // Komponenta pro tlak paliva
#define NEXTION_INTAKE_TEMP    "intakeTemp.val" // Komponenta pro teplotu sání
#define NEXTION_INTAKE_PRESS   "intakePress.val" // Komponenta pro tlak sání
#define NEXTION_AFR            "afr.val"       // Komponenta pro AFR
#define NEXTION_SPEED          "speed.val"     // Komponenta pro rychlost
#define NEXTION_BATTERY_LOW    "batLow.val"    // Komponenta pro nízkou baterii
#define NEXTION_BATTERY_HYBRID "batHybrid.val" // Komponenta pro hybridní baterii
#define NEXTION_TIME           "time.txt"      // Komponenta pro čas
#define NEXTION_DATE           "date.txt"      // Komponenta pro datum

//// ===================FUNKCTIONS===================
//Funkce která přiřazuje dannou promněnou k danné adrese na display
void updateNextionDisplay();

// Funkce pro odeslání příkazu na displej
void sendCommand(const char* cmd);
