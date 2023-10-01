#include "Adafruit_PyCamera.h"

static uint16_t *jpeg_buffer = NULL;

bool buffer_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (!jpeg_buffer) return false;
  //Serial.printf("Drawing [%d, %d] to (%d, %d)\n", w, h, x, y);
  for(int xi = x; xi < x+w; xi++) {
    if (xi >= 240) continue;
    if (xi < 0) continue;
    for (int yi = y; yi < y+h; yi++) {
      if (yi >= 240) continue;
      if (yi < 0) continue;
      jpeg_buffer[yi * 240 + xi] = bitmap[(yi-y)*w + (xi-x)];
    }
  }
  return true;
}

Adafruit_PyCamera::Adafruit_PyCamera(void) 
  : Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RESET) {

}


bool Adafruit_PyCamera::begin(void) {
  Serial.println("Init PyCamera obj");
  // Setup and turn off speaker
  pinMode(SPEAKER, OUTPUT);
  digitalWrite(SPEAKER, LOW);

  // Setup and turn off Neopixel
  pixel.setPin(PIN_NEOPIXEL);
  pixel.updateLength(1);
  pixel.begin();
  pixel.setBrightness(50);
  setNeopixel(0x0);

  // boot button is also shutter
  pinMode(SHUTTER_BUTTON, INPUT_PULLUP);
  
  I2Cscan();

  if (! initExpander()) return false;
  if (! initDisplay()) return false;
  if (! initCamera(true, FRAMESIZE_VGA)) return false;  
  if (SDdetected() && ! initSD()) return false;
  if (! initAccel()) return false;

  fb = new PyCameraFB(240, 240);
  
  _timestamp = millis();

  return true;
}


bool Adafruit_PyCamera::initSD(void) {

  if (! SDdetected()) {
    Serial.println("No SD card inserted");
    return false;
  }

  Serial.println("SD card inserted, trying to init");
  
  // power reset
  //aw.pinMode(AWEXP_SD_PWR, OUTPUT);
  //aw.digitalWrite(AWEXP_SD_PWR, HIGH); // start off  
  //delay(10);
  //aw.digitalWrite(AWEXP_SD_PWR, LOW); // turn on

  if (!sd.begin(SD_CHIP_SELECT, SD_SCK_MHZ(4))) {
    if (sd.card()->errorCode()) {
      Serial.printf("SD card init failure with code %x data %d\n", 
                    sd.card()->errorCode(), sd.card()->errorData());
    }
    else if (sd.vol()->fatType() == 0) {
      Serial.println("Can't find a valid FAT16/FAT32 partition.");
    } else {
      Serial.println("SD begin failed, can't determine error type");
    }
    return false;
  }

  Serial.println("Card successfully initialized");
  uint32_t size = sd.card()->cardSize();
  if (size == 0) {
    Serial.println("Can't determine the card size");
  } else {
    uint32_t sizeMB = 0.000512 * size + 0.5;
    Serial.printf("Card size: %d MB FAT%d\n", sizeMB, sd.vol()->fatType());
  }
  Serial.println("Files found (date time size name):");
  sd.ls(LS_R | LS_DATE | LS_SIZE);
  return true;
}

void Adafruit_PyCamera::endSD() {
  //aw.pinMode(AWEXP_SD_PWR, OUTPUT);
  //aw.digitalWrite(AWEXP_SD_PWR, HIGH); // start off  
}

bool Adafruit_PyCamera::initExpander(void) {
  Serial.print("Init AW9523...");
  if (!aw.begin(0x5B)) {
    Serial.println("AW9523 not found!");
    return false;
  }
  Serial.println("OK!");
  aw.pinMode(AWEXP_SPKR_SD, OUTPUT);
  aw.digitalWrite(AWEXP_SPKR_SD, LOW); // start muted
  aw.pinMode(AWEXP_BACKLIGHT, OUTPUT);
  aw.digitalWrite(AWEXP_BACKLIGHT, LOW); // start dark
  aw.pinMode(AWEXP_SD_PWR, OUTPUT);
  aw.digitalWrite(AWEXP_SD_PWR, LOW); // start on 
  aw.pinMode(AWEXP_SD_DET, INPUT);
  return true;
}

