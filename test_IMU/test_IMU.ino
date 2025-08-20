#include <M5Unified.h>

void setup(void)
{
  delay(2000);
  auto cfg            = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);

  M5.Lcd.setTextColor(WHITE, BLACK);
}

void loop(void)
{
  auto imu_update = M5.Imu.update();
  if (imu_update)
  {
    auto data = M5.Imu.getImuData();

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK);

    // --- Judul AETHERS ---
    M5.Lcd.setTextSize(2);            // Font lebih besar
    M5.Lcd.setTextDatum(TC_DATUM);    // Top Center
    M5.Lcd.drawString("AETHERS", M5.Lcd.width()/2, 10);

    // --- Data IMU ---
    char buf[64];
    sprintf(buf, "%0.2f %0.2f %0.2f", data.accel.x, data.accel.y, data.accel.z);
    Serial.printf("%f,%f,%f\r\n", data.accel.x, data.accel.y, data.accel.z);

    M5.Lcd.setTextSize(1);            // Kecilkan kembali
    M5.Lcd.setTextDatum(TC_DATUM);    // Tetap rata tengah
    M5.Lcd.drawString(buf, M5.Lcd.width()/2, 50);   // tampil di bawah judul
  }
  delay(200);
}
