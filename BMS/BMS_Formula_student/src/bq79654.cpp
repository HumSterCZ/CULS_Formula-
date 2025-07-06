#include "bq79654.h"

/*
 * BQ_RESPONSE_TIMEOUT:
 * Maximální čekací doba (v ms) na odpověď (ACK) z BQ po zápisu/čtení registru.
 * Pokud dojde k timeoutu, vyvolá se disconnectBattery().
 */
static const unsigned long BQ_RESPONSE_TIMEOUT = 50;

/*
 * Definice příkazů (CMD) pro komunikaci s BQ v režimu single-device.
 * CMD_SINGLE_WRITE (0x00) a CMD_SINGLE_READ (0x01) se používají k odeslání požadavků.
 * Odpovědi mají nastavený bit7 (0x80/0x81).
 */
static const uint8_t CMD_SINGLE_WRITE = 0x00;
static const uint8_t CMD_SINGLE_READ  = 0x01;
static const uint8_t CMD_ACK_WRITE    = 0x80;
static const uint8_t CMD_ACK_READ     = 0x81;

/*
 * Status kódy v ACK
 * STATUS_OK = 0x00 značí, že operace proběhla úspěšně.
 */
static const uint8_t STATUS_OK = 0x00;

/* --------------------------------------------------------------------------
   Konstruktor: Uloží odkaz na HardwareSerial pro komunikaci s BQ79654.
   Např. použití: BQ79654 bq(Serial1);
-------------------------------------------------------------------------- */
BQ79654::BQ79654(HardwareSerial &port) : _serial(port)
{
}

/* --------------------------------------------------------------------------
   begin(baudRate):
   Inicializuje sériovou komunikaci na zadané rychlosti (typicky 1 Mb/s).
-------------------------------------------------------------------------- */
void BQ79654::begin(unsigned long baudRate)
{
    _serial.begin(baudRate);
    delay(10);
}

/* --------------------------------------------------------------------------
   wakeDevice():
   Reálný WAKE ping: 
     1. Ukončí se hardware Serial,
     2. Pin TX (pin 1 na Leonardo) se přepne do režimu OUTPUT a drží LOW ~2 ms,
     3. Poté se opět nastaví HIGH a znovu spustí Serial.
   Tím se BQ probudí z SHUTDOWN.
-------------------------------------------------------------------------- */
void BQ79654::wakeDevice()
{
    // Ukončíme hardware Serial, aby bylo možné přepnout TX pin do režimu GPIO.
  _serial.end();
  delay(2);

  // Nastavíme TX pin (na Arduino Leonardo je to pin 1) jako výstup.
  pinMode(1, OUTPUT);
  
  // Generujeme periodický signál s periodou 2 ms (1 ms HIGH, 1 ms LOW)
  // – opakujeme to např. 10 krát (tedy cca 20 ms probuzení).
  for (int i = 0; i < 10; i++) {
    digitalWrite(1, HIGH);
    delayMicroseconds(4000);  // 1 ms HIGH
    digitalWrite(1, LOW);
    delayMicroseconds(2000);  // 1 ms LOW
  }

  // Po probuzení přepneme pin zpět do režimu hardware Serial.
  _serial.begin(1000000);
  delay(5);
}

/* --------------------------------------------------------------------------
   enterSleep():
   Přečte registr CONTROL1, nastaví bit 5 (GOTO_SLEEP) a zapíše zpět.
-------------------------------------------------------------------------- */
void BQ79654::enterSleep()
{
    uint8_t val = 0;
    if(readReg(REG_CONTROL1, &val, 1) == 0)
    {
        val |= (1 << 5);
        writeReg(REG_CONTROL1, &val, 1);
    }
}

/* --------------------------------------------------------------------------
   shutdownDevice():
   Přečte registr CONTROL1, nastaví bit 4 (GOTO_SHUTDOWN) a zapíše zpět.
-------------------------------------------------------------------------- */
void BQ79654::shutdownDevice()
{
    uint8_t val = 0;
    if(readReg(REG_CONTROL1, &val, 1) == 0)
    {
        val |= (1 << 4);
        writeReg(REG_CONTROL1, &val, 1);
    }
}

/* --------------------------------------------------------------------------
   disconnectBattery():
   Bezpečnostní funkce – okamžitě odpojí baterii.
   Nastaví definovaný pin (RELAY_PIN, zde pin 5) do LOW a zůstane v nekonečné smyčce.
-------------------------------------------------------------------------- */
void BQ79654::disconnectBattery()
{
    const int RELAY_PIN = 5; // Pin pro relé/MOSFET
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    //while(true) delay(1000);
}

