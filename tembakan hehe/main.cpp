#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DMD32.h>
#include "fonts/SystemFont5x7.h"

// PIN CONFIGURATION
#define TRIGGER_PIN 36
#define POT_PIN     39
#define RESET_PIN   0     
#define FIRESTOP    0
#define FIRESTART   1
#define FIREDURING  2

// DMD CONFIG
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN   1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
hw_timer_t* timer = NULL;

void IRAM_ATTR triggerScan() {
  dmd.scanDisplayBySPI();
}

// VARIABLES
unsigned long bulletCount = 0;
unsigned long shotCount = 0;
unsigned long rateOfFirePerMinute = 0;
unsigned long lastRateTime = 0;
unsigned long lastLCDUpdate = 0;
const unsigned long rateInterval = 1000;
const unsigned long lcdUpdateInterval = 100;

int nowFire = FIRESTOP;
bool inCalibrationMode = false;
bool buttonPressed = false;
bool longPressDetected = false;
unsigned long buttonPressStart = 0;

uint32_t sensorADC = 0;
uint32_t potADC = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// PROTOTYPES
void handleButtonLogic();
void handleShotLogic();
void doCalibration();
void updateLCD();
void updateDMD();
void resetCounters();

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.print(" SYSTEM READY ");
  
  uint8_t cpuClock = ESP.getCpuFreqMHz();
  timer = timerBegin(0, cpuClock, true);
  timerAttachInterrupt(timer, &triggerScan, true);
  timerAlarmWrite(timer, 300, true);
  timerAlarmEnable(timer);

  delay(1500);
  lcd.clear();
  dmd.clearScreen(true);
  updateDMD();
  updateLCD();
}

void loop() {
  handleButtonLogic();

  if (inCalibrationMode) {
    doCalibration();
  } else {
    handleShotLogic();
    
    // Perhitungan ROF Terjadwal (1 Detik)
    unsigned long currentTime = millis();
    if (currentTime - lastRateTime >= rateInterval) {
      rateOfFirePerMinute = shotCount * 60;
      shotCount = 0;
      lastRateTime = currentTime;
      updateDMD();
      updateLCD();
    }
  }
}

// ==========================================
// LOGIKA SENSOR (Hybrid: Analog Threshold rasa Digital)
// ==========================================
void handleShotLogic() {
  sensorADC = analogRead(TRIGGER_PIN);
  potADC = analogRead(POT_PIN);

  // Deteksi Tembakan (Threshold)
  if (sensorADC < potADC && nowFire == FIRESTOP) {
    nowFire = FIRESTART;
  } else if (sensorADC > (potADC + 200)) { // Hysteresis untuk stabilitas
    nowFire = FIRESTOP;
  }

  if (nowFire == FIRESTART) {
    shotCount++;
    bulletCount++;
    updateDMD(); // Update P10 instan
    nowFire = FIREDURING;
  }
}

// ==========================================
// LOGIKA TOMBOL (Latch Mode & Reset)
// ==========================================
void handleButtonLogic() {
  unsigned long currentTime = millis();
  bool btnState = (digitalRead(RESET_PIN) == LOW);

  if (btnState && !buttonPressed) {
    buttonPressed = true;
    buttonPressStart = currentTime;
    longPressDetected = false;
  }

  if (btnState && buttonPressed && !longPressDetected) {
    if (currentTime - buttonPressStart >= 2000) { // Tahan 2 detik
      longPressDetected = true;
      inCalibrationMode = true; 
      dmd.clearScreen(true);
      lcd.clear();
      lcd.print("MODE KALIBRASI");
    }
  }

  if (!btnState && buttonPressed) {
    if (!longPressDetected) {
      if (inCalibrationMode) {
        inCalibrationMode = false; // Lepas pengunci
        nowFire = FIRESTOP;
        lcd.clear();
        lcd.print("   READY!   ");
        delay(1000);
        dmd.clearScreen(true);
        updateDMD();
        updateLCD();
      } else {
        resetCounters();
      }
    }
    buttonPressed = false;
  }
}

// ==========================================
// MODE KALIBRASI
// ==========================================
void doCalibration() {
  static unsigned long lastCalibShow = 0;
  if (millis() - lastCalibShow > 250) {
    sensorADC = analogRead(TRIGGER_PIN);
    potADC = analogRead(POT_PIN);

    dmd.clearScreen(true);
    dmd.selectFont(System5x7);
    String s = "S:" + String(sensorADC);
    String p = "P:" + String(potADC);
    dmd.drawString(0, 0, s.c_str(), s.length(), GRAPHICS_NORMAL);
    dmd.drawString(0, 8, p.c_str(), p.length(), GRAPHICS_NORMAL);

    lcd.setCursor(0, 1);
    lcd.print("S:"); lcd.print(sensorADC);
    lcd.print(" P:"); lcd.print(potADC);
    lcd.print("    ");
    lastCalibShow = millis();
  }
}

void resetCounters() {
  bulletCount = 0;
  shotCount = 0;
  rateOfFirePerMinute = 0;
  lcd.clear();
  lcd.print(" RESET DONE! ");
  delay(1000);
  lcd.clear();
  updateDMD();
  updateLCD();
}

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("ROF: "); lcd.print(rateOfFirePerMinute); lcd.print("     ");
  lcd.setCursor(0, 1);
  lcd.print("BLT: "); lcd.print(bulletCount); lcd.print("     ");
}

void updateDMD() {
  dmd.clearScreen(true);
  dmd.selectFont(System5x7);
  String r = String(rateOfFirePerMinute);
  String b = String(bulletCount);
  dmd.drawString((32 - (r.length() * 6)) / 2, 0, r.c_str(), r.length(), GRAPHICS_NORMAL);
  dmd.drawString((32 - (b.length() * 6)) / 2, 8, b.c_str(), b.length(), GRAPHICS_NORMAL);
}