#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_NeoPixel.h>
#include <Adafruit_seesaw.h>
#include <SdFat.h>
#include "esp_camera.h"

#ifndef TAG
#define TAG "PYCAM"
#endif

// on the main chip
#define SHUTTER_BUTTON 0
// on the seesaw expander
// inputs
#define SS_DOWN 16
#define SS_LEFT 2
#define SS_UP 3
#define SS_RIGHT 6
#define SS_CARDDET 4
#define SS_DOWN_MASK (1UL << SS_DOWN)
#define SS_LEFT_MASK (1UL << SS_LEFT)
#define SS_UP_MASK  (1UL << SS_UP)
#define SS_RIGHT_MASK  (1UL << SS_RIGHT)
#define SS_CARDDET_MASK (1UL << SS_CARDDET)
#define SS_INPUTS_MASK (SS_DOWN_MASK | SS_LEFT_MASK | SS_UP_MASK | SS_RIGHT_MASK | SS_CARDDET_MASK)
// analogs
#define SS_BATTMON 18

// outputs
#define SS_CAMERA_RST 19
#define SS_CAMERA_PWDN 20
#define SS_MUTE 17

class Adafruit_PyCamera : public Adafruit_ST7789 {
 public:
  Adafruit_PyCamera();

  bool begin(void);

  bool initCamera(void);
  bool initDisplay(void);
  bool initSeesaw(void);
  bool initSD(void);

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
  Adafruit_seesaw ss;
  SdFat sd;
  uint32_t _timestamp;
  uint32_t last_button_state = 0xFFFFFFFF;
  uint32_t button_state = 0xFFFFFFFF;
};
