#include "globals.h"
#include "time.h"

// Screen size
constexpr int SW = 240;
constexpr int SH = 240;

// Clock layout
constexpr int CENTER_X = SW / 2;
constexpr int CENTER_Y = SH / 2;
constexpr int CLOCK_R  = 100;

// Mode enum
enum ClockMode { MODE_ANALOG, MODE_DIGITAL };
ClockMode mode = MODE_ANALOG;

// Button to toggle mode
constexpr int BUTTON_PIN = 15;
bool lastButton = HIGH;

// ---------- Drawing helpers ----------

void drawAnalogClock(struct tm *timeinfo) {
  canvas.fillScreen(ST77XX_BLACK);

  // outline
  canvas.drawCircle(CENTER_X, CENTER_Y, CLOCK_R, ST77XX_WHITE);

  // 12, 3, 6, 9
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(2);

  canvas.setCursor(CENTER_X - 10, CENTER_Y - CLOCK_R + 8);
  canvas.print("12");
  canvas.setCursor(CENTER_X + CLOCK_R - 28, CENTER_Y - 8);
  canvas.print("3");
  canvas.setCursor(CENTER_X - 6, CENTER_Y + CLOCK_R - 22);
  canvas.print("6");
  canvas.setCursor(CENTER_X - CLOCK_R + 10, CENTER_Y - 8);
  canvas.print("9");

  float h = timeinfo->tm_hour % 12;
  float m = timeinfo->tm_min;
  float s = timeinfo->tm_sec;

  float angleH = (h + m / 60.0) * 30.0; // 360/12
  float angleM = m * 6.0;               // 360/60
  float angleS = s * 6.0;

  // degrees -> radians, rotate so 0° is at 12 o'clock
  float ah = (angleH - 90.0) * PI / 180.0;
  float am = (angleM - 90.0) * PI / 180.0;
  float as = (angleS - 90.0) * PI / 180.0;

  int hx = CENTER_X + cos(ah) * (CLOCK_R * 0.5);
  int hy = CENTER_Y + sin(ah) * (CLOCK_R * 0.5);

  int mx = CENTER_X + cos(am) * (CLOCK_R * 0.75);
  int my = CENTER_Y + sin(am) * (CLOCK_R * 0.75);

  int sx = CENTER_X + cos(as) * (CLOCK_R * 0.85);
  int sy = CENTER_Y + sin(as) * (CLOCK_R * 0.85);

  // hour & minute: white, second: red
  canvas.drawLine(CENTER_X, CENTER_Y, hx, hy, ST77XX_WHITE);
  canvas.drawLine(CENTER_X, CENTER_Y, mx, my, ST77XX_WHITE);
  canvas.drawLine(CENTER_X, CENTER_Y, sx, sy, ST77XX_RED);
}

void drawDigitalClock(struct tm *timeinfo) {
  canvas.fillScreen(ST77XX_BLACK);

  // ----- TIME (12-hour format + AM/PM) -----
  int hour12 = timeinfo->tm_hour % 12;
  if (hour12 == 0) hour12 = 12;  // 00 → 12

  char timeBuf[20];
  sprintf(timeBuf, "%02d:%02d:%02d %s",
          hour12,
          timeinfo->tm_min,
          timeinfo->tm_sec,
          (timeinfo->tm_hour < 12 ? "AM" : "PM"));

  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(3);
  canvas.setCursor(10, 70);   // moved slightly up
  canvas.print(timeBuf);

  // ----- DATE (dd/mm/yy) -----
  char dateBuf[20];
  sprintf(dateBuf, "%02d/%02d/%02d",
          timeinfo->tm_mday,
          timeinfo->tm_mon + 1,           // tm_mon = 0–11
          timeinfo->tm_year % 100);       // tm_year = 1900-based

  canvas.setTextSize(3);
  canvas.setCursor(50, 140);  // center-ish
  canvas.print(dateBuf);
}


// ---------- Public init / update ----------

void initClockLogic() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void updateClockLogic() {
  // Toggle analog/digital on button press
  bool b = digitalRead(BUTTON_PIN);
  if (!b && lastButton) {
    mode = (mode == MODE_ANALOG ? MODE_DIGITAL : MODE_ANALOG);
  }
  lastButton = b;

  // Get current time from NTP (set by WiFi)
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // if no time yet, just return
    return;
  }

  // Draw into canvas
  if (mode == MODE_ANALOG) {
    drawAnalogClock(&timeinfo);
  } else {
    drawDigitalClock(&timeinfo);
  }

  // Push to TFT
  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), SW, SH);

  delay(50); // ~20 FPS
}
