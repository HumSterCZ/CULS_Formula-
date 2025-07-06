#include <Arduino.h>
#include "bq79654.h"

/* --------------------------------------------------------------------------
   Definice hardwarových pinů:
   - BQ_RX_PIN a BQ_TX_PIN jsou určeny pro Serial1 (Leonardo: piny 0 a 1)
   - BQ_RELAY_PIN (pin 5) ovládá relé nebo MOSFET pro odpojení baterie
   - Pokud je připojen signál NFAULT, lze jej přiřadit dalšímu volnému pinu (např. pin 7)
-------------------------------------------------------------------------- */
#define BQ_RELAY_PIN 5

// Vytvoření instance BQ79654, využívající Serial1 (Leonardo: piny 0,1)
BQ79654 bq(Serial1);

/*
 * Definice OV/UV limitů:
 * Například: 4.25 V pro OV a 2.80 V pro UV
 */
bqUVOV_t uvov = {
  .ovLimit = 4.25f,
  .uvLimit = 2.80f
};

// Globální pole pro měření článků a teplot (NTC)
float cellVolt[14];
float cellTemp[8];

// Proměnná pro sledování, zda je balancování aktivní
static bool balancing = false;

void setup()
{
  // Inicializace USB sériového monitoru
  Serial.begin(115200);
  // Počkej, dokud se neaktivuje USB sériový port (typické pro nativní USB, např. Leonardo)
  while (!Serial) {
    ; // čekání, dokud není Serial připojeno
  }
  delay(200);

  // Nastavení pinu pro relé (odpojení baterie)
  pinMode(BQ_RELAY_PIN, OUTPUT);
  digitalWrite(BQ_RELAY_PIN, HIGH);  // Výchozí stav: baterie připojena

  // Inicializace BQ79654 s baudrate 1 Mb/s
  bq.begin(1000000);

  // Probuzení BQ z režimu SHUTDOWN pomocí WAKE pingu
  bq.wakeDevice();

  // Nastavení zařízení jako single-device s 14 články
  bq.initSingleDevice();

  // Nastavení OV/UV prahů
  bq.setupProtection(uvov);

  // Zapnutí open-wire (placeholder)
  bq.enableOpenWire();

  // Nastavení ADC v continuous módu
  bq.configMainADC14sContinuous();

  Serial.println("=== BQ79654 init done ===");
}

void loop()
{
  // Kontrola, zda jsou data připravena (DRDY_MAIN_ADC)
  if (!bq.isMainADCReady())
  {
    delay(200);
    return;
  }

  // Čtení napětí článků a teplot (NTC)
  bq.readCellVoltages(cellVolt, 14);
  bq.readTemperatures(cellTemp, 8);

  // Hledání maximální a minimální hodnoty a jejich indexů
  float maxV = 0.0f;
  int iMax = 0;
  float minV = 1000.0f;
  int iMin = 0;
  for (int i = 0; i < 14; i++)
  {
    if (cellVolt[i] > maxV)
    {
      maxV = cellVolt[i];
      iMax = i;
    }
    if (cellVolt[i] < minV)
    {
      minV = cellVolt[i];
      iMin = i;
    }
  }
  float delta = maxV - minV;

  // Kontrola teplot: všechny teploty musí být pod 60°C
  bool safeTemp = true;
  for (int t = 0; t < 8; t++)
  {
    if (cellTemp[t] >= 60.0f)
    {
      safeTemp = false;
      break;
    }
  }

  // Logika balancování:
  // Pokud je delta > 0.02 V a teplota je bezpečná, spustí se balancování na článku s nejvyšším napětím.
  if (delta > 0.02f && safeTemp)
  {
    if (!balancing)
    {
      uint16_t mask = (1 << iMax); // Bit odpovídající cell(iMax+1)
      // timeCode 0x03 např. znamená 60 s balancování
      bq.startBalancingManual(mask, 0x03, true, true);
      balancing = true;
      Serial.print("Balancing started on cell #");
      Serial.println(iMax + 1);
    }
  }
  else
  {
    if (balancing)
    {
      bq.stopAllBalancing();
      balancing = false;
      Serial.println("Balancing stopped");
    }
  }

  // Výpis výsledků do seriového monitoru
  Serial.println("-------------------------------------------------");
  Serial.println("Cell Voltages (V):");
  for (int i = 0; i < 14; i++)
  {
    Serial.print("  Cell ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(cellVolt[i], 3);
    if (i == iMax)
      Serial.print(" (max)");
    if (i == iMin)
      Serial.print(" (min)");
    Serial.println(" V");
  }
  Serial.println("Temperatures (°C):");
  for (int t = 0; t < 8; t++)
  {
    Serial.print("  T");
    Serial.print(t + 1);
    Serial.print(": ");
    Serial.print(cellTemp[t], 1);
    if (cellTemp[t] >= 60.0f)
      Serial.print(" [ALERT]");
    Serial.println(" °C");
  }
  Serial.print("Delta = ");
  Serial.print(delta, 3);
  Serial.println(" V");
  if (balancing)
  {
    Serial.print("Balancing active on cell #");
    Serial.println(iMax + 1);
  }
  else
  {
    Serial.println("No balancing active");
  }
  Serial.println("-------------------------------------------------\n");

  delay(1000);
}