bool Adafruit_PyCamera::initDisplay(void) {
  Serial.print("Init display....");

  aw.pinMode(AWEXP_BACKLIGHT, OUTPUT);
  aw.digitalWrite(AWEXP_BACKLIGHT, LOW); // Backlight off
  init(240, 240);   // Initialize ST7789 screen
  setRotation(1);
  fillScreen(ST77XX_GREEN);

  digitalWrite(AWEXP_BACKLIGHT, LOW); // Backlight on
  Serial.println("done!");
  return true;
}


bool Adafruit_PyCamera::initCamera(bool hwreset, framesize_t framesize) {
  Serial.print("Config camera...");

  if (hwreset) {
    // perform a hardware reset
    aw.pinMode(AWEXP_CAM_PWDN, OUTPUT);
    aw.pinMode(AWEXP_CAM_RST, OUTPUT);
    aw.digitalWrite(AWEXP_CAM_RST, LOW);
    aw.digitalWrite(AWEXP_CAM_PWDN, HIGH);
    delay(10);
    aw.digitalWrite(AWEXP_CAM_PWDN, LOW);
    delay(10);
    aw.digitalWrite(AWEXP_CAM_RST, HIGH);
    delay(10);
  }

  camera_config.ledc_channel = LEDC_CHANNEL_0;
  camera_config.ledc_timer = LEDC_TIMER_0;
  camera_config.pin_d0 = Y2_GPIO_NUM;
  camera_config.pin_d1 = Y3_GPIO_NUM;
  camera_config.pin_d2 = Y4_GPIO_NUM;
  camera_config.pin_d3 = Y5_GPIO_NUM;
  camera_config.pin_d4 = Y6_GPIO_NUM;
  camera_config.pin_d5 = Y7_GPIO_NUM;
  camera_config.pin_d6 = Y8_GPIO_NUM;
  camera_config.pin_d7 = Y9_GPIO_NUM;
  camera_config.pin_xclk = XCLK_GPIO_NUM;
  camera_config.pin_pclk = PCLK_GPIO_NUM;
  camera_config.pin_vsync = VSYNC_GPIO_NUM;
  camera_config.pin_href = HREF_GPIO_NUM;
  camera_config.pin_sccb_sda = SIOD_GPIO_NUM;
  camera_config.pin_sccb_scl = SIOC_GPIO_NUM;
  camera_config.pin_pwdn = PWDN_GPIO_NUM;
  camera_config.pin_reset = RESET_GPIO_NUM;
  camera_config.xclk_freq_hz = 20000000;
  camera_config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  camera_config.fb_location = CAMERA_FB_IN_PSRAM;

  Serial.print("Config format...");
  /* 
     // using RGB565 for immediate blitting
  camera_config.pixel_format = PIXFORMAT_RGB565;
  camera_config.frame_size = FRAMESIZE_240X240;     
  camera_config.fb_count = 1;
   */
  camera_config.pixel_format = PIXFORMAT_JPEG;
  camera_config.frame_size = framesize;
  camera_config.jpeg_quality = 10;
  camera_config.fb_count = 2;

  Serial.print("Initializing...");
  // camera init
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n\r", err);
    return false;
  }

  Serial.println("OK");

  camera = esp_camera_sensor_get();
  Serial.printf("Found camera PID %04X\n\r", camera->id.PID);
  camera->set_hmirror(camera, 1);

  // NOTE esp-camera deinits i2c!
  Wire.end();
  Wire.setPins(SDA, SCL);
  Wire.begin();

  return true;
}



float Adafruit_PyCamera::readBatteryVoltage(void) {
  return analogRead(BATT_MONITOR) * 2.0 * 3.3 / 4096;
}

bool Adafruit_PyCamera::SDdetected(void) {
  return aw.digitalRead(AWEXP_SD_DET);
}

uint32_t Adafruit_PyCamera::readButtons(void) {
  last_button_state = button_state;
  button_state = aw.inputGPIO() & AW_INPUTS_MASK;
  button_state |= (bool) digitalRead(SHUTTER_BUTTON);
  return button_state;
}

bool Adafruit_PyCamera::justPressed(uint8_t button_pin) {
  return ((last_button_state & (1UL << button_pin)) && // was not pressed before
          !(button_state & (1UL << button_pin))); // and is pressed now
}

bool Adafruit_PyCamera::justReleased(uint8_t button_pin) {
  return (!(last_button_state & (1UL << button_pin)) && // was pressed before
          (button_state & (1UL << button_pin))); // and isnt pressed now
}

