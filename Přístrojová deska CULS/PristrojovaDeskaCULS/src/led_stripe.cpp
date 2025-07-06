// ===================INCLUDES===================
#include <led_stripe.h>

 
CRGB leds[NUM_LEDS];

// Časovač pro efekt odrážení
unsigned long previousMillis = 0;
const long interval = 250;  // Interval pro blikání LED (250 ms)
const long intervalRPM = 60;  // Interval pro blikání LED (250 ms)
bool blinkState = false;

// ===================INICIALIZATION===================
// Inicializace pomocí knihovny FastLED
void ledStripeInit(){
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
}

// ===================FUNKCTIONS===================
// Funkce pro mýchání barev
CRGB blendColors(CRGB color1, CRGB color2, uint8_t blendAmount) {
  return blend(color1, color2, blendAmount);
}

void setLEDs(int rpm, int gear, int waterTemp, int oilTemp) {
  setRPM(rpm);
  setGear(gear);
  setWaterTemp(waterTemp);
  setOilTemp(oilTemp);
  //leds[21] = CRGB::Red;
  //leds[22] = CRGB::Black;
  // leds[23] = CRGB::Red;
  FastLED.show();
}

void setRPM(int rpm) {
  if (rpm < 12000) {
    int rpmIndex = map(rpm, 6000, 12000, 19, 0);
    for (int i = 19; i >= 0; i--) {
      if (i >= rpmIndex) {
        uint8_t blendAmount = map(i, 19, 0, 0, 255);
        leds[i] = blendColors(CRGB::Green, CRGB::Red, blendAmount);
      } else {
        leds[i] = CRGB::Black;
      }
    }
  } else if (rpm < 12300) {
    for (int i = 19; i >= 0; i--) {
      leds[i] = CRGB::Red;
    }
  } else if (rpm < 12500) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= intervalRPM) {
      previousMillis = currentMillis;
      blinkState = !blinkState;
    }
    CRGB color = blinkState ? CRGB::Red : CRGB::Black;

    for (int i = 19; i >= 0; i--) {
      leds[i] = color;
    }
  }
}

void setGear(int gear) {
  switch (gear) {
    case 0:  // Neutrál
      leds[20] = CRGB::Orange;
      leds[21] = CRGB::Black;
      leds[22] = CRGB::Black;
      break;
    case 1:  // Zařazený stupeň 1
      leds[20] = CRGB::Black;
      leds[21] = CRGB::Green;
      leds[22] = CRGB::Black;
      break;
    case 2:  // Zařazený stupeň 2
      leds[20] = CRGB::Black;
      leds[21] = CRGB::Black;
      leds[22] = CRGB::Red;
      break;
      //default:
      //  break;
  }
}

void setWaterTemp(int waterTemp) {
  // Reset all related LEDs
  leds[23] = CRGB::Black;
  leds[24] = CRGB::Black;
  leds[25] = CRGB::Black;
  leds[26] = CRGB::Black;
  leds[27] = CRGB::Black;

  if (waterTemp < 50) {
    leds[23] = CRGB::Blue;
    leds[27] = CRGB::Blue;
  } else if (waterTemp < 85) {
    leds[23] = CRGB::Blue;
    leds[24] = CRGB::Green;
    leds[25] = CRGB::Green;
    leds[26] = CRGB::Green;
    leds[27] = CRGB::Blue;
  } else if (waterTemp < 95) {
    leds[23] = CRGB::Blue;
    leds[24] = CRGB::Red;
    leds[25] = CRGB::Red;
    leds[26] = CRGB::Red;
    leds[27] = CRGB::Blue;
  } else {
    // Blink effect
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      blinkState = !blinkState;
    }
    CRGB color = blinkState ? CRGB::Red : CRGB::Black;
    leds[23] = color;
    leds[24] = color;
    leds[25] = color;
    leds[26] = color;
    leds[27] = color;
  }
}


void setOilTemp(int oilTemp) {
  if (oilTemp < 60) {
    leds[28] = CRGB::Blue;
    leds[29] = CRGB::Blue;
    leds[30] = CRGB::Blue;
  } else if (oilTemp < 120) {
    leds[28] = CRGB::Green;
    leds[29] = CRGB::Green;
    leds[30] = CRGB::Green;
  } else if (oilTemp < 130) {
    leds[28] = CRGB::Orange;
    leds[29] = CRGB::Orange;
    leds[30] = CRGB::Orange;
  }

  else if (oilTemp < 140) {
    leds[28] = CRGB::Red;
    leds[29] = CRGB::Red;
    leds[30] = CRGB::Red;
  } else if (oilTemp < 150) {

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      blinkState = !blinkState;
    }
    CRGB color = blinkState ? CRGB::Red : CRGB::Black;
    leds[28] = color;
    leds[29] = color;
    leds[30] = color;
  }
}

void simulateValues() {
  static unsigned long lastUpdate = 0;
  static int rpm = 500;         // Otáčky motoru
  static int gear = 0;          // Zařazený převodový stupeň
  static int waterTemp = 40;    // Teplota vody
  static int oilTemp = 50;      // Teplota oleje
  static float rpmPhase = 0;    // Fáze pro kolísání otáček
  static bool waterTempIncreasing = true;
  static bool oilTempIncreasing = true;
  static bool inLimiter = false; // Indikátor, zda je motor v omezovači
  static unsigned long limiterTimer = 0;

  // Interval aktualizace simulace
  if (millis() - lastUpdate >= 50) {
    lastUpdate = millis();

    // Simulace kolísání otáček pomocí sinusové funkce
    if (!inLimiter) {
      rpmPhase += 0.1; // Zrychlujeme fázi otáček
      if (rpmPhase >= 6.28) rpmPhase = 0; // Reset fáze po 2π (1 cyklus sinusovky)

      rpm = 7000 + sin(rpmPhase) * 3000; // Střední hodnota 7000, amplituda 3000
      if (random(0, 10) > 8) { // Občasná nepravidelnost
        rpm += random(-500, 500);
      }

      // Pokud otáčky přesáhnou 12,500, přejdeme do omezovače
      if (rpm >= 12500) {
        inLimiter = true;
        limiterTimer = millis();
      }
    } else {
      // Simulace omezovače
      if (millis() - limiterTimer < 500) { // Omezovač běží 0,5 sekundy
        rpm = 12500 + random(-300, 300); // Rychlé kolísání kolem 12,500
      } else {
        inLimiter = false; // Po omezovači se vracíme k normálnímu průběhu
      }
    }

    rpm = constrain(rpm, 500, 13000); // Omezit otáčky mezi 500 a 13,000

    // Simulace zařazení převodového stupně
    static unsigned long lastGearChange = 0;
    if (millis() - lastGearChange >= 5000) {  // Řazení každých 5 sekund
      gear = (gear + 1) % 3;  // Přepínání mezi neutrál (0), 1, 2
      lastGearChange = millis();
    }

    // Simulace teploty vody
    if (waterTempIncreasing) {
      waterTemp++;
      if (waterTemp >= 90) waterTempIncreasing = false;
    } else {
      waterTemp--;
      if (waterTemp <= 40) waterTempIncreasing = true;
    }

    // Simulace teploty oleje
    if (oilTempIncreasing) {
      oilTemp++;
      if (oilTemp >= 130) oilTempIncreasing = false;
    } else {
      oilTemp--;
      if (oilTemp <= 50) oilTempIncreasing = true;
    }

    // Aktualizace LED na základě simulovaných hodnot
    setLEDs(rpm, gear, waterTemp, oilTemp);
  }
}