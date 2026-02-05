#include "Config.h"

int minRange = DEFAULT_MIN_VAL;
int maxRange = DEFAULT_MAX_VAL;
int diceCount = 1; 

int chanceCrit = DEFAULT_CHANCE_CRIT;  // Domyślnie 0 (wyłączone)
int chanceEvent = DEFAULT_CHANCE_EVENT; // Domyślnie 0 (wyłączone)

// Tablica na wyniki
int results[MAX_DICE_COUNT];
// Tablica na flagi krytyków
bool criticals[MAX_DICE_COUNT];
// Zmienna na zdarzenie losowe (globalne dla rzutu)
bool randomEvent = false;

// --- FUNKCJA DO OBSŁUGI SPRZĘTOWEGO RNG (nRF52840) ---
uint8_t getHardwareRandomByte() {
  NRF_RNG->EVENTS_VALRDY = 0;     
  NRF_RNG->TASKS_START = 1;        
  
  while (NRF_RNG->EVENTS_VALRDY == 0) {
    // Czekaj aż sprzęt wygeneruje bajt z szumu termicznego
  }
  
  uint8_t value = (uint8_t)NRF_RNG->VALUE; // Odczytujemy wartość
  NRF_RNG->TASKS_STOP = 1;                 // Zatrzymujemy generator (oszczędzanie energii)
  return value;
}

// 4 bajty, by stworzyć pełną liczbę 32-bitową do ziarna
uint32_t getHardwareSeed() {
  uint32_t seed = 0;
  seed |= ((uint32_t)getHardwareRandomByte() << 24);
  seed |= ((uint32_t)getHardwareRandomByte() << 16);
  seed |= ((uint32_t)getHardwareRandomByte() << 8);
  seed |= ((uint32_t)getHardwareRandomByte());
  return seed;
}

void initGameLogic() {
  uint32_t trueRandomSeed = getHardwareSeed();
  randomSeed(trueRandomSeed);
  random(); //odrzucenie pierwszego wyniku
}

void setDiceConfig(int valMin, int valMax, int count, int crit, int evt) {
  if (valMin > valMax) { int t = valMin; valMin = valMax; valMax = t; }
  
  minRange = max(0, valMin);
  maxRange = min(valMax, 99);
  
  diceCount = constrain(count, 1, MAX_DICE_COUNT);
  chanceCrit = constrain(crit, 0, 100);
  chanceEvent = constrain(evt, 0, 100);
}

void generateResults() {
  // 1. Losuj czy wystąpił Random Event dla całego rzutu
  randomEvent = (random(0, 100) < chanceEvent);

  for (int i = 0; i < diceCount; i++) {
    // 2. Losuj liczbę
    results[i] = random(minRange, maxRange + 1);
    
    // 3. Losuj czy ta konkretna kość ma krytyka
    criticals[i] = (random(0, 100) < chanceCrit);
  }
}

int* getResults() {
  return results;
}

bool* getCriticals() {
  return criticals;
}

bool isRandomEvent() {
  return randomEvent;
}

int getDiceCount() {
  return diceCount;
}

int getLastResult() {
  if (diceCount > 0) return results[diceCount - 1];
  return 1;
}