void Adafruit_PyCamera::speaker_tone(uint32_t tonefreq, uint32_t tonetime) {
   aw.digitalWrite(AWEXP_SPKR_SD, HIGH); // un-mute
   tone(SPEAKER, tonefreq, tonetime);  // tone1 - B5
   delay(tonetime);
   aw.digitalWrite(AWEXP_SPKR_SD, LOW); // mute

}

bool Adafruit_PyCamera::takePhoto(char *filename_base) {
  if (!SDdetected()) {
    Serial.println("No SD card inserted");
    return false;
  }

  if (!sd.card() || (sd.card()->cardSize() == 0)) {
    Serial.println("No SD card found");
    return false;
  }

  char fullfilename[64];
  for (int inc=0; inc <= 1000; inc++) {
    if (inc == 1000) return false;
    snprintf(fullfilename, sizeof(fullfilename), "%s%03d.jpg", filename_base, inc);
    if (! sd.exists(fullfilename)) break;
  }

  File file;
  // Create the file
  if (! file.open(fullfilename, FILE_WRITE)) {
    Serial.printf("File '%s' open failed\n\r", fullfilename);
    return false;
  }

  //Serial.println("Reconfiguring resolution");
  // uint8_t ret = camera->set_framesize(camera, FRAMESIZE_240X240);
  // if (ret != 0) {
  //   Serial.printf("Could not set resolution: error 0x%x\n", ret);
  //  return false;
  //  }
  // delay(500);
  if (captureFrame()) {
      if (file.write(frame->buf, frame->len)) {
        Serial.printf("Saved JPEG to filename %s\n\r", fullfilename);
        file.close();
        sd.ls(LS_R | LS_DATE | LS_SIZE);

        esp_camera_fb_return(frame);
        return true;
      } else {
        Serial.println("Couldn't write JPEG data to file");
      }
  } else {
    Serial.println("Couldnt capture frame");
  }

  // even if it doesnt work out, reset camera and close file
  file.close();
  //if (camera->set_framesize(camera, FRAMESIZE_240X240) != 0) {
  //  Serial.println("Could not re-set resolution");
  // }
  return false;

}

uint32_t Adafruit_PyCamera::timestamp(void) {
  uint32_t delta = millis() - _timestamp;
  _timestamp = millis();
  return delta;
}

void Adafruit_PyCamera::timestampPrint(const char *msg) {
  Serial.printf("%s: %d ms elapsed\n\r", msg, timestamp());
}

bool Adafruit_PyCamera::captureFrame(void) {
  //Serial.println("Capturing...");
  esp_err_t res = ESP_OK;
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  int64_t fr_start = esp_timer_get_time();
#endif
  
  frame = esp_camera_fb_get();
  
  if (!frame) {
    ESP_LOGE(TAG, "Camera frame capture failed");
    return false;
  }
  
  /*
  Serial.printf("\t\t\tFramed %d bytes (%d x %d) in %d ms\n\r", 
                frame->len, 
                frame->width, frame->height, 
                timestamp());
  */
  if (camera_config.pixel_format == PIXFORMAT_JPEG) {
    //Serial.print("JPEG");
    // create the framebuffer if we haven't yet
    if (! fb->getBuffer() || ! jpeg_buffer) {
      jpeg_buffer = (uint16_t *) malloc(240*240*2);
      fb->setFB(jpeg_buffer);
    }
    uint16_t w = 0, h = 0, scale = 1;
    int xoff, yoff;
    TJpgDec.getJpgSize(&w, &h, frame->buf, frame->len);
    if (w <= 240 || h <= 240) {
      scale = 1;
      xoff = yoff = 0;
    } else if (w <= 480 || h <= 480) {
      scale = 2;
      xoff = (480-w)/2;
      yoff = (480-h)/2;
    } else if (w <= 960 || h <= 960) {
      scale = 4;
    } else {
      scale = 8;
    }
    Serial.printf(" size: %d x %d, scale %d\n\r", w, h, scale);
    TJpgDec.setJpgScale(scale);
    TJpgDec.setCallback(buffer_output);
    TJpgDec.drawJpg(xoff, yoff, frame->buf, frame->len);     
    fb->setFB(jpeg_buffer);
  } else if (camera_config.pixel_format == PIXFORMAT_RGB565) {
    // flip endians
    uint8_t temp;
    for (uint32_t i=0; i<frame->len; i+=2) {
      temp = frame->buf[i+0];
      frame->buf[i+0] = frame->buf[i+1];
      frame->buf[i+1] = temp;
    }
    fb->setFB((uint16_t *)frame->buf);
  }

  return true;
}

