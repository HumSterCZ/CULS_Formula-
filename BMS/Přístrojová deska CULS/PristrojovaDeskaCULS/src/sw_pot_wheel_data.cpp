//// ===================INCLUDES===================
#include "sw_pot_wheel_data.h"


//// ===================VARIABLES===================
//Startovací tlačítko
bool softLedState = LOW;          // Stav softwarové LED
bool lastButtonState = LOW;       // Předchozí stav tlačítka
bool currentButtonState = LOW;    // Aktuální stav tlačítka
unsigned long lastDebounceTime = 0; // Čas poslední změny stavu tlačítka
const unsigned long debounceDelay = 50; // Zpoždění pro debounce

//Proměné potenciometrů
int potValue1 = 0;
int potValue2 = 0;
int potValue3 = 0;

//Stavy tlačítek na volantu a řadících pádel
int shiftUpState = 0;
int shiftDownState = 0;
int leveTlacState = 0;
int praveTlacState = 0;

int selectedPosition = -1;
String stateText = "-1";
String switchStates = "";
int arrSwitchs[numSingleSwitches];

//// ===================INICIALIZATION===================
void swPotModeInit(){
  // Nastavení pinů přepínače jako vstup
  for (int i = 0; i < numSwitchPins; i++) pinMode(switchPins[i], INPUT);
  
  // Nastavení pinů pro 4 přepínače na přístrojové desce a jejich LED
  for (int i = 0; i < numSingleSwitches; i++) {
    pinMode(switchInputs[i], INPUT);
    pinMode(ledOutputs[i], OUTPUT);
    digitalWrite(ledOutputs[i], LOW);
  }

  // Nastavení pro startovací tlačítko
  pinMode(START_BUTTON_PIN, INPUT);
  pinMode(START_BUTTON_LED_PIN, OUTPUT);
  digitalWrite(START_BUTTON_LED_PIN, softLedState);

  // Nastavení pro kill switch
  pinMode(KILL_SWITCH_BUTTON_PIN, INPUT);
  pinMode(KILL_SWITCH_LED_1_PIN, OUTPUT);
  pinMode(KILL_SWITCH_LED_2_PIN, OUTPUT);

  // Nastavení pro tlačítka na volantu a řadící pádla
  pinMode(SHIFT_UP_PIN, INPUT);
  pinMode(SHIFT_DOWN_PIN, INPUT);
  pinMode(LEFT_WHEEL_BUTTON_PIN, INPUT);
  pinMode(RIGHT_WHEEL_BUTTON_PIN, INPUT);
}

//// ===================FUNKCTIONS===================
// Funkce pro čtení analogových potenciometrů
void readPotentiometerData(){
  potValue1 = analogRead(POTENTIOMETER_1_PIN);
  potValue2 = analogRead(POTENTIOMETER_2_PIN);
  potValue3 = analogRead(POTENTIOMETER_3_PIN);
}

// Načtení dat ze 4 tlačítek z přístrojové desky
void processFourBoardSwitches() {
  String result = "";
  for (int i = 0; i < numSingleSwitches; i++) {
    if (digitalRead(switchInputs[i]) == HIGH) {
      digitalWrite(ledOutputs[i], HIGH);
      result += "Vyp " + String(i + 1) + ": Zap | ";
      arrSwitchs[i] = 1;
    } else {
      digitalWrite(ledOutputs[i], LOW);
      result += "Vyp " + String(i + 1) + ": Vyp | ";
      arrSwitchs[i] = 0;
    }
  }
  switchStates = result;
}

// Funkce pro  čtení kill switche
void processKillSwitch() {
  int buttonState = digitalRead(KILL_SWITCH_BUTTON_PIN);
  if (buttonState == HIGH) {
    digitalWrite(KILL_SWITCH_LED_1_PIN, HIGH);
    digitalWrite(KILL_SWITCH_LED_2_PIN, LOW);
  } else {
    digitalWrite(KILL_SWITCH_LED_1_PIN, LOW);
    digitalWrite(KILL_SWITCH_LED_2_PIN, HIGH);
  }
}

// Čtení stavů dalších tlačítek z volantu a pádel
void wheelSwitchesRead(){
  shiftUpState = digitalRead(SHIFT_UP_PIN);
  shiftDownState = digitalRead(SHIFT_DOWN_PIN);
  leveTlacState = digitalRead(LEFT_WHEEL_BUTTON_PIN);
  praveTlacState = digitalRead(RIGHT_WHEEL_BUTTON_PIN);
}

// Funkce pro zpracování startovacího tlačítka
void readStartSwitch() {
  bool reading = digitalRead(START_BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      if (currentButtonState == HIGH) {
        softLedState = !softLedState;
        digitalWrite(START_BUTTON_LED_PIN, softLedState);
      }
    }
  }
  lastButtonState = reading;
}

// Pošle int data o startovacím tlačítku
int sendDataStarButton(){
  if (softLedState) return 1;
  else return 0;
}

// Načtení stavu z desetipolohového switche
void readTenStateSwitch(){
  selectedPosition = readSwitchPosition();
  stateText = (selectedPosition != -1) ? String(selectedPosition + 1) : "Neurčený";
}

// Funkce pro čtení polohy 10polohového přepínače
int readSwitchPosition() {
  for (int i = 0; i < numSwitchPins; i++) {
    if (digitalRead(switchPins[i]) == HIGH) {
      return i;
    }
  }
  return -1;
}

// Výpis do Serial Monitoru
void serialPrintSwPotWheel(){
  Serial.print("Pot 1: ");
  Serial.print(potValue1);
  Serial.print(" | Pot 2: ");
  Serial.print(potValue2);
  Serial.print(" | Pot 3: ");
  Serial.print(potValue3);
  Serial.print(" | Stav: ");
  Serial.print(stateText);
  Serial.print(" | ");
  Serial.print(switchStates);
  Serial.print("StartTlac: ");
  Serial.print(softLedState ? "Zap" : "Vyp");
  Serial.print(" | KillSw: ");
  Serial.print(digitalRead(KILL_SWITCH_BUTTON_PIN) == HIGH ? "Zap" : "Vyp");
  Serial.print(" | ShiftUP: ");
  Serial.print(shiftUpState);
  Serial.print(" | ShiftDown: ");
  Serial.print(shiftDownState);
  Serial.print(" | LeveTlac: ");
  Serial.print(leveTlacState);
  Serial.print(" | PraveTlac: ");
  Serial.println(praveTlacState);
}
