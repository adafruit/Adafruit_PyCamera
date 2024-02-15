#include <Arduino.h>
#include "Adafruit_PyCamera.h"
#include <CatGFX.h>
// need to pull in main of CatGFX
// https://github.com/TheNitek/CatGFX

Adafruit_PyCamera pycamera;

// Buffer which can hold 400 lines
byte buffer[48 * 400] = {0};

// Create a printer supporting those 400 lines
CatPrinter cat(400);

// hard reset
void(* resetFunc) (void) = 0;

// atkinson dithering algorithm parameters from CircuitPython
struct DitherAlgorithmInfo {
    int mx, my;
    int divisor;
    struct { int dx, dy, dl; } terms[4];
} atkinson = {
    1, 1, 8, // mx, my, divisor
    {
        {1, 0, 1}, {-1, 1, 1}, {0, 1, 1}, {1, 1, 1} // dx, dy, dl
    }
};

// this sends a full bitmap to the printer

/*void gameboy(uint16_t *source, int width, int height, bool createForPrinter) {
    uint16_t *bitmap = new uint16_t[width * height];
    int16_t *errorBuffer = new int16_t[(width + 2) * (height + 2)]();
    uint8_t *bitmapForPrinter = nullptr;
    if (createForPrinter) {
        bitmapForPrinter = new uint8_t[(width * height)]();
    }
    // dither the pixels
    // referenced from CP module
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y + 1) * (width + 2) + (x + 1);
            uint16_t rgb = source[y * width + x];
            uint8_t r = (rgb & 0xF800) >> 8;
            uint8_t g = (rgb & 0x07E0) >> 3;
            uint8_t b = (rgb & 0x001F) << 3;
            uint8_t grayscale = (r * 30 + g * 59 + b * 11) / 100;
            grayscale += errorBuffer[idx] / atkinson.divisor;
            uint16_t outPixel = grayscale > 127 ? 0xFFFF : 0x0000; // White or Black for RGB bitmap
            bitmap[y * width + x] = outPixel;
            if (createForPrinter) {
                if (grayscale > 127) {
                    bitmapForPrinter[(y * width + x) / 8] |= (1 << (7 - (x % 8)));
                }
            }
            int16_t error = grayscale - (outPixel ? 255 : 0);
            for (int i = 0; i < 4; i++) {
                int dx = atkinson.terms[i].dx, dy = atkinson.terms[i].dy;
                errorBuffer[idx + dy * (width + 2) + dx] += error * atkinson.terms[i].dl;
            }
        }
    }
    if (createForPrinter == true) {
        cat.fillScreen(1);
        cat.drawBitmap(0, 0, bitmapForPrinter, 240, 240, 0);
        delay(500);
        Serial.println("Printing...");
        cat.printBuffer();
        delay(500);
        cat.fillScreen(1);
        delay(500);
        cat.fillBuffer(0x00);
        delay(2000);
        Serial.println("scrolling..");
        cat.feed(200);
        Serial.println("Done!");
        Serial.println("Disconnecting..");
        cat.disconnect();
        Serial.println("Disconnected.");
        resetFunc();
    } 
    pycamera.drawRGBBitmap(0, 0, bitmap, width, height);
    pycamera.refresh();
    delete[] bitmap;
    delete[] errorBuffer;
    delete[] bitmapForPrinter;
}*/

// this method uses drawPixel

