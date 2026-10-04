// AR Nav prototype - Arduino Uno R3 - MPU6050 + OLED build (GPS removed)
// MPU6050 (I2C) -> SH1106 OLED (I2C), hand-rolled drivers, no third-party libraries.
// HUD-style symbology: fixed boresight reference, roll-tilted horizon, roll in degrees.
#include <Wire.h>
#include <math.h>
#include <string.h>
#include <avr/wdt.h>

#define MPU_ADDR 0x68
#define OLED_ADDR 0x3C
#define OLED_COL_OFFSET 2

uint8_t fb[1024];
float pitch = 0, roll = 0;
float pitchTrim = 0, rollTrim = 0;
float gxOff = 0, gyOff = 0;
unsigned long lastUs;

// ---------------- MPU6050 ----------------
int16_t rd16(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR); Wire.write(reg); Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2);
  return (Wire.read() << 8) | Wire.read();
}
void mpuInit() {
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x6B); Wire.write(0); Wire.endTransmission();
}
void mpuCalibrate() {                          // ADD THIS FUNCTION
  long sx = 0, sy = 0;
  for (int i = 0; i < 200; i++) { sx += rd16(0x43); sy += rd16(0x45); delay(3); }
  gxOff = sx / 200.0; gyOff = sy / 200.0;
  int16_t ax = rd16(0x3B), ay = rd16(0x3D), az = rd16(0x3F);
  pitchTrim = atan2(-ax, sqrt((float)ay * ay + (float)az * az)) * 57.29578;
  rollTrim  = atan2(ay, az) * 57.29578;
}
void mpuUpdate(float dt) {
  int16_t ax = rd16(0x3B), ay = rd16(0x3D), az = rd16(0x3F);
  int16_t gx = rd16(0x43), gy = rd16(0x45);
  float accPitch = atan2(-ax, sqrt((float)ay * ay + (float)az * az)) * 57.29578;
  float accRoll  = atan2(ay, az) * 57.29578;
  pitch = 0.98 * (pitch + ((gx - gxOff) / 131.0) * dt) + 0.02 * (accPitch - pitchTrim);
  roll  = 0.98 * (roll  + ((gy - gyOff)/ 131.0) * dt) + 0.02 * (accRoll - rollTrim);
}

// ---------------- SH1106 I2C driver ----------------
void oledCmd(uint8_t c) {
  Wire.beginTransmission(OLED_ADDR); Wire.write(0x00); Wire.write(c); Wire.endTransmission();
}
void oledInit() {
  const uint8_t init[] = {0xAE, 0xD5,0x80, 0xA8,0x3F, 0xD3,0x00, 0x40,
                           0xAD,0x8B, 0xA1, 0xC8, 0xDA,0x12,
                           0x81,0xCF, 0xD9,0xF1, 0xDB,0x40, 0xA4, 0xA6, 0xAF};
  for (uint8_t i = 0; i < sizeof(init); i++) oledCmd(init[i]);
}
void setPixel(int x, int y) {
  if (x < 0 || x > 127 || y < 0 || y > 63) return;
  fb[x + (y >> 3) * 128] |= (1 << (y & 7));
}
void line(int x0, int y0, int x1, int y1) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1, err = dx + dy;
  while (true) {
    setPixel(x0, y0);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}
const uint8_t SEG[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
void digit(int x, int y, uint8_t d) {
  uint8_t s = SEG[d];
  if (s & 0x01) line(x,   y,   x+6, y);
  if (s & 0x02) line(x+6, y,   x+6, y+5);
  if (s & 0x04) line(x+6, y+5, x+6, y+10);
  if (s & 0x08) line(x,   y+10,x+6, y+10);
  if (s & 0x10) line(x,   y+5, x,   y+10);
  if (s & 0x20) line(x,   y,   x,   y+5);
  if (s & 0x40) line(x,   y+5, x+6, y+5);
}
int number(int x, int y, int v) {             // now returns final cursor X
  if (v < 0) { line(x, y+4, x+3, y+4); x += 5; v = -v; }
  if (v >= 100) { digit(x, y, (v/100)%10); x += 8; }
  if (v >= 10)  { digit(x, y, (v/10)%10);  x += 8; }
  digit(x, y, v % 10);
  return x + 8;
}
void degreeSymbol(int x, int y) {              // 4-pixel ring, no trig, no font table
  setPixel(x+1, y); setPixel(x, y+1); setPixel(x+2, y+1); setPixel(x+1, y+2);
}
void oledFlush() {
  for (uint8_t page = 0; page < 8; page++) {
    oledCmd(0xB0 + page);
    oledCmd(0x00 | (OLED_COL_OFFSET & 0x0F));
    oledCmd(0x10 | (OLED_COL_OFFSET >> 4));
    for (uint8_t c = 0; c < 128; c += 16) {
      Wire.beginTransmission(OLED_ADDR); Wire.write(0x40);
      for (uint8_t k = 0; k < 16; k++) Wire.write(fb[page * 128 + c + k]);
      Wire.endTransmission();
    }
  }
}

// ---------------- Symbology ----------------
void boresight() {                             // fixed aircraft reference, center screen
  line(48, 32, 58, 32);   // left tick
  line(70, 32, 80, 32);   // right tick
  setPixel(64, 32);       // center point (gap between ticks = boresight gap, standard HUD convention)
}
void draw() {
  memset(fb, 0, sizeof(fb));
  line(20, 32 - (int)roll, 108, 32 + (int)roll);  // roll-tilted horizon
  boresight();
  number(4, 50, (int)pitch);
  int rx = number(88, 50, (int)roll);
  degreeSymbol(rx, 50);
  oledFlush();
}

void telemetry() {
  Serial.print(pitch, 1); Serial.print(',');
  Serial.println(roll, 1);
}

void setup() {
  Wire.begin(); Wire.setClock(400000);
  Wire.setWireTimeout(3000, true);
  Serial.begin(115200);
  mpuInit();
  mpuCalibrate();
  oledInit();
  lastUs = micros();
  wdt_enable(WDTO_2S);
}
void loop() {
  wdt_reset();
  unsigned long now = micros();
  float dt = (now - lastUs) / 1e6; lastUs = now;
  mpuUpdate(dt);
  draw();
  telemetry();
  delay(50);
}
