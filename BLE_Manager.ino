#include <bluefruit.h>
#include "Config.h"

BLEService        diceService = BLEService(BLE_SERVICE_UUID);
BLECharacteristic diceCharacteristic = BLECharacteristic(BLE_CHARACTERISTIC_UUID);
BLECharacteristic configCharacteristic = BLECharacteristic(BLE_CONFIG_UUID);

// Deklaracje funkcji
void onConfigWrite(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len);
void onDisconnect(uint16_t conn_hdl, uint8_t reason);

void initBLE() {
  Bluefruit.begin();
  Bluefruit.setTxPower(BLE_TX_POWER); 
  Bluefruit.setName("SmartDice");

  Bluefruit.autoConnLed(false); 
  
  pinMode(BLE_LED, OUTPUT);
  digitalWrite(BLE_LED, HIGH); 

  // Wyłączamy automat, przejmujemy kontrolę
  Bluefruit.Advertising.restartOnDisconnect(false);
  // Rejestrujemy callback rozłączenia
  Bluefruit.Periph.setDisconnectCallback(onDisconnect);

  diceService.begin();
  
  diceCharacteristic.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  diceCharacteristic.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  diceCharacteristic.setMaxLen(MAX_DICE_COUNT + 1);
  diceCharacteristic.begin();
  
  configCharacteristic.setProperties(CHR_PROPS_WRITE | CHR_PROPS_WRITE_WO_RESP);
  configCharacteristic.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  configCharacteristic.setFixedLen(5); 
  configCharacteristic.setWriteCallback(onConfigWrite);
  configCharacteristic.begin();
}

// Funkcja włączająca Bluetooth (gdy podłączono zasilanie)
void enableBLE() {
  if (!Bluefruit.connected() && !Bluefruit.Advertising.isRunning()) {
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addService(diceService);
    Bluefruit.ScanResponse.addName();
    Bluefruit.Advertising.setInterval(BLE_ADV_INTERVAL, BLE_ADV_INTERVAL); 
    
    // START NA 60 SEKUND po podłączeniu kabla
    Bluefruit.Advertising.start(60); 
  }
}

// Funkcja wywoływana przy odłączeniu kabla (w main.ino)
// Tylko zatrzymuje szukanie, ale NIE zrywa aktywnego połączenia
void restrictBLEOnBattery() {
  if (Bluefruit.Advertising.isRunning()) {
    Bluefruit.Advertising.stop();
  }
  digitalWrite(BLE_LED, HIGH);
}

// Całkowite wyłączenie (Deep Sleep)
void disableBLE() {
  if (Bluefruit.Advertising.isRunning()) {
    Bluefruit.Advertising.stop();
  }
  Bluefruit.disconnect(0); 
  digitalWrite(BLE_LED, HIGH);
}

// --- ZMODYFIKOWANA LOGIKA PO ROZŁĄCZENIU ---
void onDisconnect(uint16_t conn_hdl, uint8_t reason) {
  // Niezależnie czy jesteśmy na baterii, czy na kablu:
  // Jeśli połączenie zostało zerwane (np. uciekł zasięg, reset apki),
  // dajemy 60 sekund szansy na powrót.
  
  Bluefruit.Advertising.start(60);
  
  // Opcjonalnie: Mignij diodą, żeby użytkownik wiedział, że szuka
  // (Obsługa diody jest w main loop, więc tutaj tylko startujemy advertising)
}

void onConfigWrite(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  if (len >= 5) {
    setDiceConfig(data[0], data[1], data[2], data[3], data[4]);
    digitalWrite(STATUS_LED, LOW); delay(50); digitalWrite(STATUS_LED, HIGH);
  }
}

void sendBleResults() {
  if (Bluefruit.connected()) {
    int count = getDiceCount();
    int* res = getResults();
    bool* crits = getCriticals();     
    bool evt = isRandomEvent();

    uint8_t data[MAX_DICE_COUNT + 1];
    data[0] = evt ? 1 : 0;

    for(int i=0; i<count; i++) {
      uint8_t val = (uint8_t)res[i];
      if (crits[i]) val |= 0x80; 
      data[i+1] = val;
    }
    diceCharacteristic.notify(data, count + 1);
  }
}