/* --------------------------------------------------------------------------
   writeReg(regAddr, data, len):
   Sestaví TX rámec s příkazem CMD_SINGLE_WRITE, odesílá jej a čeká na ACK.
   Pokud ACK neodpovídá (cmd=0x80, status=0) nebo dojde k timeoutu, volá disconnectBattery().
   Vrací 0 při úspěchu, jiný kód při chybě.
-------------------------------------------------------------------------- */
uint8_t BQ79654::writeReg(uint16_t regAddr, const uint8_t *data, uint8_t len)
{
    uint8_t txFrame[64];
    uint8_t txLen = 0;
    buildTxFrame(CMD_SINGLE_WRITE, regAddr, data, len, txFrame, txLen);

    while(_serial.available()) _serial.read();

    _serial.write(txFrame, txLen);
    _serial.flush();

     unsigned long start = millis();
    while(_serial.available() < 5)
    {
        if(millis() - start > BQ_RESPONSE_TIMEOUT)
        {
            Serial.println("ERR: writeReg TIMEOUT => Battery Disconnected!");
            disconnectBattery();
            return 1;
        }
    }

    uint8_t rxBuf[64];
    uint8_t idx = 0;
    delay(1);
    while(_serial.available() && idx < 64)
    {
        rxBuf[idx++] = _serial.read();
        delayMicroseconds(200);
    }

    uint8_t ackData[32];
    uint8_t ackLen = 0;
    if(!parseRxFrame(rxBuf, idx, ackData, ackLen))
    {
        Serial.println("ERR: parse fail in writeReg => disconnectBattery!");
        disconnectBattery();
        return 2;
    }
    if(ackLen < 2)
    {
        Serial.println("ERR: ACK too short in writeReg => disconnectBattery!");
        disconnectBattery();
        return 3;
    }
    uint8_t ackCmd = ackData[0];
    uint8_t ackStat = ackData[1];
    if(ackCmd != CMD_ACK_WRITE)
    {
        Serial.print("ERR: invalid ACK cmd in writeReg: 0x");
        Serial.println(ackCmd, HEX);
        disconnectBattery();
        return 4;
    }
    if(ackStat != STATUS_OK)
    {
        Serial.print("ERR: ACK status in writeReg: ");
        Serial.println(ackStat);
        disconnectBattery();
        return 5;
    }
    return 0;
}

/* --------------------------------------------------------------------------
   readReg(regAddr, data, len):
   Sestaví TX rámec s příkazem CMD_SINGLE_READ a čeká na odpověď s CMD_ACK_READ.
   Při chybě vyvolá disconnectBattery(). Vrací 0 při úspěchu.
-------------------------------------------------------------------------- */
uint8_t BQ79654::readReg(uint16_t regAddr, uint8_t *data, uint8_t len)
{
    uint8_t txFrame[64];
    uint8_t txLen = 0;
    buildTxFrame(CMD_SINGLE_READ, regAddr, nullptr, len, txFrame, txLen);

    while(_serial.available()) _serial.read();

    _serial.write(txFrame, txLen);
    _serial.flush();

    unsigned long start = millis();
    while(_serial.available() < 5)
    {
        if(millis() - start > BQ_RESPONSE_TIMEOUT)
        {
            Serial.println("ERR: readReg TIMEOUT => disconnectBattery!");
            disconnectBattery();
            return 1;
        }
    }

    uint8_t rxBuf[64];
    uint8_t idx = 0;
    delay(1);
    while(_serial.available() && idx < 64)
    {
        rxBuf[idx++] = _serial.read();
        delayMicroseconds(200);
    }

    uint8_t ackData[64];
    uint8_t ackLen = 0;
    if(!parseRxFrame(rxBuf, idx, ackData, ackLen))
    {
        Serial.println("ERR: parse fail in readReg => disconnectBattery!");
        disconnectBattery();
        return 2;
    }
    if(ackLen < 3)
    {
        Serial.println("ERR: ACK too short in readReg => disconnectBattery!");
        disconnectBattery();
        return 3;
    }
    uint8_t ackCmd = ackData[0];
    uint8_t ackStat = ackData[1];
    uint8_t ackDLen = ackData[2];

    if(ackCmd != CMD_ACK_READ)
    {
        Serial.print("ERR: Not a read ACK, cmd=0x");
        Serial.println(ackCmd, HEX);
        disconnectBattery();
        return 4;
    }
    if(ackStat != STATUS_OK)
    {
        Serial.print("ERR: ACK status in readReg: ");
        Serial.println(ackStat);
        disconnectBattery();
        return 5;
    }
    if(ackDLen < len)
    {
        Serial.println("WARN: ACK data length less than requested");
    }
    for(uint8_t i = 0; i < len && i < ackDLen; i++)
    {
        data[i] = ackData[3 + i];
    }
    return 0;
}

