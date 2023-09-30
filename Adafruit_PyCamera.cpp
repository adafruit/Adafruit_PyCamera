#include "Adafruit_PyCamera.h"

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
  if (! initCamera()) return false;  // NOTE esp-camera deinits i2c!
 // if (! initExpander()) return false;
 // if (! initSeesaw()) return false;

 // initSD();

  _timestamp = millis();

  return true;
}


bool Adafruit_PyCamera::initSD(void) {

  if (! SDdetected()) {
    Serial.println("No SD card inserted");
    return false;
  }

  Serial.println("SD card inserted, trying to init");
    
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
  aw.digitalWrite(AWEXP_SD_PWR, HIGH); // start off  
  aw.pinMode(AWEXP_SD_DET, INPUT);

  aw.pinMode(AWEXP_CAM_PWDN, OUTPUT);
  aw.digitalWrite(AWEXP_CAM_PWDN, HIGH); // camera off
  aw.pinMode(AWEXP_CAM_RST, OUTPUT);
  aw.digitalWrite(AWEXP_CAM_RST, LOW); // camera in reset
  delay(10);
  aw.digitalWrite(AWEXP_CAM_PWDN, LOW);
  delay(10);
  aw.digitalWrite(AWEXP_CAM_RST, HIGH);
  delay(10);

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


bool Adafruit_PyCamera::initCamera(void) {
  Serial.print("Config camera...");

  aw.pinMode(AWEXP_CAM_PWDN, OUTPUT);
  aw.pinMode(AWEXP_CAM_RST, OUTPUT);
  aw.digitalWrite(AWEXP_CAM_RST, LOW);
  aw.digitalWrite(AWEXP_CAM_PWDN, HIGH);
  delay(10);
  aw.digitalWrite(AWEXP_CAM_PWDN, LOW);
  delay(10);
  aw.digitalWrite(AWEXP_CAM_RST, HIGH);
  delay(10);

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
  camera_config.jpeg_quality = 12;
  camera_config.fb_count = 1;

  Serial.print("Config format...");
  //camera_config.frame_size = FRAMESIZE_UXGA;
  camera_config.pixel_format = PIXFORMAT_RGB565;
  camera_config.frame_size = FRAMESIZE_240X240;
  camera_config.fb_count = 1;

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
  return true;
}



float Adafruit_PyCamera::readBatteryVoltage(void) {
  return 0; //ss.analogRead(AW_BATTMON) * 2.0 * 3.3 / 1024.0;
}

bool Adafruit_PyCamera::SDdetected(void) {
  return aw.digitalRead(AWEXP_SD_DET);
}

uint32_t Adafruit_PyCamera::readButtons(void) {
  last_button_state = button_state;
  //button_state = aw.digitalReadBulk(AW_INPUTS_MASK);
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
  aw.digitalWrite(AWEXP_SD_DET, HIGH);
  uint16_t udelay = 1000000 / tonefreq;
  uint32_t count = (tonetime * 1000) / (2 * udelay);
  //tone(45, tonefreq, tonetime);
  //delay(tonetime);
  for (int i=0; i<count; i++) {
    digitalWrite(45, HIGH);
    delayMicroseconds(udelay);
    digitalWrite(45, LOW);
    delayMicroseconds(udelay);
  }
  aw.digitalWrite(AWEXP_SD_DET, LOW);
}


uint32_t Adafruit_PyCamera::timestamp(void) {
  uint32_t delta = millis() - _timestamp;
  _timestamp = millis();
  return delta;
}

void Adafruit_PyCamera::timestampPrint(const char *msg) {
  Serial.printf("%s: %d ms elapsed\n\r", msg, timestamp());
}

bool Adafruit_PyCamera::captureAndBlit(void) {
    //Serial.println("Capturing...");

    camera_fb_t *frame = NULL;
    esp_err_t res = ESP_OK;
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
    int64_t fr_start = esp_timer_get_time();
#endif

    frame = esp_camera_fb_get();
    
    if (!frame) {
      ESP_LOGE(TAG, "Camera frame capture failed");
      return false;
    }
    
    Serial.printf("\t\t\tFramed %d bytes (%d x %d) in %d ms\n\r", 
                  frame->len, 
                  frame->width, frame->height, 
                  timestamp());

    // flip endians
    uint8_t temp;
    for (uint32_t i=0; i<frame->len; i+=2) {
      temp = frame->buf[i+0];
      frame->buf[i+0] = frame->buf[i+1];
      frame->buf[i+1] = temp;
    }
    drawRGBBitmap(0, 0, (uint16_t *)frame->buf, 240, 240);

    esp_camera_fb_return(frame);
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
