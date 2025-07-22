
#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- TAMBAHAN UNTUK OLED ---
#define SCREEN_WIDTH 128 // Lebar layar OLED dalam piksel
#define SCREEN_HEIGHT 64 // Tinggi layar OLED dalam piksel
#define OLED_RESET -1    // Pin reset (atau -1 jika berbagi pin reset Arduino)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// -------------------------

// --- Konfigurasi WiFi & ThingSpeak ---
const char* ssid = "Wokwi-GUEST"; // Biarkan seperti ini untuk simulasi Wokwi 
const char* password = ""; // Biarkan kosong untuk simulasi Wokwi  
const char* thingSpeakAddress = "https://api.thingspeak.com/update"; 
String apiKey = "32PN290V7YWC7GTO"; // <-- GANTI DENGAN WRITE API KEY ANDA 

// --- Pengaturan Sensor DHT ---
#define DHTPIN 17     // Pin yang terhubung ke DHT22 
#define DHTTYPE DHT22 // Tipe sensor DHT22 
DHT dht(DHTPIN, DHTTYPE);

// Variabel untuk menyimpan data sensor
float hum = 0;   // Variabel untuk kelembaban 
float temp = 0;  // Variabel untuk suhu 
float hic = 0;   // Variabel untuk heat index 

// Variabel untuk timer
unsigned long lastTime = 0; 
unsigned long timerDelay = 15000; // Kirim data setiap 15 detik 

void setup() {
  Serial.begin(9600); 

  // --- TAMBAHAN: Inisialisasi OLED ---
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Alokasi SSD1306 gagal"));
    for(;;); // Jangan lanjutkan, looping selamanya
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Mencari data...");
  display.display();
  // ---------------------------------
  
  // Inisialisasi sensor DHT 
  dht.begin(); 
  
  // Koneksi ke WiFi
  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTerhubung ke WiFi!");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP()); 
}

void loop() {
  // Hanya eksekusi setiap 'timerDelay' milidetik
  if ((millis() - lastTime) > timerDelay) {
    // Membaca data suhu dan kelembaban
    hum = dht.readHumidity();
    temp = dht.readTemperature(); // Membaca suhu dalam Celsius 

    // Cek jika pembacaan gagal
    if (isnan(hum) || isnan(temp)) {
      Serial.println(F("Gagal membaca data dari sensor DHT!")); 
      return; 
    }

    // Hitung heat index dalam Celsius
    hic = dht.computeHeatIndex(temp, hum, false);

    // Cetak data ke Serial Monitor 
    Serial.print(F("Kelembaban: ")); Serial.print(hum); Serial.print(F("%  ")); 
    Serial.print(F("Suhu: "));  Serial.print(temp);   Serial.print(F("°C  ")); 
    Serial.print(F("Heat index: ")); Serial.print(hic); Serial.println(F("°C ")); 

    // --- TAMBAHAN: Tampilkan data ke layar OLED ---
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.setCursor(0, 0);
    display.print("Suhu: ");
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print(temp);
    display.print(" C");

    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print("Lembab: ");
    display.setTextSize(2);
    display.setCursor(0, 45);
    display.print(hum);
    display.print(" %");
    
    display.display(); // Tampilkan ke layar
    // ------------------------------------------

    // Kirim data ke ThingSpeak
    HTTPClient http;
    String httpRequestData = "api_key=" + apiKey + "&field1=" + String(temp) + "&field2=" + String(hum) + "&field3=" + String(hic);
    
    http.begin(thingSpeakAddress); // Kirim request POST 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); 
    int httpResponseCode = http.POST(httpRequestData);
    
    // Cek respons dari server 
    if (httpResponseCode > 0) { 
      Serial.print("Kode respons HTTP: "); 
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Kode Error: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Tutup koneksi
    lastTime = millis(); // Reset timer
  }
}