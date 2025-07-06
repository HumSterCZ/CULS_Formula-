//// ===================INCLUDES===================
#include "can_bus.h"

// ===================VARIABLES===================
MCP_CAN priorityCAN(PRIORITY_CAN_CS);   // Objekt pro PriorityCAN
MCP_CAN secondaryCAN(SECONDARY_CAN_CS); // Objekt pro SecondaryCAN

CANData canData; // Definice globální proměnné

// Stavové proměnné pro přerušení
//volatile bool priorityCANInterrupt = false; //NENÍ IMPLEMENTOVÁNO
//volatile bool secondaryCANInterrupt = false; //NENÍ IMPLEMENTOVÁNO

//// ===================INICIALIZATION===================
void canBusInit(){
    if (priorityCAN.begin(MCP_ANY, CAN_SPEED, CAN_CLOCK) == CAN_OK) {
    Serial.println("PriorityCAN initialized successfully");
    priorityCAN.setMode(MCP_NORMAL);
  } else {
    Serial.println("Error initializing PriorityCAN");
    while (1);
  }

  if (secondaryCAN.begin(MCP_ANY, CAN_SPEED, CAN_CLOCK) == CAN_OK) {
    Serial.println("SecondaryCAN initialized successfully");
    secondaryCAN.setMode(MCP_NORMAL);
  } else {
    Serial.println("Error initializing SecondaryCAN");
    while (1);
  }

  // Nastavení přerušení
  pinMode(PRIORITY_CAN_INT, INPUT);
  pinMode(SECONDARY_CAN_INT, INPUT);
}

//// ===================FUNKCTIONS===================
// Pošle všechna získaná data z přístrojové desky a volantu na CAN
void sendAllData(){
  sendCANMessage(secondaryCAN, CAN_ID_POT1, analogRead(POTENTIOMETER_1_PIN));
  sendCANMessage(secondaryCAN, CAN_ID_POT2, analogRead(POTENTIOMETER_2_PIN));
  sendCANMessage(secondaryCAN, CAN_ID_POT3, analogRead(POTENTIOMETER_3_PIN));
  sendCANMessage(secondaryCAN, CAN_ID_SWITCH_POSITION, readSwitchPosition());
  sendCANMessage(secondaryCAN, CAN_ID_SOFTWARE_SWITCH, sendDataStarButton());
  sendCANMessage(secondaryCAN, CAN_ID_SHIFT_UP, digitalRead(SHIFT_UP_PIN));
  sendCANMessage(secondaryCAN, CAN_ID_SHIFT_DOWN, digitalRead(SHIFT_DOWN_PIN));
  sendCANMessage(secondaryCAN, CAN_ID_LEVE_TLAC, digitalRead(LEFT_WHEEL_BUTTON_PIN));
  sendCANMessage(secondaryCAN, CAN_ID_PRAVE_TLAC, digitalRead(RIGHT_WHEEL_BUTTON_PIN));
  sendCANMessage(secondaryCAN, CAN_ID_KILL_SWITCH, digitalRead(KILL_SWITCH_BUTTON_PIN));
  for (int i = 0; i < 4; i++) {
    int switchState = digitalRead(switchInputs[i]);
    sendCANMessage(secondaryCAN, CAN_ID_SWITCH_INPUT1 + i, switchState);
  }
}

//Pomocná funkce pro posílání zpráv na CAN 
void sendCANMessage(MCP_CAN &canBus, unsigned int id, int value) {
  byte buf[1] = { (byte)value };
  canBus.sendMsgBuf(id, 0, 1, buf);
}

//Funkce pro čtení dat z prioritního canu 
void processCANMessage(long unsigned int id, unsigned char* data, unsigned char len) {
  switch (id) {
    case CAN_ID_GEAR:
      canData.gear = data[0]; // Předpoklad: 1 bajt pro převodový stupeň
      break;
    case CAN_ID_RPM:
      canData.rpm = (data[0] << 8) | data[1]; // Předpoklad: 2 bajty pro otáčky
      break;
    case CAN_ID_WATER_TEMP:
      canData.waterTemp = data[0]; // Předpoklad: 1 bajt pro teplotu
      break;
    case CAN_ID_OIL_TEMP:
      canData.oilTemp = data[0]; // Předpoklad: 1 bajt pro teplotu
      break;
    case CAN_ID_WATER_PRESSURE:
      canData.waterPressure = ((data[0] << 8) | data[1]) / 100.0; // Předpoklad: 2 bajty, děleno 100
      break;
    case CAN_ID_OIL_PRESSURE:
      canData.oilPressure = ((data[0] << 8) | data[1]) / 100.0;
      break;
    case CAN_ID_THROTTLE_POSITION:
      canData.throttlePosition = ((data[0] << 8) | data[1]) / 100.0;
      break;
    case CAN_ID_FUEL_PRESSURE:
      canData.fuelPressure = ((data[0] << 8) | data[1]) / 100.0;
      break;
    case CAN_ID_INTAKE_TEMP:
      canData.intakeTemp = data[0];
      break;
    case CAN_ID_INTAKE_PRESSURE:
      canData.intakePressure = ((data[0] << 8) | data[1]) / 100.0;
      break;
    case CAN_ID_AFR:
      canData.afr = ((data[0] << 8) | data[1]) / 100.0;
      break;
    case CAN_ID_SPEED:
      canData.speed = ((data[0] << 8) | data[1]) / 100.0;
      break;
    case CAN_ID_BATTERY_LOW:
      canData.batteryLow = ((data[0] << 8) | data[1]) / 100.0;
      break;
    case CAN_ID_BATTERY_HYBRID:
      canData.batteryHybrid = ((data[0] << 8) | data[1]) / 100.0;
      break;
    case CAN_ID_TIME:
      canData.time = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]; // 4 bajty pro čas
      break;
    case CAN_ID_DATE:
      canData.date = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]; // 4 bajty pro datum
      break;
    default:
      // Nepodporované ID
      break;
  }
}

// Funkce pro čtení CAN zpráv
void readCANMessages() {
  long unsigned int rxId;         // ID přijaté zprávy
  unsigned char len = 0;          // Délka datové části
  unsigned char rxBuf[8];         // Buffer pro data

  // Kontrola dostupnosti zprávy na PriorityCAN
  if (priorityCAN.checkReceive() == CAN_MSGAVAIL) {
    if (priorityCAN.readMsgBuf(&rxId, &len, rxBuf) == CAN_OK) {
      processCANMessage(rxId, rxBuf, len); // Zpracování zprávy
    }
  }
}

/* NENÍ IMPLEMENTOVÁNO
// Přerušovací rutiny PRIO 
void handlePriorityCANInterrupt() {
  priorityCANInterrupt = true;
}

// Přerušovací rutiny SEC
void handleSecondaryCANInterrupt() {
  secondaryCANInterrupt = true;
}
*/