#include <Arduino.h>
#include "time.h"

// ========= CONFIG =========

constexpr int BUZZER_PIN        = 10;
constexpr unsigned long ON_MS   = 120;   // ms buzzer "ON" (LOW)
constexpr unsigned long OFF_MS  = 120;   // ms between beeps

bool timeGet(struct tm *out);

// ========= INTERNAL STATE =========

static bool          seqActive    = false;
static bool          buzzerOn     = false;
static int           targetBeeps  = 0;
static int           beepsDone    = 0;
static unsigned long lastChangeMs = 0;
static int           lastHourSeen = -1;  // hour already beeped

// ========= HELPERS =========

// Convert 24h → 1–12 beeps
static int hourToBeeps(int hour24) {
  int h = hour24 % 12;
  if (h == 0) h = 12;
  return h;
}

static void quietBuzzer() {
  // LOW-triggered: HIGH = silent
  digitalWrite(BUZZER_PIN, HIGH);
}

static void buzzerOnPulse() {
  // LOW-triggered: LOW = sound
  digitalWrite(BUZZER_PIN, LOW);
}

static void startHourSequence(int hour24) {
  targetBeeps  = hourToBeeps(hour24);
  beepsDone    = 0;
  buzzerOn     = false;
  seqActive    = true;
  lastChangeMs = millis();
  quietBuzzer();
}

static void updateSequence() {
  if (!seqActive) return;

  unsigned long now = millis();

  if (!buzzerOn) {
    // currently OFF → maybe turn ON
    if (now - lastChangeMs >= OFF_MS) {
      buzzerOnPulse();
      buzzerOn     = true;
      lastChangeMs = now;
    }
  } else {
    // currently ON → maybe turn OFF and count one beep
    if (now - lastChangeMs >= ON_MS) {
      quietBuzzer();
      buzzerOn     = false;
      lastChangeMs = now;

      beepsDone++;
      if (beepsDone >= targetBeeps) {
        seqActive = false;   // finished sequence
      }
    }
  }
}

// ========= PUBLIC API =========

void initBuzzer() {
  pinMode(BUZZER_PIN, OUTPUT);
  quietBuzzer();            // make sure it's silent at boot
}

void updateBuzzer() {
  // Continue any current beep sequence
  updateSequence();
  if (seqActive) return;

  struct tm t;
  if (!timeGet(&t)) {
    return;  // no valid time, stay quiet
  }

  int h = t.tm_hour;

  // Beep whenever the hour value changes
  if (h != lastHourSeen) {
    startHourSequence(h);   // beeps hourToBeeps(h) times
    lastHourSeen = h;
  }
}