void Adafruit_PyCamera::blitFrame(void) {
  drawRGBBitmap(0, 0, (uint16_t *)fb->getBuffer(), 240, 240);
  
  esp_camera_fb_return(frame);
}


bool Adafruit_PyCamera::initAccel(void) {
  lis_dev = new Adafruit_I2CDevice(0x19, &Wire);
  if (!lis_dev->begin()) {
    return false;
  }
  Adafruit_BusIO_Register _chip_id = Adafruit_BusIO_Register(
      lis_dev,  LIS3DH_REG_WHOAMI, 1);
  if (_chip_id.read() != 0x33) {
    return false;
  }
  Adafruit_BusIO_Register _ctrl1 = Adafruit_BusIO_Register(
      lis_dev, LIS3DH_REG_CTRL1, 1);
  _ctrl1.write(0x07); // enable all axes, normal mode
  Adafruit_BusIO_RegisterBits data_rate_bits =
      Adafruit_BusIO_RegisterBits(&_ctrl1, 4, 4);
  data_rate_bits.write(0b0111); // set to 400Hz update

  Adafruit_BusIO_Register _ctrl4 = Adafruit_BusIO_Register(
      lis_dev, LIS3DH_REG_CTRL4, 1);
  _ctrl4.write(0x88); // High res & BDU enabled
  Adafruit_BusIO_RegisterBits range_bits =
      Adafruit_BusIO_RegisterBits(&_ctrl4, 2, 4);
  range_bits.write(0b11);

  Serial.println("Found LIS3DH");
  return true;
}


bool Adafruit_PyCamera::readAccelData(int16_t *x, int16_t *y, int16_t *z) {
  uint8_t register_address = LIS3DH_REG_OUT_X_L;
  register_address |= 0x80; // set [7] for auto-increment

  Adafruit_BusIO_Register xl_data = Adafruit_BusIO_Register(
      lis_dev, register_address, 6);

  uint8_t buffer[6];
  if (! xl_data.read(buffer, 6)) return false;

  *x = buffer[0];
  *x |= ((uint16_t)buffer[1]) << 8;
  *y = buffer[2];
  *y |= ((uint16_t)buffer[3]) << 8;
  *z = buffer[4];
  *z |= ((uint16_t)buffer[5]) << 8;

  return true;
}

bool Adafruit_PyCamera::readAccelData(float *x_g, float *y_g, float *z_g) {
  int16_t x, y, z;
  if (!readAccelData(&x, &y, &z)) return false;

  uint8_t lsb_value = 48; // for 16G
  *x_g = lsb_value * ((float)x / LIS3DH_LSB16_TO_KILO_LSB10);
  *y_g = lsb_value * ((float)y / LIS3DH_LSB16_TO_KILO_LSB10);
  *z_g = lsb_value * ((float)z / LIS3DH_LSB16_TO_KILO_LSB10);
  return true;
}

void Adafruit_PyCamera::I2Cscan(void) {
  Wire.begin();
  Serial.print("I2C Scan: ");
  for (int addr=0; addr<=0x7F; addr++) {
    Wire.beginTransmission(addr);
    bool found = (Wire.endTransmission() == 0);
    if (found) {
      Serial.print("0x");
      Serial.print(addr, HEX);
      Serial.print(", ");
    }
  }
  Serial.println();
}

void Adafruit_PyCamera::setNeopixel(uint32_t c) {
  pixel.fill(c);
  pixel.show(); // Initialize all pixels to 'off'
}

/**************************************************************************/
/*!
    @brief   Input a value 0 to 255 to get a color value. The colours are a
   transition r - g - b - back to r.
    @param  WheelPos The position in the wheel, from 0 to 255
    @returns  The 0xRRGGBB color
*/
/**************************************************************************/
uint32_t Adafruit_PyCamera::Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return Adafruit_NeoPixel::Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return Adafruit_NeoPixel::Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return Adafruit_NeoPixel::Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
