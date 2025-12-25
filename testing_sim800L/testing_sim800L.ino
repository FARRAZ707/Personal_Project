#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

#define RXD2 16
#define TXD2 17

TinyGsm modem(Serial2);

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(3000);

  Serial.println("--- Diagnosa Koneksi Data ---");

  Serial.print("Menunggu jaringan...");
  if (!modem.waitForNetwork()) {
    Serial.println(" Gagal (Cek Antena/Sinyal)");
    return;
  }
  Serial.println(" Terhubung ke Network.");

  Serial.print("Menghubungkan ke GPRS (internet)...");
  if (!modem.gprsConnect("internet", "", "")) {
    Serial.println(" Gagal (Cek APN/Kuota/IMEI)");
  } else {
    Serial.println(" Sukses!");
    Serial.print("IP Lokal: ");
    Serial.println(modem.localIP());
  }
}

void loop() {
  // Biarkan kosong untuk diagnosa
}