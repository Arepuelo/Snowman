#include "globals.h"
#include "time.h"


// Time provider (real or debug), implemented in time_core.ino
bool timeGet(struct tm *timeinfo);

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


// ---------- Drawing helpers ----------

void drawAnalogClock(struct tm *timeinfo) {
  // ---- THEME SELECTION (simple, inside the function) ----
  int h = timeinfo->tm_hour;

  uint16_t bg;
  uint16_t circle;
  uint16_t text;
  uint16_t hourCol;
  uint16_t minCol;
  uint16_t secCol = ST77XX_RED;  // always red

  if (h >= 5 && h < 12) {
    // MORNING
    bg       = 0xA73F;        // light sky
    circle   = ST77XX_BLACK;
    text     = ST77XX_WHITE;
    hourCol  = ST77XX_BLACK;
    minCol   = ST77XX_BLACK;

  } else if (h >= 12 && h < 18) {
    // AFTERNOON
    bg       = 0xFD20;        // warm yellow
    circle   = ST77XX_BLACK;
    text     = ST77XX_BLACK;
    hourCol  = ST77XX_BLACK;
    minCol   = ST77XX_BLACK;

  } else {
    // EVENING
    bg       = 0x280b;        // dark navy
    circle   = ST77XX_WHITE;
    text     = 0xfda0;
    hourCol  = ST77XX_WHITE;
    minCol   = ST77XX_WHITE;
  }

  // ---- ORIGINAL CODE BELOW ----

  canvas.fillScreen(bg);

  // outline
  canvas.drawCircle(CENTER_X, CENTER_Y, CLOCK_R, circle);

  // 12, 3, 6, 9
  canvas.setTextColor(text);
  canvas.setTextSize(2);

  canvas.setCursor(CENTER_X - 10, CENTER_Y - CLOCK_R + 8);
  canvas.print("12");
  canvas.setCursor(CENTER_X + CLOCK_R - 28, CENTER_Y - 8);
  canvas.print("3");
  canvas.setCursor(CENTER_X - 6, CENTER_Y + CLOCK_R - 22);
  canvas.print("6");
  canvas.setCursor(CENTER_X - CLOCK_R + 10, CENTER_Y - 8);
  canvas.print("9");

  float hf = timeinfo->tm_hour % 12;
  float mf = timeinfo->tm_min;
  float sf = timeinfo->tm_sec;

  float angleH = (hf + mf / 60.0) * 30.0; // 360/12
  float angleM = mf * 6.0;               // 360/60
  float angleS = sf * 6.0;

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

  // hour & minute: themed, second: themed red
  canvas.drawLine(CENTER_X, CENTER_Y, hx, hy, hourCol);
  canvas.drawLine(CENTER_X, CENTER_Y, mx, my, minCol);
  canvas.drawLine(CENTER_X, CENTER_Y, sx, sy, secCol);
}

void drawDigitalClock(struct tm *timeinfo) {
  // ---- THEME SELECTION ----
  int h = timeinfo->tm_hour;

  uint16_t bg;
  uint16_t text;

  if (h >= 5 && h < 12) {
    // MORNING
    bg   = 0xA73F;        // light sky
    text = ST77XX_WHITE;

  } else if (h >= 12 && h < 18) {
    // AFTERNOON
    bg   = 0xFD20;        // warm yellow
    text = ST77XX_BLACK;

  } else {
    // EVENING
    bg   = 0x280b;        // dark navy
    text = 0xfda0;
  }

  // ---- ORIGINAL CODE BELOW ----

  canvas.fillScreen(bg);

  // ----- TIME (12-hour format + AM/PM) -----
  int hour12 = timeinfo->tm_hour % 12;
  if (hour12 == 0) hour12 = 12;  // 00 → 12

  char timeBuf[20];
  sprintf(timeBuf, "%02d:%02d:%02d %s",
          hour12,
          timeinfo->tm_min,
          timeinfo->tm_sec,
          (timeinfo->tm_hour < 12 ? "AM" : "PM"));

  canvas.setTextColor(text);
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
  mode = MODE_ANALOG;   // start in analog (or whatever you prefer)
  initButton();         // button lives in button.ino
}

void updateClockLogic() {
  // --- Button handling (single / double / long press) ---
  unsigned long nowMillis = millis();
  updateButton(nowMillis);

  if (buttonSingleClick()) {
    // Toggle analog/digital mode
    mode = (mode == MODE_ANALOG ? MODE_DIGITAL : MODE_ANALOG);
  }

  if (buttonDoubleClick()) {
    // TODO: cycle Morning / Afternoon / Evening theme
    // nextTheme();
  }

  if (buttonLongPress()) {
    // TODO: toggle AUTO / MANUAL theme mode
    // toggleThemeSource();
  }

  // --- Time + drawing (same as before) ---
  struct tm timeinfo;
  if (!timeGet(&timeinfo)) {
    // if no time yet, just return
    return;
  }

  // ---- tiny change: draw functions now include theme internally ----
  if (mode == MODE_ANALOG) {
    drawAnalogClock(&timeinfo);
  } else {
    drawDigitalClock(&timeinfo);
  }

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), SW, SH);

  delay(50); // ~20 FPS
}
