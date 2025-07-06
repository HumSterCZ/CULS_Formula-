//// ===================INCLUDES===================
#include "wheel_disp.h"

//// ===================VARIABLES===================
SoftwareSerial nextion(RX_PIN, TX_PIN);

//// ===================FUNKCTIONS===================
//Funkce která přiřazuje dannou promněnou k danné adrese na display
void updateNextionDisplay() {
  char buffer[32];

  sprintf(buffer, "%s=%.2f", NEXTION_WATER_PRESSURE, (double)canData.waterPressure);
  sendCommand(buffer);

  sprintf(buffer, "%s=%.2f", NEXTION_OIL_PRESSURE, (double)canData.oilPressure);
  sendCommand(buffer);

  sprintf(buffer, "%s=%.2f", NEXTION_THROTTLE_POS, (double)canData.throttlePosition);
  sendCommand(buffer);

  sprintf(buffer, "%s=%.2f", NEXTION_FUEL_PRESSURE, (double)canData.fuelPressure);
  sendCommand(buffer);

  sprintf(buffer, "%s=%.2f", NEXTION_INTAKE_PRESS, (double)canData.intakePressure);
  sendCommand(buffer);

  sprintf(buffer, "%s=%.2f", NEXTION_AFR, (double)canData.afr);
  sendCommand(buffer);

  sprintf(buffer, "%s=%.2f", NEXTION_SPEED, (double)canData.speed);
  sendCommand(buffer);

  sprintf(buffer, "%s=%.2f", NEXTION_BATTERY_LOW, (double)canData.batteryLow);
  sendCommand(buffer);

  sprintf(buffer, "%s=%.2f", NEXTION_BATTERY_HYBRID, (double)canData.batteryHybrid);
  sendCommand(buffer);
}

// Funkce pro odeslání příkazu na displej
void sendCommand(const char* cmd) {
  nextion.print(cmd);        // Odeslat příkaz
  nextion.write(0xFF);       // Ukončovací bajty
  nextion.write(0xFF);
  nextion.write(0xFF);
}
