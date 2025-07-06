#pragma once
//// ===================INCLUDES===================
#include <FastLED.h>

//// ===================DEFINES===================
// FastLED nastavení LED Pásku
#define LED_PIN     2
#define NUM_LEDS    31
#define BRIGHTNESS  100
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

// Rozsahy a barvy pro RPM
#define RPM_GREEN_START 500
#define RPM_GREEN_END   6000
#define RPM_YELLOW_START 6001
#define RPM_YELLOW_END   11000
#define RPM_RED_START    11001
#define RPM_LIMITER      12500

// Rozsahy a barvy pro teplotu vody
#define WATER_COLD 40
#define WATER_NORMAL 85
#define WATER_HOT 95

// Rozsahy a barvy pro teplotu oleje
#define OIL_COLD 50
#define OIL_NORMAL 120
#define OIL_HOT 130
#define OIL_LIMIT 140

//// ===================INICIALIZATION===================
// Inicializace pomocí knihovny FastLED.h
void ledStripeInit();

//// ===================FUNKCTIONS===================
// Funkce pro mýchání barev
CRGB blendColors(CRGB color1, CRGB color2, uint8_t blendAmount);

//Rosvicí led kompletní funkce pro RPM, GEAR, WATERTEMP, OILTEMP
void setLEDs(int rpm, int gear, int waterTemp, int oilTemp);

// Nastavení světelného rozhraní při různých úrovních teploty oleje
void setOilTemp(int oilTemp);

// Nastavení světelného rozhraní při různých úrovních teploty vody
void setWaterTemp(int waterTemp);

// Nastavení světelného rozhraní při různých rychlostních stupňů
void setGear(int gear);

// Nastavení světelného rozhraní při různých otáčkách motoru
void setRPM(int rpm);

// Simulace a ukázkový kód pro LED pásek
void simulateValues();