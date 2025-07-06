#ifndef BQ79654_H
#define BQ79654_H

#include <Arduino.h>
#include <math.h> // kvůli logf v teplotních výpočtech

/* --------------------------------------------------------------------------
   Definice registrů podle BQ79654-Q1 datasheetu
-------------------------------------------------------------------------- */

// Konfigurační registry
#define REG_COMM_CTRL      0x0308  ///< COMM_CTRL – nastavení base/top
#define REG_CONTROL1       0x0309  ///< CONTROL1 – SLEEP/SHUTDOWN, soft reset

// OV/UV prahy
#define REG_OV_THRESH      0x0009  ///< OV Threshold
#define REG_UV_THRESH      0x000A  ///< UV Threshold
#define REG_OVUV_CTRL      0x032C  ///< bit2=GO, bit1..0=MODE => spustí OV/UV

// Řízení ADC
#define REG_ADC_CTRL1      0x0520  ///< bit[2]=CS_MAIN_GO, bit[1..0]=MAIN_MODE
#define REG_ACTIVE_CELL    0x0522  ///< Počet článků (14 => 0x0E)
#define REG_ADC_STAT1      0x0527  ///< bit[5] = DRDY_MAIN_ADC

// Open-Wire
#define REG_DIAG_CTRL3     0x0333  ///< Placeholder pro open-wire enable

// Balancování
#define BAL_CTRL2          0x032F  ///< bit1=BAL_GO, bit0=AUTO_BAL, bit4=OTCB, bit5=FLTSTOP
#define CB_CELL16_CTRL     0x0318  ///< (Pak posun pro cell1 = 0x0327)

// VCELL (napětí článků)
#define VCELL16_HI         0x0568
#define VCELL16_LO         0x0569

// GPIO / AUX
#define GPIO1_HI           0x058E
#define GPIO1_LO           0x058F

/* --------------------------------------------------------------------------
   Konstanty pro ADC a NTC
-------------------------------------------------------------------------- */
#define VLSB_CELL_uV       190.73f  ///< LSB (µV) pro hlavní ADC článků
#define VLSB_GPIO_uV       152.59f  ///< LSB (µV) pro AUX ADC (GPIO)

#define NTC_R25            10000.0f ///< 10kΩ NTC
#define NTC_BETA           3950.0f
#define T25_K              298.15f
#define R_PULL             10000.0f ///< horní rezistor 10kΩ
#define V_REF              5.0f     ///< TSREF = 5 V

/* --------------------------------------------------------------------------
   Struktura pro OV/UV limit
-------------------------------------------------------------------------- */
typedef struct {
  float ovLimit; ///< ex: 4.25 V
  float uvLimit; ///< ex: 2.80 V
} bqUVOV_t;

/* --------------------------------------------------------------------------
   Třída BQ79654 - API pro ovládání jednoho BQ (14s)
-------------------------------------------------------------------------- */
class BQ79654 {
public:
    // Konstruktor, přijímá reference na HardwareSerial (např. Serial1)
    BQ79654(HardwareSerial &port);

    // Inicializace sériové linky (typ. 1 Mb/s)
    void begin(unsigned long baudRate = 1000000);

    // Probuzení (WAKE ping ~2ms na TX pin)
    void wakeDevice();
    // Přechod do SLEEP zápisem do CONTROL1
    void enterSleep();
    // Přechod do SHUTDOWN zápisem do CONTROL1
    void shutdownDevice();

    // Bezpečnost: odpojí baterii (pin 5 LOW) a skončí v nekonečné smyčce
    void disconnectBattery();

    // Zápis/čtení registru (s CRC a ACK)
    uint8_t writeReg(uint16_t regAddr, const uint8_t *data, uint8_t len);
    uint8_t readReg(uint16_t regAddr, uint8_t *data, uint8_t len);

    // Nastavit 14s, COMM_CTRL=0x02, ACTIVE_CELL=14
    void initSingleDevice();

    // OV/UV prahy => spustí round-robin
    void setupProtection(const bqUVOV_t &limits);

    // Zapnout open-wire (placeholder)
    void enableOpenWire();

    // Nastavit main ADC do continuous
    void configMainADC14sContinuous();
    // DRDY_MAIN_ADC=1 ?
    bool isMainADCReady();

    // Čtení napětí článků a teplot
    void readCellVoltages(float *voltages, uint8_t count);
    void readTemperatures(float *temps, uint8_t count);

    // Balancování (manuální)
    //   - mask: bit i => cell(i+1)
    //   - timeCode: např. 0x03 => 60 s
    //   - faultStop => bit5=FLTSTOP_EN
    //   - otcbEnable => bit4=OTCB_EN
    uint8_t startBalancingManual(uint16_t mask, uint8_t timeCode, bool faultStop, bool otcbEnable);

    // Vynuluje timery pro všech 16 cell, opět spustí => reálně se nic nestane
    uint8_t stopAllBalancing();

private:
    HardwareSerial &_serial;

    // Vytvoření TX rámce + parse
    void buildTxFrame(uint8_t cmd, uint16_t regAddr,
                      const uint8_t *payload, uint8_t paylen,
                      uint8_t *outFrame, uint8_t &outLen);
    bool parseRxFrame(uint8_t *rxBuf, uint8_t rxLen,
                      uint8_t *dataOut, uint8_t &dataLen);

    // Jediná definice computeCRC (CCITT-FALSE)
    uint16_t computeCRC(const uint8_t *data, uint8_t len);

    // Pomocné funkce – adresy
    uint16_t vcellHiReg(uint8_t cellIndex);
    uint16_t vcellLoReg(uint8_t cellIndex);
    uint16_t gpioHiReg(uint8_t gpioIndex);
    uint16_t gpioLoReg(uint8_t gpioIndex);
};

#endif // BQ79654_H
