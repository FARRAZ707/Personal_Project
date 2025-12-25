#include <Arduino.h>

#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);
  // Gunakan 9600 jika 115200 masih menghasilkan karakter kotak-kotak
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); 
  
  delay(3000); // Tunggu modul siap
  Serial.println("\n--- Mengecek Informasi SIM Card ---");

  // 1. Cek Nama Provider (Operator)
  Serial.println("\nOperator Seluler:");
  Serial2.println("AT+COPS?"); 
  delay(500);
  while(Serial2.available()) Serial.write(Serial2.read());

  // 2. Cek Nomor Telepon Sendiri
  // Catatan: Tidak semua kartu SIM menyimpan nomor di memorinya.
  // Jika muncul ERROR atau kosong, berarti nomor tidak tersimpan di SIM.
  Serial.println("\nNomor Telepon (dari memori SIM):");
  Serial2.println("AT+CNUM"); 
  delay(500);
  while(Serial2.available()) Serial.write(Serial2.read());

  // 3. Cek IMSI (ID Unik Kartu)
  Serial.println("\nIMSI Kartu:");
  Serial2.println("AT+CIMI"); 
  delay(500);
  while(Serial2.available()) Serial.write(Serial2.read());

  Serial.println("\n--- Selesai. Masuk ke Mode Bridge ---");
}

void loop() {
  if (Serial2.available()) Serial.write(Serial2.read());
  if (Serial.available()) Serial2.write(Serial.read());
}