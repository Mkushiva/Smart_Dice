#include "Config.h"
#include <Adafruit_TinyUSB.h>
#include <U8g2lib.h> 

extern U8G2_SSD1306_64X32_1F_F_HW_I2C u8g2;

// Prototypy funkcji z BLE_Manager
void enableBLE();
void restrictBLEOnBattery(); // Nowy prototyp
void disableBLE();

enum State {
  STATE_IDLE,           
  STATE_SLEEP_DEEP,     
  STATE_ANIMATING,      
  STATE_SHOW_SEQUENCE,  
  STATE_SHOW_DONE       
};

State currentState = STATE_IDLE;

// Timery
unsigned long stateTimer = 0;
unsigned long animationTimer = 0;
unsigned long rollingStartTime = 0;
unsigned long stabilityTimer = 0;   
unsigned long seqTimer = 0;
unsigned long lastActivityTime = 0; 
unsigned long ledBlinkTimer = 0; // Timer do migania niebieską diodą

// Zmienne animacji
int seqIndex = 0;
bool isSwiping = false;
int swipeOffset = 0;

// Flaga zasilania
bool isPoweredExternal = false;

void initWDT() {
  NRF_WDT->CONFIG = 0x01;
  NRF_WDT->CRV = (32768 * 2); 
  NRF_WDT->RREN = 0x01;       
  NRF_WDT->TASKS_START = 1;
}

void setup() {
  initDisplay();
  initIMU();
  initGameLogic();
  initBLE(); 
  initWDT(); 
  
  pinMode(STATUS_LED, OUTPUT); digitalWrite(STATUS_LED, HIGH);
  pinMode(BLE_LED, OUTPUT);
  digitalWrite(BLE_LED, HIGH);

  lastActivityTime = millis();
  blinkLED(STATUS_LED, 2); 
}

