# AR-HUD-attitude-display
Built with AI assistance (Claude). Hardware assembly, wiring, testing and fault diagnosis were done by me.
Bench prototype of a low-cost head-up attitude display for general aviation.
An Arduino Uno R3 reads an MPU6050 IMU and shows pitch/roll on a 1.3" SH1106
OLED, projected into the forward view through a biconvex lens and an acrylic
combiner.

## Demo
Project setup(current): <img width="1600" height="1200" alt="WhatsApp Image 2026-10-15 at 18 58 08" src="https://github.com/user-attachments/assets/c213e345-1553-435d-bc04-e2749fb25d32" /> 

[Hardware build] 

https://github.com/user-attachments/assets/855e59c7-95fa-488d-a651-3dfe779b5f4a

https://github.com/user-attachments/assets/0999632d-8ee0-4091-b69e-9a7fbe59eff1

## Status
- **Working:** MPU6050 (gyro-bias calibrated) with a complementary filter
  (98% gyro, 2% accelerometer) and OLED display of pitch/roll.
- **Not working:** the NEO-6M GPS module is faulty. The NMEA parser is written
  but disabled until the replacement arrives.
- **Not yet measured:** attitude accuracy, drift and update rate.

## Hardware
Arduino Uno R3, MPU6050, SH1106 1.3" OLED, biconvex lens, acrylic combiner,NEO-6M GPS (faulty).

| Signal | Pin |
|---|---|
| Power supply(Display) | 3.3V |
| Power supply(MPU6050 & NEO-6M | 5V |
| Data transmission(MPU6050 & Display) | A4 to SDA, A5 to SCL | 

## Software
Libraries used: Wire.h, SoftwareSerial.h,avr-lib
Hand-written parts: SH1106 driver, NMEA parser, 7-segment renderer.
A Windows host tool uses the Python standard library and ctypes (kernel32)
instead of pyserial.

## Reliability
- I2C timeout with bus reset: 'ar_nav.ino'
- Watchdog: [2 s].

## Faults found and fixed
| Fault              | Symptom   | Cause                      | Fix |
|---                 |---        |---                         |---  |
| Display controller | Glitching | SH1106 vs SSD1306 mismatch | redited code for right driver|
| Supply for MPU     | Zero data | 5 V vs 3.3 V mismatch      | changed voltage pins |


## Limitations
Not for flight. No magnetometer (the displayed "heading" is GPS track), no barometric altitude,
uncertified. Bench prototype only.

## Roadmap
1. Validate pitch/roll against a protractor reference and report the error.
2. Test the GPS once the replacement arrives.

Build log and debugging history: see `/docs`.
