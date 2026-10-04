# ar-hud-attitude-display(current stage)
Bringing Flat Display values which face risks of glaring, especially in general aviation, to 3D using an AR system
https://github.com/user-attachments/assets/64fcda81-c5b1-4346-80a7-52846426377e
<img width="1600" height="1200" alt="WhatsApp Image 2026-10-04 at 19 38 31" src="https://github.com/user-attachments/assets/f8685c11-ba8a-4b46-82a4-f1853676aaa8" />
https://github.com/user-attachments/assets/4e314238-da9f-44f7-9a37-31c74e2f305f

Hardware: Arduino Uno R3, MPU6050, SH1106 OLED, biconvex lens, acrylic combiner.
# AR Navigation Prototype (General Aviation, Bench Build, and the final planned phase)

A low-cost AR instrument prototype for general aviation: an Arduino Uno R3
reads an MPU6050 IMU and NEO-6M GPS, then drives a 1.3" OLED combined into
the forward view via a biconvex lens + acrylic sheet.

**Constraints:** no third-party Arduino libraries (Wire.h/SoftwareSerial.h/
avr-libc only) — hand-rolled SH1106 I2C driver, NMEA parser, and 7-segment
renderer. Windows host tool uses Python stdlib + ctypes→kernel32 only, no
pyserial.

**Faults found and fixed.** The SH1106/SSD1306 controller mismatch, the 5 V/3.3 V supply mismatch and the firmware regression.

**Status:** MPU6050 (gyro-bias calibrated, complementary filter) and OLED
confirmed working. GPS module confirmed dead; replacement pending — code
retained, disabled.

**Not for flight.** No magnetometer (displayed "heading" is GPS track, not
true heading), uncoated combiner is illegible in daylight, no barometric
altitude, uncertified. Ground/bench prototype only.

**Roadmap.** Validation against a reference, plus a GPS test once the replacement arrives.

**TO NOTE: NO VERIFIED NUMBERS AS OF YET DUE TO MISSING GPS MODULE.**

See `/docs` for full build log and debugging history.