/* --------------------------------------------------------------------------
   parseRxFrame():
   Ověří, že přijatý rámec má minimálně 5 bajtů, zkontroluje CRC16 a extrahuje:
     - Byte0: SourceID (ignorujeme)
     - Byte1: ACK command (CMD_ACK_WRITE nebo CMD_ACK_READ)
     - Byte2: Status
     - Pokud je ACK typu read (CMD_ACK_READ), Byte3 je data length a následují data.
   Vrací true při úspěchu, jinak false.
-------------------------------------------------------------------------- */
bool BQ79654::parseRxFrame(uint8_t *rxBuf, uint8_t rxLen,
                           uint8_t *dataOut, uint8_t &dataLen)
{
    if(rxLen < 5)
        return false;

    uint16_t calc = computeCRC(rxBuf, rxLen - 2);
    uint16_t rxCrc = (rxBuf[rxLen - 2] << 8) | (rxBuf[rxLen - 1]);
    if(calc != rxCrc)
    {
        Serial.print("CRC fail: calc=0x");
        Serial.print(calc, HEX);
        Serial.print(", rx=0x");
        Serial.print(rxCrc, HEX);
        Serial.println();
        return false;
    }

    uint8_t ackCmd = rxBuf[1];
    uint8_t status = rxBuf[2];
    dataOut[0] = ackCmd;
    dataOut[1] = status;

    if(ackCmd == CMD_ACK_READ)
    {
        if(rxLen < 6)
        {
            dataLen = 2;
            return true;
        }
        uint8_t dl = rxBuf[3];
        dataOut[2] = dl;
        for(uint8_t i = 0; i < dl && (4 + i) < (rxLen - 2); i++)
        {
            dataOut[3 + i] = rxBuf[4 + i];
        }
        dataLen = 3 + dl;
        return true;
    }
    else if(ackCmd == CMD_ACK_WRITE)
    {
        dataLen = 2;
        return true;
    }
    else
    {
        dataLen = 2;
        return true;
    }
}

/* --------------------------------------------------------------------------
   buildTxFrame():
   Sestaví TX rámec ve formátu:
     [DestID (1B)=0, CMD (1B), regAddr (2B), dataLength (1B), data..., CRC16 (2B)]
   Vrací výsledek v outFrame a jeho délku v outLen.
-------------------------------------------------------------------------- */
void BQ79654::buildTxFrame(uint8_t cmd, uint16_t regAddr,
                           const uint8_t *payload, uint8_t paylen,
                           uint8_t *outFrame, uint8_t &outLen)
{
    uint8_t idx = 0;
    outFrame[idx++] = 0x00; // DestID
    outFrame[idx++] = cmd;
    outFrame[idx++] = (regAddr >> 8) & 0xFF; // regAddr Hi
    outFrame[idx++] = regAddr & 0xFF;          // regAddr Lo
    outFrame[idx++] = paylen;                  // dataLength

    for(uint8_t i = 0; i < paylen; i++)
    {
        outFrame[idx++] = (payload ? payload[i] : 0);
    }

    uint16_t crc = computeCRC(outFrame, idx);
    outFrame[idx++] = (crc >> 8) & 0xFF;
    outFrame[idx++] = crc & 0xFF;

    outLen = idx;
}

