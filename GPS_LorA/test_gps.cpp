#include <Arduino.h> // Tambahkan ini di PlatformIO
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <ESPAsyncWebServer.h>
// ==== 1. Deklarasi Fungsi (Function Prototype) ====
// Ini memberitahu kompiler bahwa fungsi displayInfo akan ada di bawah
void displayInfo(); 

// Definisi Pin
const int RXD2 = 16; 
const int TXD2 = 17; 

TinyGPSPlus gps;
HardwareSerial gpsSerial(2); 

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("--- GPS NEO-M8N Test (PlatformIO) ---");
}

void loop() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      displayInfo(); // Sekarang kompiler sudah mengenali fungsi ini
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("Error: Modul GPS tidak terdeteksi!");
    delay(5000);
  }
}

// ==== 2. Definisi Fungsi ====
void displayInfo() {
  Serial.print("Lokasi: "); 
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6);
    Serial.print(", ");
    Serial.print(gps.location.lng(), 6);
    Serial.print(" | Satelit: ");
    Serial.println(gps.satellites.value());
  } else {
    Serial.print("Mencari Sinyal... Satelit: ");
    Serial.println(gps.satellites.value());
  }
}