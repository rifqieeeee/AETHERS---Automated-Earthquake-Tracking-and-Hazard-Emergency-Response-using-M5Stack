#include <M5Unified.h>
#include "M5AtomS3.h"
#include <WiFi.h>
#include <PubSubClient.h>

bool baseline_set = false;
float baseX, baseY, baseZ;

WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid        = "Sekolah Robot Indonesia";
const char* password    = "sadapsadap";
const char* mqtt_server = "broker.hivemq.com";

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setupWifi();
void callback(char* topic, byte* payload, unsigned int length);
void reConnect();

void setup(void)
{
  delay(2000);
  auto cfg            = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);

  M5.Lcd.setTextColor(WHITE, BLACK);

  AtomS3.begin();
  setupWifi();
  Serial.begin(9600);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

bool earthquakeSent = false;  // flag untuk cek apakah pesan sudah dikirim

void loop(void) {
    if (!client.connected()) {
        reConnect();
    }
    client.loop();

    auto imu_update = M5.Imu.update();
    if (imu_update) {
        auto data = M5.Imu.getImuData();

        if (!baseline_set) {
            baseX = data.accel.x;
            baseY = data.accel.y;
            baseZ = data.accel.z;
            baseline_set = true;
        }

        float diffX = fabs(data.accel.x - baseX);
        float diffY = fabs(data.accel.y - baseY);
        float diffZ = fabs(data.accel.z - baseZ);

        float diffTotal = sqrt(diffX*diffX + diffY*diffY + diffZ*diffZ);

        float faktor = 1.2;
        float SR = pow(10, diffTotal * faktor);

        if (SR > 10.0) SR = 10.0;

        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextColor(WHITE, BLACK);

        M5.Lcd.setTextSize(2);            
        M5.Lcd.setTextDatum(TC_DATUM);    
        M5.Lcd.drawString("AETHERS", M5.Lcd.width()/2, 10);

        char buf[64];
        sprintf(buf, "%0.2f %0.2f %0.2f", data.accel.x, data.accel.y, data.accel.z);
        Serial.printf("%f,%f,%f\r\n", data.accel.x, data.accel.y, data.accel.z);

        M5.Lcd.setTextSize(1);            
        M5.Lcd.setTextDatum(TC_DATUM);    
        M5.Lcd.drawString(buf, M5.Lcd.width()/2, 50);

        char bufSR[32];
        sprintf(bufSR, "SR: %.2f", SR);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(YELLOW, BLACK);
        M5.Lcd.drawString(bufSR, M5.Lcd.width()/2, 70);

        bool earthquake = (SR >= 4);

        if (earthquake) {
            M5.Lcd.setTextSize(2);
            M5.Lcd.setTextColor(RED, BLACK);
            M5.Lcd.drawString("Earthquake", M5.Lcd.width()/2, 100);

            if (!earthquakeSent) {
                char message[256];
                snprintf(message, sizeof(message),
                    "{\"ID\":\"7\",\"SR\":%.2f,\"latitude\":-6.1,\"longitude\":106.82,\"kedalaman\":\"10\",\"lokasi\":\"Jakarta\"}",
                    SR
                );
                client.publish("earthquake/magnitude", message);
                earthquakeSent = true;
            }
        } else {
            M5.Lcd.setTextSize(2);
            M5.Lcd.setTextColor(GREEN, BLACK);
            M5.Lcd.drawString("Normal", M5.Lcd.width()/2, 100);
        }
    }

    delay(200);
}

void setupWifi() {
    delay(10);
    AtomS3.Lcd.print("Connecting to Network...");
    Serial.printf("Connecting to %s", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nSuccess\n");
    AtomS3.Lcd.println("Success");
    AtomS3.Lcd.println("For communication information see serial port");
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void reConnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "M5Stack-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            client.publish("earthquake/alerts", "Device connected");
            client.subscribe("earthquake/alerts");  // kalau ingin juga menerima pesan
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