/* --------------------------------------------------------------------------
   computeCRC():
   Jediná implementace CRC16 CCITT-FALSE (polynom 0x1021, init=0x0000).
-------------------------------------------------------------------------- */
uint16_t BQ79654::computeCRC(const uint8_t *data, uint8_t len)
{
    uint16_t crc = 0x0000;
    for(uint8_t i = 0; i < len; i++)
    {
        crc ^= (data[i] << 8);
        for(uint8_t b = 0; b < 8; b++)
        {
            if(crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/* --------------------------------------------------------------------------
   initSingleDevice():
   Nastaví zařízení jako single-device:
     - REG_COMM_CTRL = 0x02 (base+top)
     - REG_ACTIVE_CELL = 14 (0x0E)
-------------------------------------------------------------------------- */
void BQ79654::initSingleDevice()
{
    uint8_t cval = 0x02;
    writeReg(REG_COMM_CTRL, &cval, 1);

    uint8_t numCell = 0x0E; // 14 článků
    writeReg(REG_ACTIVE_CELL, &numCell, 1);
}

/* --------------------------------------------------------------------------
   setupProtection():
   Nastaví OV/UV prahy (natvrdo např. OV=4,25 V a UV=2,80 V) a spustí měření.
-------------------------------------------------------------------------- */
void BQ79654::setupProtection(const bqUVOV_t &limits)
{
    uint8_t ovCode = 0x3E; // ~4,25 V
    uint8_t uvCode = 0x20; // ~2,80 V
    writeReg(REG_OV_THRESH, &ovCode, 1);
    writeReg(REG_UV_THRESH, &uvCode, 1);

    uint8_t ctrl = 0x05; // Spustit round-robin: bit2=1 (GO), bit1..0=01 (mode)
    writeReg(REG_OVUV_CTRL, &ctrl, 1);
}

/* --------------------------------------------------------------------------
   enableOpenWire():
   Zatím pouze placeholder – zapíše 0x01 do REG_DIAG_CTRL3.
-------------------------------------------------------------------------- */
void BQ79654::enableOpenWire()
{
    uint8_t val = 0x01;
    writeReg(REG_DIAG_CTRL3, &val, 1);
}

/* --------------------------------------------------------------------------
   configMainADC14sContinuous():
   Konfiguruje ADC v režimu continuous:
     - V REG_ADC_CTRL1 nastaví bit[2] (CS_MAIN_GO) a MAIN_MODE=0x02.
-------------------------------------------------------------------------- */
void BQ79654::configMainADC14sContinuous()
{
    uint8_t val = 0;
    readReg(REG_ADC_CTRL1, &val, 1);
    val &= 0xF0;
    val |= (1 << 2); // CS_MAIN_GO = 1
    val |= 0x02;     // MAIN_MODE = 2 (continuous)
    writeReg(REG_ADC_CTRL1, &val, 1);
}

/* --------------------------------------------------------------------------
   isMainADCReady():
   Kontroluje registr ADC_STAT1, zda je nastaven bit 5 (DRDY_MAIN_ADC).
-------------------------------------------------------------------------- */
bool BQ79654::isMainADCReady()
{
    uint8_t st = 0;
    readReg(REG_ADC_STAT1, &st, 1);
    return ((st & 0x20) != 0);
}

/* --------------------------------------------------------------------------
   readCellVoltages():
   Pro daný počet článků (max 14) přečte 2 bajty (hi/lo) z registrů VCELL a přepočítá napětí.
-------------------------------------------------------------------------- */
void BQ79654::readCellVoltages(float *voltages, uint8_t count)
{
    if(count > 14)
        count = 14;
    for(uint8_t i = 0; i < count; i++)
    {
        uint16_t hiReg = vcellHiReg(i + 1);
        uint16_t loReg = vcellLoReg(i + 1);
        uint8_t hiV = 0, loV = 0;
        readReg(hiReg, &hiV, 1);
        readReg(loReg, &loV, 1);
        int16_t raw = (int16_t)((hiV << 8) | loV);
        float uv = raw * VLSB_CELL_uV;
        voltages[i] = uv / 1000000.0f; // převod na volty
    }
}

/* --------------------------------------------------------------------------
   readTemperatures():
   Čte hodnoty z GPIO (AUX ADC) pro 8 senzorů, přepočítá napětí a následně pomocí Beta rovnice vypočítá teplotu (°C).
-------------------------------------------------------------------------- */
void BQ79654::readTemperatures(float *temps, uint8_t count)
{
    if(count > 8)
        count = 8;
    for(uint8_t m = 0; m < count; m++)
    {
        uint16_t hiReg = gpioHiReg(m + 1);
        uint16_t loReg = gpioLoReg(m + 1);
        uint8_t hiV = 0, loV = 0;
        readReg(hiReg, &hiV, 1);
        readReg(loReg, &loV, 1);
        int16_t raw = (int16_t)((hiV << 8) | loV);
        float uv = raw * VLSB_GPIO_uV;
        float vMeas = uv / 1000000.0f;
        if(vMeas < 0.0001f)
            vMeas = 0.0001f;
        if(vMeas > (V_REF - 0.0001f))
            vMeas = V_REF - 0.0001f;
        float rNtc = R_PULL * (vMeas / (V_REF - vMeas));
        float tKel = 1.0f / ((1.0f / T25_K) + (1.0f / NTC_BETA) * logf(rNtc / NTC_R25));
        temps[m] = tKel - 273.15f;
    }
}

/* --------------------------------------------------------------------------
   startBalancingManual():
   Pro manuální balancování. Funkce:
     - Prochází 16 buněk; podle masky (bit i = cell(i+1)) zapíše do registru CB_CELLn_CTRL hodnotu timeCode,
       která definuje dobu balancování (např. 0x03 => 60 s).
     - Poté nastaví BAL_CTRL2 (0x032F) s bity: BAL_GO=1, AUTO_BAL=0, případně FLTSTOP_EN a OTCB_EN.
-------------------------------------------------------------------------- */
uint8_t BQ79654::startBalancingManual(uint16_t mask, uint8_t timeCode, bool faultStop, bool otcbEnable)
{
    uint8_t countOn = 0;
    for(uint8_t i = 0; i < 16; i++)
    {
        bool en = ((mask & (1 << i)) != 0);
        if(en)
            countOn++;
        if(countOn > 8)
        {
            Serial.println("ERR: More than 8 cells selected for balancing!");
            return 1;
        }
        // Výpočet adresy: pro cell i+1, adresa = 0x0318 + (16 - (i+1))
        uint16_t rAddr = 0x0318 + (16 - (i + 1));
        uint8_t val = en ? timeCode : 0x00;
        writeReg(rAddr, &val, 1);
    }
    // Nastavení BAL_CTRL2
    // Bit1 = BAL_GO = 1, Bit0 = AUTO_BAL = 0; volitelně nastavíme FLTSTOP_EN (bit5) a OTCB_EN (bit4)
    uint8_t ctrl = 0x02;
    if(faultStop)
        ctrl |= (1 << 5);
    if(otcbEnable)
        ctrl |= (1 << 4);
    writeReg(BAL_CTRL2, &ctrl, 1);

    return 0;
}

/* --------------------------------------------------------------------------
   stopAllBalancing():
   Vynuluje hodnoty v registrech CB_CELLn_CTRL pro všechny 16 buněk a opět spustí BAL_CTRL2,
   což zastaví aktivní balancování.
-------------------------------------------------------------------------- */
uint8_t BQ79654::stopAllBalancing()
{
    for(uint8_t i = 1; i <= 16; i++)
    {
        uint16_t rAddr = 0x0318 + (16 - i);
        uint8_t zero = 0;
        writeReg(rAddr, &zero, 1);
    }
    uint8_t ctrl = 0x02; // Spustí se, ale protože time=0, balancing se neprovede.
    writeReg(BAL_CTRL2, &ctrl, 1);
    return 0;
}

/* --------------------------------------------------------------------------
   vcellHiReg(cellIndex):
   Vrací adresu HI bajtu pro cell(cellIndex).
   Např. pro cell16 vrací 0x0568; pro cell15 vrací 0x0566.
-------------------------------------------------------------------------- */
uint16_t BQ79654::vcellHiReg(uint8_t cellIndex)
{
    uint8_t diff = 16 - cellIndex;
    return (VCELL16_HI - 2 * diff);
}

/* --------------------------------------------------------------------------
   vcellLoReg(cellIndex):
   Vrací adresu LO bajtu pro cell(cellIndex).
-------------------------------------------------------------------------- */
uint16_t BQ79654::vcellLoReg(uint8_t cellIndex)
{
    return vcellHiReg(cellIndex) + 1;
}

/* --------------------------------------------------------------------------
   gpioHiReg(gpioIndex):
   Vrací adresu HI bajtu pro GPIO(gpioIndex).
   Např. pro GPIO1 vrací 0x058E.
-------------------------------------------------------------------------- */
uint16_t BQ79654::gpioHiReg(uint8_t gpioIndex)
{
    return (GPIO1_HI + 2 * (gpioIndex - 1));
}

/* --------------------------------------------------------------------------
   gpioLoReg(gpioIndex):
   Vrací adresu LO bajtu pro GPIO(gpioIndex).
-------------------------------------------------------------------------- */
uint16_t BQ79654::gpioLoReg(uint8_t gpioIndex)
{
    return gpioHiReg(gpioIndex) + 1;
}
