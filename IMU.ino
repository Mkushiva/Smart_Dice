#include <LSM6DS3.h>
#include <Wire.h>
#include "Config.h"

// Obiekt czujnika (Adres 0x6A dla wersji Sense)
LSM6DS3 imu(I2C_MODE, 0x6A);

unsigned long lastShakeTime = 0;

void initIMU() {
  if (imu.begin() != 0) {
    Serial.println("IMU Error!");
    while(1); // Zawieś system jeśli brak czujnika
  }
}

// Zwraca: 0 (Brak), 1 (Soft), 2 (Hard)
int getMotionGesture() {
  // Zmienne statyczne (pamięć między wywołaniami)
  static bool isMeasuring = false;
  static float peakForce = 0;
  static unsigned long gestureStartTime = 0;
  
  // Nowe zmienne do obsługi "ciszy" w trakcie machania
  static bool isInQuietZone = false;
  static unsigned long quietZoneStartTime = 0;

  // 1. Blokada po poprzednim rzucie (Cooldown)
  if (millis() - lastShakeTime < SHAKE_COOLDOWN_MS) {
    return 0;
  }

  // 2. Odczyt
  float x = imu.readFloatAccelX();
  float y = imu.readFloatAccelY();
  float z = imu.readFloatAccelZ();
  float gForce = sqrt(x*x + y*y + z*z);

  // --- LOGIKA GESTÓW v2 ---

  // A. START POMIARU
  if (!isMeasuring && gForce > THRESHOLD_SOFT_SHAKE) {
    isMeasuring = true;
    peakForce = gForce;
    gestureStartTime = millis();
    isInQuietZone = false; // Reset strefy ciszy
    return 0;
  }

  // B. W TRAKCIE POMIARU
  if (isMeasuring) {
    // Aktualizuj szczyt (Peak Hold)
    if (gForce > peakForce) {
      peakForce = gForce;
    }

    // PRIORYTET: Czy przebiliśmy HARD? -> Rzut natychmiastowy
    // To przerywa wszelkie czekanie, bo rzut to rzut.
    if (peakForce > THRESHOLD_HARD_SHAKE) {
      isMeasuring = false;
      peakForce = 0;
      isInQuietZone = false;
      lastShakeTime = millis(); 
      Serial.println(">>> HARD SHAKE (Instant)");
      return 2; // KOD 2: RZUT
    }

    // OBSŁUGA KOŃCA GESTU (Z opóźnieniem)
    if (gForce < THRESHOLD_RESET) {
      // Siła spadła. Sprawdzamy czy to dopiero początek spadku?
      if (!isInQuietZone) {
        isInQuietZone = true;
        quietZoneStartTime = millis();
      }
      
      // Jeśli jesteśmy w strefie ciszy już wystarczająco długo...
      if (millis() - quietZoneStartTime > GESTURE_WAIT_TIME_MS) {
        // ...to znaczy, że to faktycznie koniec gestu.
        isMeasuring = false;
        isInQuietZone = false;
        
        // Decyzja co to było (Soft czy Szum)
        if (peakForce > THRESHOLD_SOFT_SHAKE) {
          lastShakeTime = millis();
          Serial.println(">>> SOFT SHAKE (Confirmed after wait)");
          return 1; // KOD 1: POKAŻ OSTATNI WYNIK
        } else {
          return 0; // Za słabe, ignoruj
        }
      }
    } 
    else {
      // Siła WZROSŁA powyżej RESET (czyli użytkownik machnął znowu w drugą stronę)
      // Anulujemy odliczanie końca gestu!
      if (isInQuietZone) {
        // Serial.println("... resume gesture ..."); // Debug
        isInQuietZone = false; 
      }
    }

    // Timeout bezpieczeństwa (gdyby ktoś machał lekko w nieskończoność)
    if (millis() - gestureStartTime > 3000) {
      isMeasuring = false;
      isInQuietZone = false;
      return 0;
    }
  }

  return 0;
}

// Sprawdza, czy kostka przestała się ruszać
bool isDiceStable() {
  float x = imu.readFloatAccelX();
  float y = imu.readFloatAccelY();
  float z = imu.readFloatAccelZ();

  float gForce = sqrt(x*x + y*y + z*z);

  // Kostka jest stabilna, jeśli siła wypadkowa to mniej więcej 1G (sama grawitacja)
  // Jeśli jest wstrząs lub obrót, siła będzie inna (np. 0.5G przy spadaniu, 2G przy uderzeniu)
  if (gForce > THRESHOLD_STABLE_MIN && gForce < THRESHOLD_STABLE_MAX) {
    return true;
  } else {
    return false;
  }
}