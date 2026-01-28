#include <BleKeyboard.h>

// Nama perangkat yang akan muncul di menu Bluetooth Laptop
BleKeyboard bleKeyboard("ESP32_Test_ADC", "Gemini-AI", 100);

void setup() {
  Serial.begin(115200);
  Serial.println("Memulai perangkat BLE...");
  bleKeyboard.begin();
}

void loop() {
  // Mengecek apakah ESP32 sudah terhubung (paired) ke Laptop
  if(bleKeyboard.isConnected()) {
    
    // Generate data random antara 0 sampai 4095 (mensimulasikan ADC 12-bit)
    int fakeADC = random(0, 4096);
    
    Serial.print("Mengirim data ke Laptop: ");
    Serial.println(fakeADC);

    // Mengetikkan angka ke Laptop
    bleKeyboard.print("Data ADC: ");
    bleKeyboard.print(fakeADC);
    
    // Menekan tombol Enter agar data berikutnya ada di baris baru
    bleKeyboard.write(KEY_RETURN); 
    
    // Jeda 2 detik antar pengiriman agar tidak terlalu cepat
    delay(2000); 
    
  } else {
    // Jika belum terhubung, munculkan pesan di Serial Monitor setiap 5 detik
    Serial.println("Menunggu koneksi Bluetooth dari laptop...");
    delay(5000);
  }
}