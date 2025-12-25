//===================================================================
// PROGRAM: P10 LED DISPLAY TEST MODE (TEKS STATIS)
// FUNGSI: Hanya menampilkan teks diam di tengah layar.
//===================================================================

//----------------------------------------INCLUDE LIBRARIES
#include <DMD32.h>
#include "fonts/SystemFont5x7.h" 
//----------------------------------------

//----------------------------------------KONFIGURASI DISPLAY DAN DMD
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN); 

// Timer untuk Refresh Display
hw_timer_t * timer = NULL;
void IRAM_ATTR triggerScan() {
  dmd.scanDisplayBySPI();
}
//----------------------------------------

// Teks yang akan ditampilkan (Statis)
String staticText = "ESP32";
char charArray[30]; 

//________________________________________________________________________________VOID SETUP()
void setup(void){
  Serial.begin(115200);
  delay(1000);
  Serial.println("P10 Display Static Mode Active.");

  // 1. SETUP TIMER DMD
  // Menggunakan 3 argumen standar (timerBegin(0, 80, true))
  timer = timerBegin(0, 80, true); 
  timerAttachInterrupt(timer, &triggerScan, true); 
  timerAlarmWrite(timer, 300, true); 
  timerAlarmEnable(timer);

  // 2. SETUP DISPLAY
  dmd.clearScreen(true);
  dmd.selectFont(SystemFont5x7); 
  
  // Konversi String ke Char Array (Hanya dilakukan sekali)
  staticText.toCharArray(charArray, sizeof(charArray));
  
  Serial.println("Display ready. Showing static text.");
}
//________________________________________________________________________________

//________________________________________________________________________________VOID LOOP()
void loop(void){
  
  // Hitung posisi agar teks berada di tengah display (Centered)
  
  // Lebar Teks (jumlah karakter * lebar piksel font)
  int textWidth = staticText.length() * 6; 
  int displayWidth = 32 * DISPLAYS_ACROSS; // 32 piksel

  // Posisi X di tengah
  int x_pos = (displayWidth - textWidth) / 2; 
  // Posisi Y di tengah (tinggi display 16, tinggi font 7)
  int y_pos = (16 - 7) / 2; 
  
  // 1. Bersihkan layar
  dmd.clearScreen(true);
  
  // 2. Tampilkan Teks Statis di posisi yang dihitung
  dmd.drawString(x_pos, y_pos, charArray, staticText.length(), GRAPHICS_NORMAL);
  
  // Jeda panjang (misalnya 100 milidetik) agar loop tidak terlalu cepat, 
  // meskipun teks sudah statis. Display tetap di-refresh oleh Timer Interrupt.
  delay(100); 
}
//________________________________________________________________________________