void gameboy(uint16_t *source, int width, int height, bool createForPrinter) {
    uint16_t *bitmap = new uint16_t[width * height];
    int16_t *errorBuffer = new int16_t[(width + 2) * (height + 2)]();
    uint8_t *bitmapForPrinter = nullptr;
    if (createForPrinter) {
        // allocate memory for the printer bitmap, 8 bits
        bitmapForPrinter = new uint8_t[(width * height) / 8]();
    }
    // dither the pixels
    // referenced from CP module
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y + 1) * (width + 2) + (x + 1);
            uint16_t rgb = source[y * width + x];
            uint8_t r = (rgb & 0xF800) >> 8;
            uint8_t g = (rgb & 0x07E0) >> 3;
            uint8_t b = (rgb & 0x001F) << 3;
            uint8_t grayscale = (r * 30 + g * 59 + b * 11) / 100;
            grayscale += errorBuffer[idx] / atkinson.divisor;
            uint16_t outPixel = grayscale > 127 ? 0xFFFF : 0x0000; // White or Black for RGB bitmap
            bitmap[y * width + x] = outPixel;
            if (createForPrinter) {
                if (grayscale > 127) {
                    bitmapForPrinter[(y * width + x) / 8] |= (1 << (7 - (x % 8)));
                }
            }
            int16_t error = grayscale - (outPixel ? 255 : 0);
            for (int i = 0; i < 4; i++) {
                int dx = atkinson.terms[i].dx, dy = atkinson.terms[i].dy;
                errorBuffer[idx + dy * (width + 2) + dx] += error * atkinson.terms[i].dl;
            }
        }
    }
    // if printing
    if (createForPrinter) {
        // clear printer buffer
        cat.fillScreen(1);
        cat.setCursor(0, 24);
          // iterate bitmapForPrinter and use drawPixel for each bit
          for (int y = 0; y < height; y++) {
              for (int x = 0; x < width; x++) {
                  // get index in the bitmap array and the specific bit for pixel
                  int byteIndex = (y * width + x) / 8;
                  int bitIndex = x % 8;
                  bool pixel = bitmapForPrinter[byteIndex] & (1 << (7 - bitIndex));
                  // use drawPixel to draw the unpacked bit
                  cat.drawPixel(x, y, pixel ? 1 : 0);
              }
          }
          Serial.println("Printing...");
          cat.printBuffer();
          delay(200);
          cat.fillScreen(1);
          delay(200);
          Serial.println("scrolling..");
          cat.feed(200); // feed paper to show the print
          Serial.println("Done!");
          Serial.println("Disconnecting..");
          cat.disconnect();
          Serial.println("Disconnected.");
          resetFunc();
        }
    // show the dithered image
    pycamera.drawRGBBitmap(0, 0, bitmap, width, height);
    // calls esp_camera_fb_return(frame);
    pycamera.refresh();
    // delete the bitmaps
    delete[] bitmap;
    delete[] errorBuffer;
    delete[] bitmapForPrinter;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println();
  Serial.setDebugOutput(true);
  Serial.println("Hello world");
  if (!pycamera.begin()) {
    Serial.println("Failed to initialize pyCamera interface");
    while (1)
      yield();
  }
  Serial.println("pyCamera hardware initialized!");
  cat.begin(buffer, sizeof(buffer));
  cat.printNameArray();
  cat.resetNameArray();
  cat.addNameArray((char *)"MX06");
  if (cat.connect()) {
    Serial.println("Connected!");
    }
    else {
    Serial.println("Could not find printer!");
    resetFunc();
  }
  Serial.println("going into loop");
}

void loop() {
  pycamera.readButtons();
  pycamera.captureFrame();
  Serial.println("converting");
  // blits gameboy filter to display but does not print
   gameboy((uint16_t *)pycamera.fb->getBuffer(), 240, 240, false);
  // pressing okay prints the frame
  if (pycamera.justPressed(AWEXP_BUTTON_OK)) {
    gameboy((uint16_t *)pycamera.fb->getBuffer(), 240, 240, true);
    Serial.println("OKAY");
  }

  if (pycamera.justPressed(AWEXP_BUTTON_SEL)) {
    cat_scroll();
    Serial.println("SEL");
  }
  if (pycamera.justPressed(AWEXP_BUTTON_DOWN)){
    cat_reconnect();
    Serial.println("DOWN");
  }
}

void cat_scroll() {
  Serial.println("scrolling paper..");
  cat.feed(100);
}

void cat_reconnect() {
  Serial.println("Disconnecting..");
  cat.disconnect();
  Serial.println("Disconnected.");
  delay(1000);
  cat.resetNameArray();
  cat.addNameArray((char *)"MX06");
  if (cat.connect()) {
    Serial.println("Connected!");
  }
    else {
    Serial.println("Could not find printer!");
  }
  delay(500);
}
