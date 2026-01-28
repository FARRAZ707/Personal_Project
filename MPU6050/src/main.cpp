#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Inisialisasi MPU6050
Adafruit_MPU6050 mpu;

// Konstanta untuk deteksi getaran
const float VIBRATION_THRESHOLD = 10.0;  // Threshold akselerasi (m/s²)
const int BUFFER_SIZE = 10;             // Ukuran buffer untuk smoothing
float accelX_buffer[BUFFER_SIZE];
float accelY_buffer[BUFFER_SIZE];
float accelZ_buffer[BUFFER_SIZE];
int buffer_index = 0;

float accelX, accelY, accelZ;
float magnitude;
bool vibration_detected = false;

// LED Pin untuk indikator getaran (GPIO 2 - built-in LED pada ESP32)
const int LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  delay(1000);  // Tunggu Serial siap
  
  // Inisialisasi I2C
  Wire.begin(21, 22);  // SDA = GPIO21, SCL = GPIO22 (default ESP32)
  
  // Inisialisasi MPU6050
  Serial.println("\n\nMenginisialisasi MPU6050...");
  
  if (!mpu.begin()) {
    Serial.println(" MPU6050 tidak terdeteksi!");
    Serial.println("Periksa koneksi I2C (SDA=GPIO21, SCL=GPIO22)");
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println("✅ MPU6050 berhasil terhubung");
  
  // Konfigurasi sensor
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);  // Range ±8g
  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ);    // Low Pass Filter
  
  // Inisialisasi buffer
  for (int i = 0; i < BUFFER_SIZE; i++) {
    accelX_buffer[i] = 0;
    accelY_buffer[i] = 0;
    accelZ_buffer[i] = 0;
  }
  
  // Setup LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("✅ Setup selesai! Siap mendeteksi getaran...");
  Serial.println("================================================");
  delay(500);
}

void loop() {
  // Baca sensor
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Ambil data akselerasi
  accelX = a.acceleration.x;
  accelY = a.acceleration.y;
  accelZ = a.acceleration.z;
  
  // Simpan ke buffer untuk smoothing
  accelX_buffer[buffer_index] = accelX;
  accelY_buffer[buffer_index] = accelY;
  accelZ_buffer[buffer_index] = accelZ;
  buffer_index = (buffer_index + 1) % BUFFER_SIZE;
  
  // Hitung rata-rata dari buffer (smoothing)
  float avg_accelX = 0, avg_accelY = 0, avg_accelZ = 0;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    avg_accelX += accelX_buffer[i];
    avg_accelY += accelY_buffer[i];
    avg_accelZ += accelZ_buffer[i];
  }
  avg_accelX /= BUFFER_SIZE;
  avg_accelY /= BUFFER_SIZE;
  avg_accelZ /= BUFFER_SIZE;
  
  // Hitung magnitude akselerasi (dikurangi gravitasi di sumbu Z)
  magnitude = sqrt(pow(avg_accelX, 2) + pow(avg_accelY, 2) + pow(avg_accelZ - 9.81, 2));
  
  // Deteksi getaran berdasarkan magnitude
  vibration_detected = (magnitude > VIBRATION_THRESHOLD);
  
  // Kontrol LED berdasarkan deteksi getaran
  if (vibration_detected) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  
  // Tampilkan data di Serial Monitor (setiap 200ms)
  static unsigned long last_print = 0;
  if (millis() - last_print >= 200) {
    last_print = millis();
    
    Serial.print("X: ");
    Serial.print(avg_accelX, 2);
    Serial.print(" | Y: ");
    Serial.print(avg_accelY, 2);
    Serial.print(" | Z: ");
    Serial.print(avg_accelZ, 2);
    Serial.print(" | Magnitude: ");
    Serial.print(magnitude, 2);
    Serial.print(" | Status: ");
    
    if (vibration_detected) {
      Serial.println("⚠️  GETARAN TERDETEKSI!");
    } else {
      Serial.println("✓ Normal");
    }
  }
  
  delay(10);  // Delay kecil untuk memberi waktu sensor
}