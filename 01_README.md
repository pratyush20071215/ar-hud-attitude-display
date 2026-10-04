# ar-hud-attitude-display

Bench prototype of a low-cost head-up attitude display for general aviation.
An Arduino Uno R3 reads an MPU6050 IMU and shows pitch/roll on a 1.3" SH1106
OLED, projected into the forward view through a biconvex lens and an acrylic
combiner.

## Demo
[video 1 link] https://github.com/user-attachments/assets/48061d6f-56d3-4579-ba7f-48514dd9a5fb
*Caption: what this shows.*

![Hardware build](your-image-link) <img width="1600" height="1200" alt="WhatsApp Image 2026-10-04 at 19 38 31" src="https://github.com/user-attachments/assets/9cb6fef2-9329-48df-9e54-0a7867a5abde" />  
*Caption: OLED layout.*

## Status
- **Working:** MPU6050 (gyro-bias calibrated) with a complementary filter
  (98% gyro, 2% accelerometer) and OLED display of pitch/roll.
- **Not working:** the NEO-6M GPS module is faulty. The NMEA parser is written
  but disabled until the replacement arrives.
- **Not measured yet:** attitude accuracy, drift and update rate.

## Hardware
Arduino Uno R3, MPU6050, SH1106 1.3" OLED, biconvex lens, acrylic combiner,
NEO-6M GPS (faulty).

| Signal | Pin |
|---|---|
| [fill in SDA/SCL, GPS RX/TX] | [fill in] |

## Software
Libraries used: Wire.h, SoftwareSerial.h,avr-libc 
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
Not for flight. No magnetometer (the displayed "heading" is GPS track), the
uncoated combiner is hard to read in daylight, no barometric altitude,
uncertified. Bench prototype only.

## Roadmap
1. Validate pitch/roll against a protractor reference and report the error.
2. Test the GPS once the replacement arrives.

Build log and debugging history: see `/docs`.
