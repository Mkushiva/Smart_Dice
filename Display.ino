#include <U8g2lib.h>
#include <Wire.h>
#include "Config.h"

// ZMIANA 1: U8G2_R1 obraca ekran o 90 stopni (Pionowo)
// Jeśli obraz będzie "do góry nogami", zmień R1 na R3
U8G2_SSD1306_64X32_1F_F_HW_I2C u8g2(U8G2_R1, /* reset=*/ U8X8_PIN_NONE);

static int scrollPixelOffset = 0;
static int currentRollNum = 1;
static int nextRollNum = 2;
extern int minRange;
extern int maxRange;

const int SCREEN_WIDTH = 32;
const int SCREEN_HEIGHT = 64;
const int Y_BASELINE = 44; 

void initDisplay() {
  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  currentRollNum = random(minRange, maxRange + 1);
  nextRollNum = random(minRange, maxRange + 1);
}

void turnOffDisplay() {
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

bool isDisplayConnected() {
  Wire.beginTransmission(0x3C);
  return (Wire.endTransmission() == 0);
}

// --- WARSTWA 1: Rysowanie samej liczby (przesuwanej) ---
void drawDiceNumber(int number, int xOffset) {
  char buf[8]; sprintf(buf, "%d", number);
  
  // Dobór czcionki
  if (number < 100) u8g2.setFont(u8g2_font_logisoso24_tn); 
  else u8g2.setFont(u8g2_font_luRS14_tr);

  // Centrowanie z uwzględnieniem offsetu animacji
  int width = u8g2.getStrWidth(buf);
  int x = xOffset + (SCREEN_WIDTH - width) / 2;
  int y = Y_BASELINE;
  if (number >= 100) y = 38;

  u8g2.drawStr(x, y, buf);
}

// --- WARSTWA 2: Elementy statyczne (Wykrzykniki) ---
// Ta funkcja NIE przyjmuje offsetu, zawsze rysuje na środku
void drawStaticOverlays(bool isEvent) {
  if (isEvent) {
    u8g2.setFont(u8g2_font_ncenB14_tr); 
    int exW = u8g2.getStrWidth("!");
    int exX = (SCREEN_WIDTH - exW) / 2; // Zawsze środek ekranu

    // Górny wykrzyknik
    u8g2.drawStr(exX, 15, "!"); 
    // Dolny wykrzyknik
    u8g2.drawStr(exX, 63, "!"); 
  }
}

// --- WYNIK KOŃCOWY (Statyczny) ---
void drawResult(int number, bool isCrit, bool isEvent) {
  u8g2.clearBuffer();
  
  // 1. Rysujemy liczbę na miejscu (offset 0)
  drawDiceNumber(number, 0);

  // 2. Rysujemy nakładkę (Event)
  drawStaticOverlays(isEvent);

  // 3. Efekt Krytyka (Inwersja) - nakładamy na całość
  if (isCrit) {
    u8g2.setDrawColor(2); // XOR mode
    u8g2.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    u8g2.setDrawColor(1);
  }
  
  u8g2.sendBuffer();
  
  // Reset stanu animacji
  scrollPixelOffset = 0;
  currentRollNum = number; 
  nextRollNum = random(minRange, maxRange + 1);
}

// --- ANIMACJA SWIPE ---
// Teraz przyjmuje tylko jedną flagę isEvent, bo dotyczy ona obu liczb
void drawSwipeFrame(int oldNum, int newNum, bool isEvent, int offset) {
  u8g2.clearBuffer();
  
  // 1. Stara liczba ucieka w lewo
  drawDiceNumber(oldNum, -offset);
  
  // 2. Nowa liczba wjeżdża z prawej
  drawDiceNumber(newNum, 32 - offset);
  
  // 3. Wykrzykniki stoją w miejscu (rysowane na wierzchu)
  drawStaticOverlays(isEvent);
  
  u8g2.sendBuffer();
}

// --- Stara funkcja Slot Machine (dla kompatybilności) ---
void drawNumAtY(int num, int y) {
  char buf[4]; sprintf(buf, "%d", num);
  int width = u8g2.getStrWidth(buf);
  u8g2.drawStr((SCREEN_WIDTH - width) / 2, y, buf);
}

void animSlotMachine() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso24_tn); 
  int scrollSpeed = 8; 
  drawNumAtY(currentRollNum, Y_BASELINE + scrollPixelOffset);
  drawNumAtY(nextRollNum, Y_BASELINE + scrollPixelOffset - SCREEN_HEIGHT);
  u8g2.sendBuffer();
  scrollPixelOffset += scrollSpeed;
  if (scrollPixelOffset >= SCREEN_HEIGHT) {
    scrollPixelOffset = 0;      
    currentRollNum = nextRollNum; 
    nextRollNum = random(minRange, maxRange + 1);   
  }
}

void showRollingFrame() {
  animSlotMachine();
}