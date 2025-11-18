#include <SPI.h>
#include "globals.h"

// === TFT PINS (your working ones) ===
#define TFT_DC    16
#define TFT_RST    4
#define TFT_CS    -1      // no CS pin on the display
#define TFT_SCLK   5
#define TFT_MOSI   6

// === Global objects (definitions) ===
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(240, 240);

void initScreen() {
  // Use your working SPI config
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.init(240, 240, SPI_MODE3);  // MODE3 was what made it work
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);
}
