#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DMD32.h>
#include "fonts/SystemFont5x7.h"
#include "fonts/Arial_black_16.h"

#define TRIGGER_PIN 36
#define RESET_PIN   0     
#define ADC_PIN     39  // Pin ADC untuk potensiometer
#define FIRESTOP    0
#define FIRESTART   1
#define FIREDURING  2

// PROTOTYPE FUNGSI
void resetCounters();
void updateLCD();
void updateDMD();

// DMD / P10 display configuration (1 panel 32x16)
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN   1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

hw_timer_t* timer = NULL;

void IRAM_ATTR triggerScan() {
  dmd.scanDisplayBySPI();
}

unsigned long shotCount = 0;
unsigned long bulletCount = 0;
unsigned long rateOfFire = 0;
unsigned long highestRoF = 0;
unsigned long highestRoFm = 0;
unsigned long lastRateTime = 0;
unsigned long lastLCDUpdate = 0;
unsigned long rateOfFirePerMinute = 0;
const unsigned long rateInterval = 1000;
const unsigned long lcdUpdateInterval = 100;
int  nowFire = FIRESTOP;
bool lastResetState = HIGH;
bool needLCDUpdate = false;
int adcValue = 0;  // Nilai ADC dari potensiometer
int lastAdcValue = -1;  // Nilai ADC sebelumnya untuk deteksi perubahan

// LCD I2C 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(RESET_PIN,  INPUT_PULLUP);
  pinMode(ADC_PIN, INPUT);  // Inisialisasi pin ADC

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("READY TO");
  lcd.setCursor(5, 1);
  lcd.print("SHOOT ");
  delay(2000);
  lcd.clear();

  Serial.println("Shot Counter Ready! Reset dengan GPIO0");
  lastRateTime = millis();
  lastLCDUpdate = millis();

  // Init timer untuk DMD / P10
  delay(200);
  uint8_t cpuClock = ESP.getCpuFreqMHz();
  timer = timerBegin(0, cpuClock, true);
  timerAttachInterrupt(timer, &triggerScan, true);
  timerAlarmWrite(timer, 300, true);
  timerAlarmEnable(timer);

  // Splash "START" di P10
  dmd.selectFont(System5x7);
  {
    String startTxt = "START";
    char startBuf[6];
    startTxt.toCharArray(startBuf, sizeof(startBuf));

    int textPixelWidth = 0;
    for (size_t i = 0; i < startTxt.length(); i++) {
      textPixelWidth += dmd.charWidth((unsigned char)startBuf[i]) + 1;
    }
    int totalWidth  = DMD_PIXELS_ACROSS * DISPLAYS_ACROSS;
    int totalHeight = DMD_PIXELS_DOWN   * DISPLAYS_DOWN;

    int x = (totalWidth - textPixelWidth) / 2;
    if (x < 0) x = 0;

    int fontHeight = 8;
    int y = (totalHeight - fontHeight) / 2;
    if (y < 0) y = 0;

    dmd.clearScreen(true);
    dmd.drawString(x, y, startBuf, startTxt.length(), GRAPHICS_NORMAL);
    delay(2000);
    dmd.clearScreen(true);
  }

  updateDMD();
  updateLCD();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Cek tombol reset
  bool resetState = digitalRead(RESET_PIN);
  if (resetState == LOW && lastResetState == HIGH) {
    resetCounters();
    delay(50);
  }
  lastResetState = resetState;

  // Baca nilai ADC dari potensiometer
  adcValue = analogRead(ADC_PIN);
  if (adcValue != lastAdcValue) {
    lastAdcValue = adcValue;
    //needLCDUpdate = true;
  }

  // Update LCD secara terjadwal (throttled) jika ada perubahan
  if (needLCDUpdate && (currentTime - lastLCDUpdate >= lcdUpdateInterval)) {
    updateLCD();
    lastLCDUpdate = currentTime;
    needLCDUpdate = false;
  }

  if (digitalRead(TRIGGER_PIN) == HIGH && nowFire == FIRESTOP) {
    nowFire = FIRESTART;
  } else if (digitalRead(TRIGGER_PIN) == LOW) {
    nowFire = FIRESTOP;
    delay(7);
  }

  if (nowFire == FIRESTART) {
    shotCount++;
    bulletCount++;

    // Update P10 setiap peluru (cepat, tidak masalah)
    updateDMD();
    
    // Tandai bahwa LCD perlu update (akan diupdate di interval yang aman)
    needLCDUpdate = true;

    if (currentTime - lastRateTime >= rateInterval) {
      rateOfFire = shotCount;
      shotCount = 0;
      lastRateTime = currentTime;

      if (rateOfFire > highestRoF) highestRoF = rateOfFire;
      rateOfFirePerMinute = rateOfFire * 60;
      if (rateOfFirePerMinute > highestRoFm) highestRoFm = rateOfFirePerMinute;

      // Update display untuk nilai RoF dan RPM baru
      updateDMD();
      updateLCD();
      lastLCDUpdate = currentTime;
      needLCDUpdate = false;

      Serial.print("Peluru: "); Serial.print(bulletCount);
      Serial.print(" | RoF: "); Serial.print(rateOfFire);
      Serial.print(" | RPM: "); Serial.print(rateOfFirePerMinute);
      Serial.print(" | Max: "); Serial.println(highestRoFm);
      Serial.print(" | ADC: "); Serial.println(adcValue);
    }
    nowFire = FIREDURING;
  }
}

