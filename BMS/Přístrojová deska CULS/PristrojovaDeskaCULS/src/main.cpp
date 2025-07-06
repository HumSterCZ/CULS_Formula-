//// ================INCLUDES================
#include "led_stripe.h"
#include "can_bus.h"
#include "sw_pot_wheel_data.h"
#include "wheel_disp.h"

//// ===================DEFINES===================
bool testing_mode = true;
#define SERIAL_MONITOR_BAUD_RATE 9600
#define POWER_UP_SAFETY_DELAY 3000

//// ===================VARIABLES===================
int rpm;
int gear;
int waterTemp;
int oilTemp;

//// ================SETUP================
void setup() {
  delay(POWER_UP_SAFETY_DELAY);  // Power-up safety delay

  Serial.begin(SERIAL_MONITOR_BAUD_RATE);  // Inicializace Serial Monitoru
  
  ledStripeInit();  // Inicializace led pásku

  canBusInit(); // Inicializace CAN bus

  swPotModeInit(); // Inicializace tlačítek, potenciometrů, 10 polohového přepínače, killswitche, startovacího tlačítka, pádla pod volantem a volantových talčítek 
}

//// ================LOOP================
void loop() {
  if (testing_mode) simulateValues();  // Testovací funkce pro Led pásek
  else setLEDs(rpm, gear, waterTemp, oilTemp); // Funkce pro zobrazování hodnot z auta na LED Pásek

  readPotentiometerData();  // Čtení potenciometrů

  readTenStateSwitch(); // Získání aktuálního stavu 10polohového přepínače

  readStartSwitch(); // Funkce pro zpracování startovacího tlačítka
  
  processFourBoardSwitches(); // Načtení dat ze 4 tlačítek z přístrojové desky

  processKillSwitch(); // Funkce pro  čtení kill switche

  wheelSwitchesRead(); // Čtení stavů dvou tlačítek z volantu a řadících pádel

  serialPrintSwPotWheel(); // Tisk pomocných proměnných do seriového monitoru

  sendAllData(); // Funkce pošle na can všechna data získaná z přístrojové desky

  readCANMessages(); // Načte všechny data z canu a uloží do promněnýh
  
  updateNextionDisplay();
}
