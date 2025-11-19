#include <Arduino.h>

// ================== CONFIG ==================
const int BUTTON_PIN = 15;        // Change if needed (active LOW)

// Timings (ms)
static const unsigned long LONG_PRESS_MS = 700;
static const unsigned long DOUBLE_GAP_MS = 200;

// ================== INTERNAL STATE ==================
static bool lastRawState    = false;  // previous raw read (NO debounce)
static unsigned long pressStartTime   = 0;
static unsigned long lastReleaseTime  = 0;

static bool longPressFired = false;
static uint8_t clickCount  = 0;

// event bitmask
// bit 0 -> single click
// bit 1 -> double click
// bit 2 -> long press
static uint8_t eventMask = 0;

// Active LOW button
static inline bool readButtonPressed() {
  return digitalRead(BUTTON_PIN) == LOW;
}

// ================== PUBLIC API ==================

void initButton() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lastRawState = readButtonPressed();
  pressStartTime  = 0;
  lastReleaseTime = 0;
  longPressFired  = false;
  clickCount      = 0;
  eventMask       = 0;
}

void updateButton(unsigned long nowMillis) {
  bool rawState = readButtonPressed();

  // ----- RELEASED → PRESSED -----
  if (!lastRawState && rawState) {
    pressStartTime = nowMillis;
    longPressFired = false;
  }

  // ----- PRESSED (check long press) -----
  if (rawState && !lastRawState) {
    // nothing; handled above
  }

  if (rawState && !longPressFired) {
    if (nowMillis - pressStartTime >= LONG_PRESS_MS) {
      longPressFired = true;
      clickCount = 0;
      eventMask |= 0x04;  // long press
    }
  }

  // ----- PRESSED → RELEASED -----
  if (lastRawState && !rawState) {
    unsigned long pressDuration = nowMillis - pressStartTime;

    if (!longPressFired) {
      // SHORT press candidate
      clickCount++;

      if (clickCount == 1) {
        lastReleaseTime = nowMillis; // wait to see if double-click
      }
      else if (clickCount == 2) {
        if ((nowMillis - lastReleaseTime) <= DOUBLE_GAP_MS) {
          eventMask |= 0x02;  // double click
          clickCount = 0;
        } else {
          // too late → treat previous press as single click
          eventMask |= 0x01;
          clickCount = 1;
          lastReleaseTime = nowMillis;
        }
      }
    }
    // if longPressFired → ignore, no click
  }

  // ----- Single click finalization -----
  if (!rawState && clickCount == 1) {
    if (nowMillis - lastReleaseTime > DOUBLE_GAP_MS) {
      eventMask |= 0x01;  // single click
      clickCount = 0;
    }
  }

  lastRawState = rawState;
}

// ================== EVENT GETTERS ==================

bool buttonSingleClick() {
  if (eventMask & 0x01) {
    eventMask &= ~0x01;
    return true;
  }
  return false;
}

bool buttonDoubleClick() {
  if (eventMask & 0x02) {
    eventMask &= ~0x02;
    return true;
  }
  return false;
}

bool buttonLongPress() {
  if (eventMask & 0x04) {
    eventMask &= ~0x04;
    return true;
  }
  return false;
}
