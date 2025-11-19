#include <Adafruit_NeoPixel.h>
#include "time.h"
#include "globals.h"

constexpr int LED_PIN   = 18;
constexpr int LED_COUNT = 8;

bool timeGet(struct tm *out);

Adafruit_NeoPixel bar(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void initLeds() {
  bar.begin();
  bar.setBrightness(40);
  bar.show();
}

void updateLeds() {
  struct tm t;
  if (!timeGet(&t)) {
    for (int i = 0; i < LED_COUNT; i++)
      bar.setPixelColor(i, bar.Color(150, 0, 0));
    bar.show();
    return;
  }

  int sec = t.tm_sec;                       // 0–59
  float pos = (sec / 60.0f) * LED_COUNT;    // map minute progress to 0–8
  int full = (int)pos;
  float frac = pos - full;
  if (full >= LED_COUNT) full = LED_COUNT - 1;

  // second tick blink
  static int lastSec = -1;
  static bool blink = false;
  if (sec != lastSec) {
    lastSec = sec;
    blink = !blink;
  }

  for (int i = 0; i < LED_COUNT; i++) {
    uint8_t r = 0, g = 0;

    if (i < full) {
      r = 180; g = 110;            // filled LEDs
    }
    if (i == full) {
      if (frac == 0) {            // exact boundary → solid
        r = 180; g = 110;
      } else if (blink) {         // ticking partial LED
        r = 255; g = 125;
      } else {
        r = 40; g = 20;
      }
    }

    bar.setPixelColor(i, bar.Color(r, g, 0));
  }

  bar.show();
}
