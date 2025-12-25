// 1. Definisikan tipe modem sebelum include TinyGSM
#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 1024

#include <TinyGsmClient.h>
#include <PubSubClient.h>

// 2. Konfigurasi Pin UART2 ESP32
#define RXD2 16
#define TXD2 17

// 3. Konfigurasi GPRS (Sesuaikan APN dengan Provider Anda)
const char apn[]      = "internet"; // Contoh: "internet", "xlunlimited", atau "indosatgprs"
const char gprsUser[] = "";
const char gprsPass[] = "";

// 4. Konfigurasi MQTT Broker
const char* mqtt_broker = "broker.hivemq.com";
//const char* mqtt_broker = "test.mosquitto.org";
const char* mqtt_topic  = "esp32/sim800l/data";
const int   mqtt_port   = 1883;

// Objek Modem dan MQTT
TinyGsm modem(Serial2);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

unsigned long lastMsg = 0;

void setup() {
  Serial.begin(115200);
  delay(10);

  // Serial2 untuk SIM800L (Pastikan baudrate sesuai, biasanya 9600 atau 115200)
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  
  Serial.println("\n--- Memulai Inisialisasi SIM800L ---");
  delay(3000); 

  // Inisialisasi Modem
  Serial.print("Menghubungi Modem...");
  if (!modem.restart()) {
    Serial.println(" Gagal! Cek Power/Kabel.");
    return;
  }
  Serial.println(" OK.");

  // Menunggu Jaringan Seluler
  Serial.print("Menunggu Jaringan...");
  if (!modem.waitForNetwork()) {
    Serial.println(" Gagal.");
    return;
  }
  Serial.println(" Terhubung.");

  // Menghubungkan ke GPRS
  Serial.print("Menghubungkan ke APN: ");
  Serial.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" Gagal.");
    return;
  }
  Serial.println(" Sukses.");

  // Konfigurasi MQTT
  mqtt.setServer(mqtt_broker, mqtt_port);
  Serial.println("Siap mengirim data MQTT.");
}

void reconnect() {
  // Loop sampai terhubung kembali ke MQTT Broker
  while (!mqtt.connected()) {
    Serial.print("Mencoba koneksi MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("Connected!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" Coba lagi dalam 5 detik...");
      delay(1000);
    }
  }
}

void loop() {
  // Pastikan koneksi MQTT tetap aktif
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();

  // Mengirim data random setiap 10 detik
  unsigned long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;

    // Generate Data Random
    int randomTemp = random(20, 40); // Simulasi suhu
    int randomHum  = random(50, 90); // Simulasi kelembapan

    // Membuat format JSON
    String payload = "{\"temp\":" + String(randomTemp) + ",\"humi\":" + String(randomHum) + "}";
    
    Serial.print("Publishing: ");
    Serial.println(payload);

    // Kirim ke MQTT Broker
    if (mqtt.publish(mqtt_topic, payload.c_str())) {
      Serial.println("Pesan terkirim!");
    } else {
      Serial.println("Gagal mengirim pesan.");
    }
  }
}