void resetCounters() {
  shotCount = 0;
  bulletCount = 0;
  rateOfFire = 0;
  highestRoF = 0;
  highestRoFm = 0;
  rateOfFirePerMinute = 0;
  lastRateTime = millis();
  needLCDUpdate = false;

  // LCD feedback
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("RESET!");

  // DMD: tulis "RESET" kecil di tengah
  dmd.selectFont(System5x7);
  String txt = "RESET";
  char txtBuf[txt.length() + 1];
  txt.toCharArray(txtBuf, txt.length() + 1);

  int textPixelWidth = 0;
  for (size_t i = 0; i < txt.length(); i++) {
    textPixelWidth += dmd.charWidth((unsigned char)txtBuf[i]) + 1;
  }
  int totalWidth  = DMD_PIXELS_ACROSS * DISPLAYS_ACROSS;
  int totalHeight = DMD_PIXELS_DOWN   * DISPLAYS_DOWN;

  int x = (totalWidth - textPixelWidth) / 2;
  if (x < 0) x = 0;

  int fontHeight = 8;
  int y = (totalHeight - fontHeight) / 2;
  if (y < 0) y = 0;

  dmd.clearScreen(true);
  dmd.drawString(x, y, txtBuf, txt.length(), GRAPHICS_NORMAL);

  delay(1000);
  
  dmd.clearScreen(true);
  lcd.clear();

  Serial.println("=== COUNTERS RESET ===");

  updateLCD();
  updateDMD();
  lastLCDUpdate = millis();
}


void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("                "); 
  lcd.setCursor(0, 0);
  lcd.print("ROF:");
  lcd.print(rateOfFirePerMinute);


  lcd.setCursor(0, 1);
  lcd.print("                ");  
  lcd.setCursor(0, 1);
  lcd.print("BLT: ");
  lcd.print(bulletCount);
  // lcd.print("ADC:");
  // lcd.print(adcValue);
}

void updateDMD() {
  dmd.clearScreen(true);
  dmd.selectFont(System5x7);

  // Baris atas: RPM
  String s1 = String(rateOfFirePerMinute);
  char b1[s1.length() + 1];
  s1.toCharArray(b1, s1.length() + 1);

  // Baris bawah: BulletCount
  String s2 = String(bulletCount);
  char b2[s2.length() + 1];
  s2.toCharArray(b2, s2.length() + 1);

  int totalWidth = DMD_PIXELS_ACROSS * DISPLAYS_ACROSS;

  // Hitung lebar RPM
  int w1 = 0;
  for (size_t i = 0; i < s1.length(); i++) {
    w1 += dmd.charWidth((unsigned char)b1[i]) + 1;
  }
  int x1 = (totalWidth - w1) / 2;
  if (x1 < 0) x1 = 0;

  // Hitung lebar BulletCount
  int w2 = 0;
  for (size_t i = 0; i < s2.length(); i++) {
    w2 += dmd.charWidth((unsigned char)b2[i]) + 1;
  }
  int x2 = (totalWidth - w2) / 2;
  if (x2 < 0) x2 = 0;

  dmd.drawString(x1, 0, b1, s1.length(), GRAPHICS_NORMAL);  // RPM
  dmd.drawString(x2, 8, b2, s2.length(), GRAPHICS_NORMAL);  // Bullet
}
