Hardware

The Automated Earthquake Tracking and Hazard Emergency Response (AETHERS) system uses compact, IoT-ready devices to detect seismic activity and deliver real-time alerts.

Components
1. M5Stack AtomS3 (Earthquake Detection Node)

Processor: ESP32-S3 with 8 MB Flash

Display: 0.85-inch IPS screen

Sensors: Built-in MPU6886 6-axis IMU (3-axis accelerometer + 3-axis gyroscope)

Connectivity: Wi-Fi (802.11 b/g/n)

Function:

Detects ground acceleration using the accelerometer

Processes motion data to estimate earthquake intensity

Publishes earthquake information (magnitude, location) to an MQTT broker

2. M5StickC PLUS (Wearable Alert Device)

Processor: ESP32-PICO

Display: 1.14-inch color LCD

Power: Built-in 120 mAh LiPo battery (USB rechargeable)

Controls: Onboard buttons (for SOS trigger)

Connectivity: Wi-Fi (802.11 b/g/n)

Accessories: Watch band for wearable form factor

Function:

Subscribes to earthquake alerts via MQTT

Displays real-time earthquake notifications (location, magnitude, depth)

Sends SOS signals when the user presses the button, notifying rescue teams through the dashboard

Hardware Setup Overview

AtomS3 → MQTT Broker: Publishes earthquake data in JSON format

M5StickC PLUS → MQTT Broker: Subscribes to earthquake alerts, publishes SOS alerts

Both devices operate over Wi-Fi and require an SSID/password configured in code.

Power can be supplied via USB or internal batteries for portable operation.
