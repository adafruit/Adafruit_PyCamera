#include "esp_camera.h"
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789


class Adafruit_PyCamera : public Adafruit_ST7789 {
 public:
  Adafruit_PyCamera();

  bool begin(void);

};
