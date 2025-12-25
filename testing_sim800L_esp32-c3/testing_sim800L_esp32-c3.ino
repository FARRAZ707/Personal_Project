#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

// Definisi Pin sesuai permintaan
const int RX_PIN = 5;
const int TX_PIN = 7;

// Gunakan HardwareSerial ke-1 (Serial1)
// ESP32-C3 memiliki 2 UART: Serial0 (USB) dan Serial1
HardwareSerial SerialGsm(1); 
TinyGsm modem(SerialGsm);

void setup() {
  // Serial Monitor melalui USB
  Serial.begin(115200);
  delay(1000);

  // Inisialisasi Serial1 pada pin 5 dan 7
  // Parameter: baudrate, config, rxPin, txPin
  SerialGsm.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  
  delay(2000);

  Serial.println("--- Diagnosa Koneksi Data (ESP32-C3) ---");

  Serial.print("Inisialisasi modem...");
  if (!modem.restart()) {
    Serial.println(" Gagal! Periksa kabel TX/RX dan Power.");
    return;
  }
  Serial.println(" Berhasil.");

  Serial.print("Menunggu jaringan...");
  if (!modem.waitForNetwork()) {
    Serial.println(" Gagal (Cek Antena/Sinyal)");
    return;
  }
  Serial.println(" Terhubung.");

  Serial.print("Menghubungkan ke GPRS...");
  if (!modem.gprsConnect("internet", "", "")) {
    Serial.println(" Gagal!");
  } else {
    Serial.println(" Sukses!");
    Serial.print("IP Lokal: ");
    Serial.println(modem.localIP());
  }
}

void loop() {
  // Kosong
}