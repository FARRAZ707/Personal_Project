#define TINY_GSM_MODEM_SIM7600  // Definisikan tipe modem sebelum include library
#define TINY_GSM_RX_BUFFER 1024 // Tingkatkan buffer untuk kestabilan

#include <TinyGsmClient.h>
#include <PubSubClient.h>

// Pin UART2 ESP32
#define RXD2 16
#define TXD2 17

// Konfigurasi APN (Sesuaikan dengan provider)
const char apn[]      = "internet"; 
const char gprsUser[] = "";
const char gprsPass[] = "";

// Konfigurasi MQTT Broker
const char* mqtt_broker = "broker.hivemq.com";
const char* mqtt_topic  = "esp32/sim7600/data";
const int   mqtt_port   = 1883;

// Objek Jaringan
TinyGsm modem(Serial2);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi Serial ke SIM7600
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(3000);

  Serial.println("Menginisialisasi modem...");
  if (!modem.restart()) {
    Serial.println("Gagal restart modem, mencoba wakeup...");
    modem.init();
  }

  // Menghubungkan ke Jaringan GPRS
  Serial.print("Menghubungkan ke APN: ");
  Serial.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" fail");
    ESP.restart(); // Restart jika gagal konek internet
  }
  Serial.println(" success");

  // Setting MQTT
  mqtt.setServer(mqtt_broker, mqtt_port);
}

void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Mencoba koneksi MQTT...");
    // Membuat Client ID unik
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();

  // Kirim data setiap 10 detik
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 10000) {
    lastMsg = millis();
    
    int val = random(0, 100);
    String payload = "{\"temp\":" + String(val) + "}";
    
    Serial.print("Publish message: ");
    Serial.println(payload);
    mqtt.publish(mqtt_topic, payload.c_str());
  }
}