#include <Arduino.h>
#include "time.h"

// ========= CONFIG =========

// Your buzzer pin:
constexpr int BUZZER_PIN        = 10;

// Active buzzer (LOW = sound, HIGH = silent)
constexpr unsigned long ON_MS   = 120;   // ms buzzer ON
constexpr unsigned long OFF_MS  = 120;   // ms between beeps

// time_core.ino:
bool timeGet(struct tm *out);

// ========= INTERNAL STATE =========

static bool          seqActive        = false;
static bool          buzzerOn         = false;
static int           targetBeeps      = 0;
static int           beepsDone        = 0;
static unsigned long lastChangeMs     = 0;

static bool          initialized      = false;
static int           lastHourChimed   = -1;  // last hour we did hh:00 chime
static int           lastHalfChimed   = -1;  // last hour we did hh:30 chime

// ========= HELPERS =========

// 24h → 1..12 beeps
static int hourToBeeps(int hour24) {
  int h = hour24 % 12;
  if (h == 0) h = 12;
  return h;
}

static void quietBuzzer() {
  // LOW-triggered: HIGH = silent
  digitalWrite(BUZZER_PIN, HIGH);
  // If your buzzer is HIGH-triggered, swap HIGH/LOW in this function and in buzzerOnPulse()
}

static void buzzerOnPulse() {
  // LOW-triggered: LOW = sound
  digitalWrite(BUZZER_PIN, LOW);
}

static void startSequence(int beeps) {
  targetBeeps  = beeps;
  beepsDone    = 0;
  buzzerOn     = false;
  seqActive    = true;
  lastChangeMs = millis();
  quietBuzzer();
}

static void startHourSequence(int hour24) {
  startSequence(hourToBeeps(hour24));
}

static void startHalfHourBeep() {
  startSequence(1);    // just one beep at :30
}

static void updateSequence() {
  if (!seqActive) return;

  unsigned long now = millis();

  if (!buzzerOn) {
    // OFF → maybe turn ON
    if (now - lastChangeMs >= OFF_MS) {
      buzzerOnPulse();
      buzzerOn     = true;
      lastChangeMs = now;
    }
  } else {
    // ON → maybe turn OFF and count one beep
    if (now - lastChangeMs >= ON_MS) {
      quietBuzzer();
      buzzerOn     = false;
      lastChangeMs = now;

      beepsDone++;
      if (beepsDone >= targetBeeps) {
        seqActive = false;
      }
    }
  }
}

// ========= PUBLIC API =========

void initBuzzer() {
  pinMode(BUZZER_PIN, OUTPUT);
  quietBuzzer();
}

void updateBuzzer() {
  // keep any running sequence going
  updateSequence();
  if (seqActive) return;

  struct tm t;
  if (!timeGet(&t)) {
    return; // no valid time → stay quiet
  }

  int h = t.tm_hour;
  int m = t.tm_min;
  int s = t.tm_sec;

  // First call: sync state so it doesn't beep immediately on power-on
  if (!initialized) {
    initialized = true;
    lastHourChimed = h;
    // If we've already passed :30 in this hour, mark half-hour as done too
    if (m > 30 || (m == 30 && s >= 3)) {
      lastHalfChimed = h;
    }
    return;
  }

  // ----- FULL HOUR: hh:00:xx → N beeps -----
  if (m == 0 && s < 3 && h != lastHourChimed) {
    startHourSequence(h);
    lastHourChimed = h;
    lastHalfChimed = -1;  // reset half-hour chime for this new hour
    return;
  }

  // ----- HALF HOUR: hh:30:xx → 1 beep -----
  if (m == 30 && s < 3 && h != lastHalfChimed) {
    startHalfHourBeep();
    lastHalfChimed = h;
    return;
  }
}
