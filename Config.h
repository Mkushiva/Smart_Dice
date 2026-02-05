#ifndef CONFIG_H
#define CONFIG_H

// --- DEBUG ---
// #define DEBUG_MODE 

// --- Piny (XIAO nRF52840) ---
#define PIN_OLED_SDA    D4
#define PIN_OLED_SCL    D5
#define STATUS_LED      LED_RED  // Czerwona dioda (statusy, błędy)
#define BLE_LED         LED_BLUE // Niebieska dioda (Bluetooth)

// --- BLUETOOTH LE ---
#define BLE_SERVICE_UUID        "19B10000-E8F2-537E-4F6C-D104768A1214"
#define BLE_CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"
#define BLE_CONFIG_UUID         "19B10002-E8F2-537E-4F6C-D104768A1214"

#define BLE_TX_POWER            -4   // dBm
#define BLE_ADV_INTERVAL        160  // 100ms

// --- Ustawienia Gry ---
#define DEFAULT_MIN_VAL 1
#define DEFAULT_MAX_VAL 6
#define MAX_DICE_COUNT  6    
#define SEQ_DISPLAY_TIME_MS 2000 
#define IDLE_TIMEOUT_MS  (30 * 60 * 1000) // 30 minut

// --- Szanse ---
#define DEFAULT_CHANCE_CRIT   50 
#define DEFAULT_CHANCE_EVENT  50  

// --- Akcelerometr ---
#define THRESHOLD_HARD_SHAKE 2.5f 
#define THRESHOLD_SOFT_SHAKE 1.5f 
#define THRESHOLD_RESET      1.2f 
#define GESTURE_WAIT_TIME_MS 350

// --- Stabilizacja ---
#define THRESHOLD_STABLE_MIN 0.90f 
#define THRESHOLD_STABLE_MAX 1.15f 
#define STABLE_TIME_REQ_MS   400  
#define MAX_ROLL_TIME_MS     4000 

// --- Animacja ---
#define ANIMATION_FRAME_DELAY 10
#define SWIPE_SPEED_PX        4
#define SHAKE_COOLDOWN_MS    1000 

#endif