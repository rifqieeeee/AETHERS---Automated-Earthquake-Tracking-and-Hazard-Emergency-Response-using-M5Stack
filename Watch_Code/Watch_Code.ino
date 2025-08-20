#include "M5StickCPlus.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid        = "Sekolah Robot Indonesia";
const char* password    = "sadapsadap";
const char* mqtt_server = "broker.hivemq.com";
const char* topic_users = "earthquake/alerts";
const char* topic_eq    = "earthquake/magnitude";

String displayText = "AETHERS";  // Teks standby default
bool newMessage = false;         // Penanda ada pesan baru
String lastDisplayed = "";  // Tambahkan variabel global

String lastLokasi = "";
float lastSR = 0.0;
String lastKedalaman = "";

void setupWifi();
void callback(char* topic, byte* payload, unsigned int length);
void reConnect();
void showCenteredText(String text, uint16_t color, uint8_t size);

void setup() {
    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    setupWifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    showCenteredText(displayText, YELLOW, 3);
}

void loop() {
    if (!client.connected()) {
        reConnect();
    }
    client.loop();

    M5.update();

    // Jika tombol A ditekan → kirim pesan darurat
    if (M5.BtnA.wasReleased()) {
        client.publish(topic_users, "User 1 - Code Red - Need Help ASAP");
        showCenteredText("HELP SENT!", RED, 2);
        delay(2000);

        // Kembali ke tampilan sebelumnya
        if (newMessage && lastDisplayed == "EQ") {
            showEarthquakeInfo(lastLokasi.c_str(), lastSR, lastKedalaman.c_str());
        } else {
            showCenteredText("AETHERS", YELLOW, 3);
        }
    }

    // Jika tidak ada pesan gempa → tetap tampilkan "AETHERS" hanya sekali
    if (!newMessage && lastDisplayed != "AETHERS") {
        showCenteredText("AETHERS", YELLOW, 3);
        lastDisplayed = "AETHERS";
    }
}

void setupWifi() {
    delay(10);
    M5.Lcd.printf("Connecting to %s", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");
    }
    M5.Lcd.printf("\nSuccess\n");
}

void callback(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, topic_eq) == 0) {
        String jsonStr;
        for (int i = 0; i < length; i++) {
            jsonStr += (char)payload[i];
        }

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, jsonStr);
        if (error) {
            showCenteredText("Invalid Data", RED, 2);
            newMessage = false;
            return;
        }

        const char* lokasi = doc["lokasi"];
        float sr = doc["SR"];
        const char* kedalaman = doc["kedalaman"];

        // simpan teks terakhir agar bisa dikembalikan setelah HELP SENT
        lastDisplayed = "EQ";
        lastLokasi = lokasi;
        lastSR = sr;
        lastKedalaman = kedalaman;
        newMessage = true;

        showEarthquakeInfo(lokasi, sr, kedalaman);
    }
}

void reConnect() {
    while (!client.connected()) {
        M5.Lcd.print("Attempting MQTT connection...");
        String clientId = "M5Stack-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            M5.Lcd.printf("\nSuccess\n");
            client.subscribe(topic_eq);
        } else {
            M5.Lcd.print("failed, rc=");
            M5.Lcd.print(client.state());
            M5.Lcd.println(" try again in 5s");
            delay(5000);
        }
    }
}

void showCenteredText(String text, uint16_t color, uint8_t size) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextSize(size);
    
    // hitung lebar & tinggi teks kira-kira
    int16_t w = text.length() * 6 * size;  // lebar per karakter * size
    int16_t h = 8 * size;                  // tinggi font * size

    int16_t x = (M5.Lcd.width() - w) / 2;
    int16_t y = (M5.Lcd.height() - h) / 2;

    if (x < 0) x = 0; // kalau teks terlalu panjang
    if (y < 0) y = 0;

    M5.Lcd.setCursor(x, y);
    M5.Lcd.print(text);
}

void showEarthquakeInfo(const char* lokasi, float sr, const char* kedalaman) {
    M5.Lcd.fillScreen(BLACK);

    // Baris 1: Judul
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(2);
    int16_t x = (M5.Lcd.width() - (strlen("Earthquake Detected") * 12)) / 2;
    if (x < 0) x = 0;
    M5.Lcd.setCursor(x, 10);
    M5.Lcd.println("Earthquake Detected");

    // Baris 2: Lokasi
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    x = (M5.Lcd.width() - (strlen(lokasi) * 12)) / 2;
    if (x < 0) x = 0;
    M5.Lcd.setCursor(x, 40);
    M5.Lcd.println(lokasi);

    // Baris 3: SR
    char srBuf[16];
    sprintf(srBuf, "%.1f SR", sr);
    x = (M5.Lcd.width() - (strlen(srBuf) * 12)) / 2;
    if (x < 0) x = 0;
    M5.Lcd.setCursor(x, 70);
    M5.Lcd.println(srBuf);

    // Baris 4: Kedalaman
    char kdBuf[32];
    sprintf(kdBuf, "Kedalaman %s KM", kedalaman);
    x = (M5.Lcd.width() - (strlen(kdBuf) * 12)) / 2;
    if (x < 0) x = 0;
    M5.Lcd.setCursor(x, 100);
    M5.Lcd.println(kdBuf);
}
