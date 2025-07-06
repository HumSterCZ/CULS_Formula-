#pragma once
//// ===================INCLUDES===================
#include <SPI.h>

//// ===================DEFINES===================
// Piny pro přepínač 10 poloh
const int switchPins[] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31}; // Piny desetipolohového switche 
const int numSwitchPins = sizeof(switchPins) / sizeof(switchPins[0]);

// Piny pro 4 přepínače a jejich LED
const int switchInputs[] = {32, 34, 36, 38}; // Piny 4 tlačítek na přístrojovce
const int ledOutputs[] = {33, 35, 37, 39};  // Piny 4 LED  podsvícení tlačítek na přístrojovce
const int numSingleSwitches = sizeof(switchInputs) / sizeof(switchInputs[0]);

// Piny pro startovací tlačítko
#define START_BUTTON_PIN        8   // Pin startovacího tlačítka
#define START_BUTTON_LED_PIN    7   // Pin LED startovacího tlačítka

// Kill Switch piny
#define KILL_SWITCH_BUTTON_PIN  42    // Pin tlačítka Kill Switch
#define KILL_SWITCH_LED_1_PIN   41    // První LED Kill Switch
#define KILL_SWITCH_LED_2_PIN   40    // Druhá LED Kill Switch

// Tlačítka na volantu a řadící pádla
#define SHIFT_UP_PIN            3   // Pin zařadit nahoru talčítko
#define SHIFT_DOWN_PIN          4   // Pin zařadit dolu talčítko
#define LEFT_WHEEL_BUTTON_PIN   5   // Pin levé tlačítko volant
#define RIGHT_WHEEL_BUTTON_PIN  6   // Pin pravé tlačítko volant

//Definice Potenciomerů
#define POTENTIOMETER_1_PIN A0
#define POTENTIOMETER_2_PIN A1
#define POTENTIOMETER_3_PIN A2


//// ===================INICIALIZATION===================
// Nastaví kompletní pinmódy
void swPotModeInit();

//// ===================FUNKCTIONS===================
// Funkce pro čtení analogových potenciometrů
void readPotentiometerData();

// Načtení dat ze 4 tlačítek z přístrojové desky
void processFourBoardSwitches();

// Funkce pro  čtení kill switche
void processKillSwitch();

// Čtení stavů dalších tlačítek z volantu a pádel
void wheelSwitchesRead();

// Čtení stavu startovacího tlačítka
void  readStartSwitch();
int sendDataStarButton(); // Pošle int data o startovacím tlačítku

// Načtení stavu z desetipolohového switche
void readTenStateSwitch();
int readSwitchPosition(); //pomocná funkce pro čtení stavu

// Výpis debug proměnných do Serial Monitoru
void serialPrintSwPotWheel();
