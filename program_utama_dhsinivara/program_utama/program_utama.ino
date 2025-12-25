#include "DHT.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- PINOUT (Sesuai Rangkaian Anda) ---
const int pinRelay1   = 0;
const int pinRelay2   = 1;
const int pinDHT      = 2;
const int pinPB_Mode  = 3; // Tombol 1
const int pinPB_SetR2 = 4; // Tombol 2
const int pinPB_WifiUp = 6; // Tombol 3

// --- PARAMETER ---
#define DHTTYPE DHT21       
float thresholdHumi = 65.0; 
const int jedaTransisi    = 3000; 

// --- OBJEK ---
DHT dht(pinDHT, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 20, 4); 

// --- VARIABEL STATE ---
bool modeAuto = false;
bool modulWifiAktif = false;
bool statusRelay2Manual = false; 
bool statusTransisi = false;
bool wifiTriggered = false;

bool lastPB_ModeState  = LOW; // Pull-down default LOW
bool lastPB_SetR2State = LOW;
bool lastPB_WifiUpState = LOW;

unsigned long waktuTekanWiFi = 0;
unsigned long waktuMulaiTransisi = 0;
unsigned long waktuUpdateLCD = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9); 
  
  lcd.init();
  lcd.backlight();
  
  pinMode(pinRelay1, OUTPUT);
  pinMode(pinRelay2, OUTPUT);
  digitalWrite(pinRelay1, HIGH); // OFF (Active LOW Relay)
  digitalWrite(pinRelay2, HIGH); 

  // Menggunakan INPUT biasa karena resistor pull-down sudah ada di rangkaian fisik
  pinMode(pinPB_Mode, INPUT);
  pinMode(pinPB_SetR2, INPUT);
  pinMode(pinPB_WifiUp, INPUT);

  dht.begin();
  WiFi.mode(WIFI_OFF);
  
  lcd.setCursor(0, 1);
  lcd.print("  SYSTEM  INITIAL   "); 
  lcd.setCursor(0, 2);
  lcd.print("   PULL-DOWN MODE   "); 
  delay(1500);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. TOMBOL 1: MODE (PULL-DOWN: HIGH = TEKAN) ---
  bool readPB_Mode = digitalRead(pinPB_Mode);
  if (readPB_Mode == HIGH && lastPB_ModeState == LOW) {
    modeAuto = !modeAuto; 
    if (!modeAuto) {
      statusRelay2Manual = false;
      digitalWrite(pinRelay2, HIGH);
    } else {
      statusTransisi = true;
      waktuMulaiTransisi = currentMillis;
    }
    delay(200); 
  }
  lastPB_ModeState = readPB_Mode;

  // --- 2. TOMBOL 2: SET THRES (-) / RELAY 2 ---
  bool readPB_SetR2 = digitalRead(pinPB_SetR2);
  if (readPB_SetR2 == HIGH && lastPB_SetR2State == LOW) {
    if (modeAuto) {
      if (thresholdHumi > 10.0) thresholdHumi -= 0.5;
    } else {
      statusRelay2Manual = !statusRelay2Manual; 
      digitalWrite(pinRelay2, statusRelay2Manual ? LOW : HIGH);
    }
    delay(200);
  }
  lastPB_SetR2State = readPB_SetR2;

  // --- 3. TOMBOL 3: THRES (+) / WIFI HOLD ---
  bool readPB_WifiUp = digitalRead(pinPB_WifiUp);
  
  // Deteksi Tekan
  if (readPB_WifiUp == HIGH && lastPB_WifiUpState == LOW) {
    waktuTekanWiFi = currentMillis;
    wifiTriggered = false;
  }
  
  // Logika Tahan 2 Detik (WiFi)
  if (readPB_WifiUp == HIGH && (currentMillis - waktuTekanWiFi > 2000) && !wifiTriggered) {
    modulWifiAktif = !modulWifiAktif;
    if (modulWifiAktif) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP("ESP32-System", "password123");
      ArduinoOTA.begin();
    } else {
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_OFF);
    }
    wifiTriggered = true;
    delay(200);
  }

  // Deteksi Lepas (Jika sebentar = Tambah Threshold)
  if (readPB_WifiUp == LOW && lastPB_WifiUpState == HIGH) {
    if (currentMillis - waktuTekanWiFi < 2000 && modeAuto) {
      if (thresholdHumi < 90.0) thresholdHumi += 0.5;
    }
  }
  lastPB_WifiUpState = readPB_WifiUp;

  if (modulWifiAktif) ArduinoOTA.handle();

  // --- 4. LOGIKA SENSOR & RELAY 1 ---
  float humi = dht.readHumidity();
  if (modeAuto) {
    digitalWrite(pinRelay2, HIGH); 
    if (!isnan(humi)) {
      if (humi < thresholdHumi) {
        if (statusTransisi && (currentMillis - waktuMulaiTransisi < jedaTransisi)) {
            digitalWrite(pinRelay1, HIGH);
        } else {
            digitalWrite(pinRelay1, LOW);
            statusTransisi = false;
        }
      } else {
        digitalWrite(pinRelay1, HIGH);
        statusTransisi = false;
      }
    }
  } else {
    digitalWrite(pinRelay1, HIGH); 
  }

  // --- 5. UPDATE LCD ---
  if (currentMillis - waktuUpdateLCD >= 400) {
    waktuUpdateLCD = currentMillis;
    
    lcd.setCursor(0, 0);
    if (isnan(humi)) lcd.print("  Kelembapan: ERR   ");
    else { lcd.print("  Kelembapan: "); lcd.print(humi, 1); lcd.print("%  "); }
    
    lcd.setCursor(0, 1);
    if (modeAuto) {
      lcd.print("  OTOMATIS: "); lcd.print(thresholdHumi, 1); lcd.print("%  ");
    } else {
      lcd.print("    MODE: MANUAL    ");
    }

    lcd.setCursor(0, 2);
    lcd.print("  R1:"); lcd.print(digitalRead(pinRelay1) == LOW ? "ON" : "OFF");
    lcd.print("  |  R2:"); lcd.print(digitalRead(pinRelay2) == LOW ? "ON" : "OFF");
    lcd.print("   ");

    lcd.setCursor(0, 3);
    lcd.print(modulWifiAktif ? "      WiFi: ON      " : "     WiFi: OFF      ");
  }
}