#include "Adafruit_PyCamera.h"

Adafruit_PyCamera::Adafruit_PyCamera(void) 
  : Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RESET) {

}


bool Adafruit_PyCamera::begin(void) {
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, LOW); // Backlight off
  init(240, 240);   // Initialize ST7789 screen
  setRotation(1);
  fillScreen(ST77XX_RED);


  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on
  
  return true;
}