void loop() {
  NRF_WDT->RR[0] = 0x6E524635;

  // --- 1. DETEKCJA ZASILANIA (USB/Indukcja 5V) ---
  bool currentPowerState = (NRF_POWER->USBREGSTATUS & 3); // Bit 0 i 1 oznaczają VBUS detect/ready
  
  // Wykrycie zmiany stanu zasilania
  if (currentPowerState != isPoweredExternal) {
    isPoweredExternal = currentPowerState;
    
    if (isPoweredExternal) {
      // PODŁĄCZONO KABEL/INDUKCJĘ -> Włączamy możliwość łączenia
      enableBLE();  
      if (currentState == STATE_SLEEP_DEEP) wakeUpFromDeepSleep();
    } else {
      // ODŁĄCZONO KABEL (PRACA NA BATERII)
      // Nie wyłączamy BLE całkowicie! Tylko blokujemy nowe połączenia.
      restrictBLEOnBattery(); 
    }
  }

  // --- 2. OBSŁUGA DIODY NIEBIESKIEJ ---
  // Miga TYLKO gdy jesteśmy na zasilaniu zewnętrznym (sygnalizuje tryb parowania/gotowości)
  if (isPoweredExternal && currentState != STATE_SLEEP_DEEP) {
    if (millis() - ledBlinkTimer > 1000) {
      ledBlinkTimer = millis();
      digitalWrite(BLE_LED, !digitalRead(BLE_LED));
    }
  } else {
    // Na baterii dioda zawsze zgaszona (oszczędność + brak sygnalizacji parowania)
    digitalWrite(BLE_LED, HIGH);
  }

  // --- 3. ERROR HANDLING EKRANU ---
  if (!isDisplayConnected()) {
    digitalWrite(STATUS_LED, LOW); delay(50);
    digitalWrite(STATUS_LED, HIGH);
    if (isDisplayConnected()) initDisplay();
    return;
  }
  
  int gesture = getMotionGesture();
  if (gesture != 0) lastActivityTime = millis();

  // --- 4. MASZYNA STANÓW ---
  switch (currentState) {
    
    case STATE_IDLE:
      if (millis() - lastActivityTime > IDLE_TIMEOUT_MS) {
        goToDeepSleep();
        break;
      }

      if (gesture == 2) { 
        u8g2.setPowerSave(0);
        startRolling();
      } 
      else if (gesture == 1) { 
        u8g2.setPowerSave(0);
        recallSequence();
      }
      delay(40); 
      break;

    case STATE_SLEEP_DEEP:
      if (gesture == 2) { 
        wakeUpFromDeepSleep();
        u8g2.setPowerSave(0);
        startRolling();
      }
      delay(200);
      break;

    case STATE_ANIMATING: {
      lastActivityTime = millis();
      if (millis() - animationTimer > ANIMATION_FRAME_DELAY) {
        showRollingFrame();
        animationTimer = millis();
      }
      
      bool isStable = isDiceStable() && (millis() - stabilityTimer > STABLE_TIME_REQ_MS);
      bool isTimeout = (millis() - rollingStartTime > MAX_ROLL_TIME_MS);

      if (isStable || isTimeout) finishRolling();
      if (!isDiceStable()) stabilityTimer = millis();
      delay(2);
      break;
    }

    case STATE_SHOW_SEQUENCE: {
      lastActivityTime = millis();
      if (gesture == 2) { startRolling(); return; }

      int count = getDiceCount();
      int* results = getResults();
      bool* crits = getCriticals();
      bool evt = isRandomEvent();

      if (isSwiping) {
        int oldNum = results[seqIndex];
        int nextIdx = (seqIndex + 1) % count;
        int newNum = results[nextIdx];
        drawSwipeFrame(oldNum, newNum, evt, swipeOffset);
        swipeOffset += SWIPE_SPEED_PX;

        if (swipeOffset >= 32) {
          isSwiping = false;
          seqIndex++;
          seqTimer = millis(); 
        }
      }
      else {
        drawResult(results[seqIndex], crits[seqIndex], evt);
        if (millis() - seqTimer > SEQ_DISPLAY_TIME_MS) {
          if (seqIndex < count - 1) {
            isSwiping = true;
            swipeOffset = 0;
          } else {
            currentState = STATE_SHOW_DONE;
            stateTimer = millis();
          }
        }
      }
      delay(5);
      break;
    }

    case STATE_SHOW_DONE:
      lastActivityTime = millis();
      if (gesture == 2) { startRolling(); return; } 
      if (gesture == 1) { recallSequence(); return; } 
      
      if (millis() - stateTimer > 5000) {
        turnOffDisplay();
        u8g2.setPowerSave(1); 
        currentState = STATE_IDLE;
      }
      delay(50);
      break;
  }
}

// --- Funkcje Pomocnicze ---

void goToDeepSleep() {
  disableBLE(); // Tu wyłączamy całkowicie (disconnect), bo idziemy spać na długo
  u8g2.setPowerSave(1); 
  currentState = STATE_SLEEP_DEEP;
}

void wakeUpFromDeepSleep() {
  // Włączamy BLE tylko jeśli jest podłączone zasilanie
  if (isPoweredExternal) enableBLE();
  
  lastActivityTime = millis();
  currentState = STATE_IDLE;
  blinkLED(STATUS_LED, 1);
}

void startRolling() {
  currentState = STATE_ANIMATING;
  rollingStartTime = millis();
  stabilityTimer = millis();
}

void finishRolling() {
  generateResults();    
  sendBleResults(); // Wyśle jeśli połączony
  seqIndex = 0;
  seqTimer = millis();
  isSwiping = false;
  currentState = STATE_SHOW_SEQUENCE;
}

void recallSequence() {
  seqIndex = 0;
  seqTimer = millis();
  isSwiping = false;
  currentState = STATE_SHOW_SEQUENCE;
}

void blinkLED(int pin, int times) {
  for(int i=0; i<times; i++) {
    digitalWrite(pin, LOW);
    delay(100);
    digitalWrite(pin, HIGH); delay(100);
  }
}