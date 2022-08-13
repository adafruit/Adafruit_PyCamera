#include "esp_camera.h"
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_NeoPixel.h>

#ifndef TAG
#define TAG "PYCAM"
#endif

// on the main chip
#define SHUTTER_BUTTON 0
// on the seesaw expander
#define SS_CARDDET 4
#define SS_BATTMON 18
#define SS_CAMERA_RST 19
#define SS_CAMERA_PWDN 20


class Adafruit_PyCamera : public Adafruit_ST7789 {
 public:
  Adafruit_PyCamera();

  bool begin(void);

  bool initCamera(void);
  bool initDisplay(void);
  bool initSeesaw(void);

  bool captureAndBlit(void);
  void setNeopixel(uint32_t c);
  uint32_t Wheel(byte WheelPos);

  sensor_t *camera;
  Adafruit_NeoPixel pixel;
};
