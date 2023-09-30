#include <Arduino.h>
#include "Adafruit_PyCamera.h"

Adafruit_PyCamera pycamera;


void setup() {
  Serial.begin(115200);
  while (!Serial) yield();
  delay(1000);

  Serial.setDebugOutput(true);
  Serial.println("Hello world");
  if (! pycamera.begin()) {
    Serial.println("Failed to initialize pyCamera interface");
    while (1) yield();
  }
  Serial.println("pyCamera hardware initialized!");
  
}



void loop() {
  static uint8_t loopn=0;
  pycamera.setNeopixel(pycamera.Wheel(loopn));
  loopn += 8;
  
  pycamera.readButtons();
  //Serial.printf("Buttons: 0x%08X\n\r",  pycamera.readButtons());

  if (pycamera.justPressed(SHUTTER_BUTTON)) {
    Serial.println("Snap!");
    pycamera.speaker_tone(988, 100);  // tone1 - B5
    pycamera.speaker_tone(1319, 200); // tone2 - E6
  }

  //pycamera.timestamp();
  pycamera.captureFrame();
  
  // once the frame is captured we can draw ontot he framebuffer
  if (pycamera.justPressed(AWEXP_SD_DET)) {
    Serial.println("SD Card removed");
    pycamera.endSD();
    pycamera.fb->setCursor(0, 32);
    pycamera.fb->setTextSize(2);
    pycamera.fb->setTextColor(pycamera.color565(255, 0, 0));
    pycamera.fb->print("SD Card removed");
    delay(200);
  }
  if (pycamera.justReleased(AWEXP_SD_DET)) {
    Serial.println("SD Card inserted!");
    pycamera.initSD();
    pycamera.fb->setCursor(0, 32);
    pycamera.fb->setTextSize(2);
    pycamera.fb->setTextColor(pycamera.color565(255, 0, 0));
    pycamera.fb->print("SD Card inserted");
    delay(200);
  }

  float A0_voltage = analogRead(A0) / 4096.0 * 3.3;
  float A1_voltage = analogRead(A1) / 4096.0 * 3.3;
  if (loopn == 0) {
    Serial.printf("A0 = %0.1f V, A1 = %0.1f V, Battery = %0.1f V\n\r", 
                  A0_voltage, A1_voltage, pycamera.readBatteryVoltage());
  }
  pycamera.fb->setCursor(0,0);
  pycamera.fb->setTextSize(2);
  pycamera.fb->setTextColor(pycamera.color565(255, 255, 255));
  pycamera.fb->print("A0 = "); pycamera.fb->print(A0_voltage, 1);
  pycamera.fb->print("V, A1 = "); pycamera.fb->print(A1_voltage, 1);
  pycamera.fb->print("V\nBattery = "); pycamera.fb->print(pycamera.readBatteryVoltage(), 1); 
  pycamera.fb->print(" V");


  float x_ms2, y_ms2, z_ms2;
  if (pycamera.readAccelData(&x_ms2, &y_ms2, &z_ms2)) {
    Serial.printf("X=%0.2f, Y=%0.2f, Z=%0.2f\n\r", x_ms2, y_ms2, z_ms2);
    pycamera.fb->setCursor(0, 220);
    pycamera.fb->setTextSize(2);
    pycamera.fb->setTextColor(pycamera.color565(255, 255, 255));
    pycamera.fb->print("3D: "); pycamera.fb->print(x_ms2, 1);
    pycamera.fb->print(", "); pycamera.fb->print(y_ms2, 1);
    pycamera.fb->print(", "); pycamera.fb->print(z_ms2, 1);
  }
  
  pycamera.blitFrame();

  delay(10);
  
}
