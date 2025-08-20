#include <M5Unified.h>

bool baseline_set = false;
float baseX, baseY, baseZ;

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

    // Set baseline hanya sekali di awal
    if (!baseline_set) {
      baseX = data.accel.x;
      baseY = data.accel.y;
      baseZ = data.accel.z;
      baseline_set = true;
    }

    // --- Hitung delta akselerasi ---
    float diffX = fabs(data.accel.x - baseX);
    float diffY = fabs(data.accel.y - baseY);
    float diffZ = fabs(data.accel.z - baseZ);

    // Magnitudo delta (total perubahan akselerasi)
    float diffTotal = sqrt(diffX*diffX + diffY*diffY + diffZ*diffZ);

    float faktor = 1.2;   // bisa diubah sesuai eksperimen
    float SR = pow(10, diffTotal * faktor);  

    // Batasi agar tidak lebih dari 10
    if (SR > 10.0) SR = 10.0;


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
    M5.Lcd.setTextSize(1);            
    M5.Lcd.setTextDatum(TC_DATUM);    
    M5.Lcd.drawString(buf, M5.Lcd.width()/2, 50);

    // --- Tampilkan SR ---
    char bufSR[32];
    sprintf(bufSR, "SR: %.2f", SR);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.drawString(bufSR, M5.Lcd.width()/2, 70);

    // --- Status Normal / Earthquake ---
    bool earthquake = (SR >= 4);

    if (earthquake) {
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(RED, BLACK);
      M5.Lcd.drawString("Earthquake", M5.Lcd.width()/2, 100);
    } else {
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.drawString("Normal", M5.Lcd.width()/2, 100);
    }
  }
  delay(200);
}
