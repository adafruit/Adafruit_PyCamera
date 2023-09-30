#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_NeoPixel.h>
#include <Adafruit_AW9523.h>
#include <SdFat.h>
#include "esp_camera.h"

#ifndef TAG
#define TAG "PYCAM"
#endif

/*
#define AW_DOWN_MASK (1UL << AW_DOWN)
#define AW_LEFT_MASK (1UL << AW_LEFT)
#define AW_UP_MASK  (1UL << AW_UP)
#define AW_RIGHT_MASK  (1UL << AW_RIGHT)
#define AW_CARDDET_MASK (1UL << AW_CARDDET)
#define AW_INPUTS_MASK (AW_DOWN_MASK | AW_LEFT_MASK | AW_UP_MASK | AW_RIGHT_MASK | AW_CARDDET_MASK)
*/
class Adafruit_PyCamera : public Adafruit_ST7789 {
 public:
  Adafruit_PyCamera();

  bool begin(void);

  bool initCamera(void);
  bool initDisplay(void);
  bool initExpander(void);
  bool initSD(void);
  void I2Cscan(void);

  bool captureAndBlit(void);

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
  Adafruit_NeoPixel pixel;
  Adafruit_AW9523 aw;
  SdFat sd;
  uint32_t _timestamp;
  uint32_t last_button_state = 0xFFFFFFFF;
  uint32_t button_state = 0xFFFFFFFF;

  camera_config_t camera_config;
};
