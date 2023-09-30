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
  bool initAccel(void);
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

  bool readAccelData(int16_t *x, int16_t *y, int16_t *z);
  bool readAccelData(float *x, float *y, float *z);

  void setNeopixel(uint32_t c);
  uint32_t Wheel(byte WheelPos);

  uint32_t timestamp(void);
  void timestampPrint(const char *msg) ;

  sensor_t *camera;
  camera_fb_t *frame = NULL;
  PyCameraFB *fb = NULL;

  Adafruit_NeoPixel pixel;
  Adafruit_AW9523 aw;
  Adafruit_I2CDevice *lis_dev = NULL;

  SdFat sd;
  uint32_t _timestamp;
  uint32_t last_button_state = 0xFFFFFFFF;
  uint32_t button_state = 0xFFFFFFFF;

  camera_config_t camera_config;
};


#define LIS3DH_REG_STATUS1 0x07
#define LIS3DH_REG_OUTADC1_L 0x08 /**< 1-axis acceleration data. Low value */
#define LIS3DH_REG_OUTADC1_H 0x09 /**< 1-axis acceleration data. High value */
#define LIS3DH_REG_OUTADC2_L 0x0A /**< 2-axis acceleration data. Low value */
#define LIS3DH_REG_OUTADC2_H 0x0B /**< 2-axis acceleration data. High value */
#define LIS3DH_REG_OUTADC3_L 0x0C /**< 3-axis acceleration data. Low value */
#define LIS3DH_REG_OUTADC3_H 0x0D /**< 3-axis acceleration data. High value */
#define LIS3DH_REG_INTCOUNT  0x0E /**< INT_COUNTER register [IC7, IC6, IC5, IC4, IC3, IC2, IC1, IC0] */
#define LIS3DH_REG_WHOAMI  0x0F
#define LIS3DH_REG_TEMPCFG 0x1F
#define LIS3DH_REG_CTRL1 0x20
#define LIS3DH_REG_CTRL2 0x21
#define LIS3DH_REG_CTRL3 0x22
#define LIS3DH_REG_CTRL4 0x23
#define LIS3DH_REG_CTRL5 0x24
#define LIS3DH_REG_CTRL6 0x25
#define LIS3DH_REG_STATUS2 0x27
#define LIS3DH_REG_OUT_X_L 0x28 /**< X-axis acceleration data. Low value */
#define LIS3DH_LSB16_TO_KILO_LSB10 6400
