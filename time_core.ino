#include <Arduino.h>
#include "time.h"

// ============= INTERNAL STATE =============

static bool      useDebugTime = false;
static struct tm debugTime    = {};
static unsigned long lastMs   = 0;

// ============= INTERNAL HELPERS =============

// Keep debug time in [00:00:00, 23:59:59]
static void normalizeDebugTime() {
  long total = debugTime.tm_hour * 3600L +
               debugTime.tm_min  * 60L +
               debugTime.tm_sec;

  long day = 24L * 3600L;
  total %= day;
  if (total < 0) total += day;

  debugTime.tm_hour = (total / 3600L) % 24;
  debugTime.tm_min  = (total / 60L)   % 60;
  debugTime.tm_sec  = total % 60;
}

// Move debug time forward by deltaSeconds
static void advanceDebugSeconds(long deltaSeconds) {
  long total = debugTime.tm_hour * 3600L +
               debugTime.tm_min  * 60L +
               debugTime.tm_sec  +
               deltaSeconds;

  long day = 24L * 3600L;
  total %= day;
  if (total < 0) total += day;

  debugTime.tm_hour = (total / 3600L) % 24;
  debugTime.tm_min  = (total / 60L)   % 60;
  debugTime.tm_sec  = total % 60;
}

// Use millis() to tick the debug clock
static void updateDebugTimeFromMillis() {
  unsigned long now  = millis();
  unsigned long diff = now - lastMs;

  if (diff >= 1000UL) {
    long deltaSeconds = diff / 1000UL;
    lastMs += (unsigned long)deltaSeconds * 1000UL;
    advanceDebugSeconds(deltaSeconds);
  }
}

// ============= PUBLIC API =============

// Call from setup(), after Wi-Fi + NTP are configured.
void timeBegin(bool startInDebug) {
  useDebugTime = startInDebug;

  if (useDebugTime) {
    struct tm realNow;
    if (getLocalTime(&realNow)) {
      debugTime = realNow;
    } else {
      memset(&debugTime, 0, sizeof(debugTime));
      debugTime.tm_hour = 12;
      debugTime.tm_min  = 0;
      debugTime.tm_sec  = 0;
    }
    normalizeDebugTime();
    lastMs = millis();

    Serial.println(F("[time] Debug mode ON (boot)"));
  } else {
    Serial.println(F("[time] Debug mode OFF (real NTP time)"));
  }
}

// Enable/disable debug mode at runtime.
void timeSetDebug(bool enable) {
  if (enable == useDebugTime) return;

  useDebugTime = enable;

  if (useDebugTime) {
    struct tm realNow;
    if (getLocalTime(&realNow)) {
      debugTime = realNow;
    } else {
      memset(&debugTime, 0, sizeof(debugTime));
      debugTime.tm_hour = 12;
      debugTime.tm_min  = 0;
      debugTime.tm_sec  = 0;
    }
    normalizeDebugTime();
    lastMs = millis();
    Serial.println(F("[time] Debug mode ON"));
  } else {
    Serial.println(F("[time] Debug mode OFF (back to NTP)"));
  }
}

// Set fake time explicitly in debug mode (24h format).
void timeSetDebugTime(int hour24, int minute, int second) {
  debugTime.tm_hour = hour24;
  debugTime.tm_min  = minute;
  debugTime.tm_sec  = second;
  normalizeDebugTime();
  lastMs = millis();

  Serial.print(F("[time] Set debug time to "));
  Serial.print(debugTime.tm_hour);
  Serial.print(':');
  Serial.print(debugTime.tm_min);
  Serial.print(':');
  Serial.println(debugTime.tm_sec);
}

// Main function for the rest of the project to get the time.
bool timeGet(struct tm *out) {
  if (!out) return false;

  if (!useDebugTime) {
    // Normal mode: use NTP/system time.
    return getLocalTime(out);
  }

  // Debug mode: use fake time that ticks with millis().
  updateDebugTimeFromMillis();
  *out = debugTime;
  return true;
}

// ============= SERIAL DEBUG INTERFACE =============
//
// Commands (type in Serial Monitor, end with Enter):
//
//   D 1        -> debug ON
//   D 0        -> debug OFF (back to NTP)
//   S hh mm ss -> set debug time (24h)
//   J seconds  -> jump seconds (can be negative, e.g. J -60)
//   T          -> print current mode + time
//

void timeDebugUpdate() {
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  char cmd = toupper(line.charAt(0));
  const char *rest = line.c_str() + 1;

  switch (cmd) {
    case 'D': {
      int val;
      if (sscanf(rest, "%d", &val) == 1) {
        timeSetDebug(val != 0);
      } else {
        Serial.println(F("[time] Usage: D 1 (on) or D 0 (off)"));
      }
      break;
    }

    case 'S': {
      int h, m, s;
      if (sscanf(rest, "%d %d %d", &h, &m, &s) == 3) {
        if (!useDebugTime) timeSetDebug(true);
        timeSetDebugTime(h, m, s);
      } else {
        Serial.println(F("[time] Usage: S hh mm ss"));
      }
      break;
    }

    case 'J': {
      long delta;
      if (sscanf(rest, "%ld", &delta) == 1) {
        if (!useDebugTime) timeSetDebug(true);
        advanceDebugSeconds(delta);
        normalizeDebugTime();
        lastMs = millis();

        Serial.print(F("[time] Jumped debug time by "));
        Serial.print(delta);
        Serial.println(F(" seconds"));
      } else {
        Serial.println(F("[time] Usage: J seconds"));
      }
      break;
    }

    case 'T': {
      struct tm now;
      if (timeGet(&now)) {
        Serial.print(F("[time] Mode: "));
        Serial.println(useDebugTime ? F("DEBUG") : F("REAL"));
        Serial.print(F("       Time: "));
        Serial.print(now.tm_hour);
        Serial.print(':');
        Serial.print(now.tm_min);
        Serial.print(':');
        Serial.println(now.tm_sec);
      } else {
        Serial.println(F("[time] No time available"));
      }
      break;
    }

    default:
      Serial.println(F("[time] Commands: D,S,J,T"));
      break;
  }
}
