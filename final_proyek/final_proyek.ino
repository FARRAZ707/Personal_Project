#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 

const char* ssid = "NDtechnogy 4G";        
const char* password = "NDtech1234";

const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

const char* topic_data = "proyek/solar_panel/data";    
const char* topic_control = "proyek/solar_panel/control"; 

WiFiClient espClient;
PubSubClient client(espClient);

bool fanStatus = false;
bool chargeStatus = true;
float batteryLevel = 85.0; 
#define PIN_FAN 2 

void setup() {
  Serial.begin(115200);
  pinMode(PIN_FAN, OUTPUT);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Pesan Kontrol Masuk: ");
  Serial.println(message);

  if (message == "FAN_ON") {
    fanStatus = true;
    digitalWrite(PIN_FAN, HIGH);
  } else if (message == "FAN_OFF") {
    fanStatus = false;
    digitalWrite(PIN_FAN, LOW);
  } else if (message == "CHARGE_ON") {
    chargeStatus = true;
  } else if (message == "CHARGE_OFF") {
    chargeStatus = false;
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Terhubung!");
      client.subscribe(topic_control);
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

float randomFloat(float min, float max) {
  return min + (float)random(0, 10000) / 10000.0 * (max - min);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float voltage = randomFloat(11.5, 12.2);
  float current = randomFloat(2.0, 5.0);
  float tempPanel = randomFloat(45.0, 55.0);
  float tempBox = randomFloat(30.0, 40.0);

  if(chargeStatus && batteryLevel < 100) batteryLevel += 0.5;
  else if(!chargeStatus && batteryLevel > 0) batteryLevel -= 0.5;

  StaticJsonDocument<512> doc;
  doc["voltage"] = voltage;
  doc["current"] = current;
  doc["tempPanel"] = tempPanel;
  doc["tempBox"] = tempBox;
  doc["fanStatus"] = fanStatus ? "ON" : "OFF";
  doc["chargeStatus"] = chargeStatus ? "Charging" : "Idle";
  doc["batteryLevel"] = batteryLevel;

  char buffer[512];
  serializeJson(doc, buffer);

  if (client.publish(topic_data, buffer)) {
    Serial.println("--------------------------------------------------");
    Serial.println("[SUKSES] Data berhasil dikirim ke MQTT!");
    Serial.print("Isi Data: ");
    Serial.println(buffer);
    Serial.println("--------------------------------------------------");
  } else {
    Serial.println("[ERROR] Gagal mengirim data. Cek koneksi internet.");
  }

  delay(2000); 
}