#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DMD32.h>
#include "fonts/SystemFont5x7.h"

// KONFIGURASI PIN
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

// VARIABEL GLOBAL
unsigned long bulletCount = 0;
unsigned long shotCount = 0;
unsigned long rateOfFirePerMinute = 0;
unsigned long lastRateTime = 0;
unsigned long lastLCDUpdate = 0;

int nowFire = FIRESTOP;
bool inCalibrationMode = false; // Flag pengunci (Latch)
uint32_t sensorADC = 0;
uint32_t potADC = 0;

unsigned long buttonPressStart = 0;
bool buttonPressed = false;
bool longPressDetected = false;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==========================================
// FUNCTION PROTOTYPES (Agar tidak error)
// ==========================================
void doShotDetection();
void doCalibration();
void updateDMD();
void updateLCD();
void showOK();
void resetCounters();

// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  pinMode(RESET_PIN, INPUT_PULLUP);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("  SYSTEM READY  ");

  // Init DMD timer
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

// ==========================================
// MAIN LOOP
// ==========================================
void loop() {
  unsigned long currentTime = millis();
  bool btnNow = (digitalRead(RESET_PIN) == LOW);

  // --- LOGIKA TOMBOL DENGAN PENGUNCI (LATCH) ---
  if (btnNow && !buttonPressed) {
    buttonPressed = true;
    buttonPressStart = currentTime;
    longPressDetected = false;
  }

  if (btnNow && buttonPressed && !longPressDetected) {
    if (currentTime - buttonPressStart >= 2000) { // Tahan 2 detik untuk kunci kalibrasi
      longPressDetected = true;
      inCalibrationMode = true; 
      dmd.clearScreen(true);
      lcd.clear();
      lcd.print("MODE: KALIBRASI");
    }
  }

  if (!btnNow && buttonPressed) {
    if (!longPressDetected) {
      // Jika dilepas cepat (Short Press)
      if (inCalibrationMode) {
        inCalibrationMode = false; // Lepas pengunci kalibrasi
        nowFire = FIRESTOP;
        showOK();
      } else {
        resetCounters();
      }
    }
    buttonPressed = false;
  }

  // --- EKSEKUSI MODE ---
  if (inCalibrationMode) {
    doCalibration();
  } else {
    doShotDetection();
    
    // Hitung RoF tiap 1 detik
    if (currentTime - lastRateTime >= 1000) {
      rateOfFirePerMinute = shotCount * 60;
      shotCount = 0;
      lastRateTime = currentTime;
      updateDMD();
      updateLCD();
    }
  }

  // Throttling LCD agar tidak membebani prosesor
  if (currentTime - lastLCDUpdate >= 250) {
    updateLCD();
    lastLCDUpdate = currentTime;
  }
}

// ==========================================
// DEFINISI FUNGSI-FUNGSI
// ==========================================

void doShotDetection() {
  // Stabilisasi ADC: Rata-rata 8 sampel untuk mengurangi noise
  long sum = 0;
  for(int i=0; i<8; i++) sum += analogRead(TRIGGER_PIN);
  sensorADC = sum / 8;
  
  potADC = analogRead(POT_PIN);

  // Logika deteksi peluru (Trigger jika sensor < pot)
  if (sensorADC < potADC && nowFire == FIRESTOP) {
    bulletCount++;
    shotCount++;
    nowFire = FIRESTART;
    updateDMD(); 
  } 
  // Hysteresis: Sensor harus kembali ke kondisi terang (pot + offset)
  else if (sensorADC > (potADC + 250)) { 
    nowFire = FIRESTOP;
  }
}

void doCalibration() {
  static unsigned long lastCalibRefresh = 0;
  if (millis() - lastCalibRefresh > 200) { // Refresh rate kalibrasi 200ms
    sensorADC = analogRead(TRIGGER_PIN);
    potADC = analogRead(POT_PIN);

    dmd.clearScreen(true);
    dmd.selectFont(System5x7);
    String s = String(sensorADC);
    String p = String(potADC);
    dmd.drawString(0, 0, s.c_str(), s.length(), GRAPHICS_NORMAL);
    dmd.drawString(0, 8, p.c_str(), p.length(), GRAPHICS_NORMAL);
    
    lcd.setCursor(0,1);
    lcd.print("S:"); lcd.print(sensorADC); lcd.print(" P:"); lcd.print(potADC); lcd.print("    ");
    lastCalibRefresh = millis();
  }
}

void showOK() {
  dmd.clearScreen(true);
  dmd.selectFont(System5x7);
  dmd.drawString(10, 4, "OK", 2, GRAPHICS_NORMAL);
  lcd.clear();
  lcd.print("  SETTING SAVED ");
  delay(1000);
  dmd.clearScreen(true);
  updateDMD();
  updateLCD();
}

void resetCounters() {
  bulletCount = 0;
  shotCount = 0;
  rateOfFirePerMinute = 0;
  lcd.clear();
  lcd.print("   RESETTING...  ");
  delay(1000);
  lcd.clear();
  updateDMD();
  updateLCD();
}

void updateLCD() {
  if (inCalibrationMode) return;
  lcd.setCursor(0, 0);
  lcd.print("RPM: "); lcd.print(rateOfFirePerMinute); lcd.print("    ");
  lcd.setCursor(0, 1);
  lcd.print("BLT: "); lcd.print(bulletCount); lcd.print("    ");
}

void updateDMD() {
  if (inCalibrationMode) return;
  dmd.clearScreen(true);
  dmd.selectFont(System5x7);
  
  String r = String(rateOfFirePerMinute);
  String b = String(bulletCount);
  
  // Hitung posisi X agar teks di tengah (Center Alignment)
  int x1 = (32 - (r.length() * 6)) / 2;
  int x2 = (32 - (b.length() * 6)) / 2;
  
  dmd.drawString(x1 < 0 ? 0 : x1, 0, r.c_str(), r.length(), GRAPHICS_NORMAL);
  dmd.drawString(x2 < 0 ? 0 : x2, 8, b.c_str(), b.length(), GRAPHICS_NORMAL);
}