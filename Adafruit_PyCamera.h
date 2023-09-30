#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_NeoPixel.h>
#include <Adafruit_AW9523.h>
#include <SdFat.h>
#include "esp_camera.h"

#ifndef TAG
#define TAG "PYCAM"
#endif

#define AW_DOWN_MASK (1UL << AWEXP_BUTTON_DOWN)
#define AW_LEFT_MASK (1UL << AWEXP_BUTTON_LEFT)
#define AW_UP_MASK  (1UL << AWEXP_BUTTON_UP)
#define AW_RIGHT_MASK  (1UL << AWEXP_BUTTON_RIGHT)
#define AW_OK_MASK  (1UL << AWEXP_BUTTON_OK)
#define AW_SEL_MASK  (1UL << AWEXP_BUTTON_SEL)
#define AW_CARDDET_MASK (1UL << AWEXP_SD_DET)
#define AW_INPUTS_MASK (AW_DOWN_MASK | AW_LEFT_MASK | AW_UP_MASK | AW_RIGHT_MASK | AW_OK_MASK | AW_SEL_MASK | AW_CARDDET_MASK)



class PyCameraFB : public GFXcanvas16 {
 public:
 PyCameraFB(uint16_t w, uint16_t h) : GFXcanvas16(w,h) { free(buffer); buffer = NULL; };
  
  void setFB(uint16_t *fb) {
    buffer = fb;
  }
};


class Adafruit_PyCamera : public Adafruit_ST7789 {
 public:
  Adafruit_PyCamera();

  bool begin(void);

  bool initCamera(void);
  bool initDisplay(void);
  bool initExpander(void);
  bool initSD(void);
  void endSD(void);
  void I2Cscan(void);

  bool captureFrame(void);
  void blitFrame(void);

  void speaker_tone(uint32_t tonefreq, uint32_t tonetime);

  float readBatteryVoltage(void);
  uint32_t readButtons(void);
  bool SDdetected(void);
  bool justReleased(uint8_t button_pin);
  bool justPressed(uint8_t button_pin);

  void setNeopixel(uint32_t c);
  uint32_t Wheel(byte WheelPos);

  uint32_t timestamp(void);
  void timestampPrint(const char *msg) ;

  sensor_t *camera;
  camera_fb_t *frame = NULL;
  PyCameraFB *fb;

  Adafruit_NeoPixel pixel;
  Adafruit_AW9523 aw;
  SdFat sd;
  uint32_t _timestamp;
  uint32_t last_button_state = 0xFFFFFFFF;
  uint32_t button_state = 0xFFFFFFFF;

  camera_config_t camera_config;
};

