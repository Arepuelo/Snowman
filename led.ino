#include <Adafruit_NeoPixel.h>
#include "time.h"
#include "globals.h"

constexpr int LED_PIN   = 12;
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

  int secOfHour = t.tm_min * 60 + t.tm_sec;
  float pos = (secOfHour / 3600.0f) * LED_COUNT;  // 0–8
  int full = (int)pos;                           // full LEDs
  float frac = pos - full;                       // fraction for partial
  if (full >= LED_COUNT) full = LED_COUNT - 1;

  // toggle state once per second
  static int lastSec = -1;
  static bool blink = false;
  if (t.tm_sec != lastSec) {
    lastSec = t.tm_sec;
    blink = !blink;
  }

  for (int i = 0; i < LED_COUNT; i++) {
    uint8_t r = 0, g = 0;

    if (i < full) {          // solid completed
      r = 10; g = 180;
    }
    if (i == full) {         // ticking partial LED
      if (frac == 0) {       // on boundary → solid
        r = 10; g = 180;
      } else if (blink) {    // blink state
        r = 30; g = 255;
      } else {
        r = 5; g = 40;
      }
    }

    bar.setPixelColor(i, bar.Color(r, g, 0));
  }

  bar.show();
}
