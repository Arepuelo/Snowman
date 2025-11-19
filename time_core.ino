#include <Arduino.h>
#include "time.h"

// ================== INTERNAL STATE ==================

static bool      snowmanUseDebugTime = false;
static struct tm snowmanDebugTime    = {};
static unsigned long snowmanLastMs   = 0;

// ================== INTERNAL HELPERS ==================

static void normalizeDebugTime() {
  // Keep only time-of-day correct (0–23:59:59)
  long total = snowmanDebugTime.tm_hour * 3600L +
               snowmanDebugTime.tm_min  * 60L +
               snowmanDebugTime.tm_sec;

  // Allow negative deltas too
  long day = 24L * 3600L;
  total %= day;
  if (total < 0) total += day;

  snowmanDebugTime.tm_hour = (total / 3600L) % 24;
  snowmanDebugTime.tm_min  = (total / 60L)   % 60;
  snowmanDebugTime.tm_sec  = total % 60;
}

static void advanceDebugSeconds(long deltaSeconds) {
  long total = snowmanDebugTime.tm_hour * 3600L +
               snowmanDebugTime.tm_min  * 60L +
               snowmanDebugTime.tm_sec  +
               deltaSeconds;

  long day = 24L * 3600L;
  total %= day;
  if (total < 0) total += day;

  snowmanDebugTime.tm_hour = (total / 3600L) % 24;
  snowmanDebugTime.tm_min  = (total / 60L)   % 60;
  snowmanDebugTime.tm_sec  = total % 60;
}

static void updateDebugTimeFromMillis() {
  unsigned long now = millis();
  unsigned long diff = now - snowmanLastMs;

  if (diff >= 1000UL) {
    long deltaSeconds = diff / 1000UL;
    snowmanLastMs += (unsigned long)deltaSeconds * 1000UL;
    advanceDebugSeconds(deltaSeconds);
  }
}

// ================== PUBLIC API ==================

// Call from setup(), after Wi-Fi + NTP are configured.
void snowmanTimeBegin(bool startInDebug) {
  snowmanUseDebugTime = startInDebug;

  if (snowmanUseDebugTime) {
    // Start debug time from current real time if available,
    // otherwise fall back to 12:00:00.
    struct tm realNow;
    if (getLocalTime(&realNow)) {
      snowmanDebugTime = realNow;
    } else {
      memset(&snowmanDebugTime, 0, sizeof(snowmanDebugTime));
      snowmanDebugTime.tm_hour = 12;
      snowmanDebugTime.tm_min  = 0;
      snowmanDebugTime.tm_sec  = 0;
    }
    normalizeDebugTime();
    snowmanLastMs = millis();
  }
}

// Toggle between real time (NTP) and debug time.
void snowmanTimeSetDebug(bool enableDebug) {
  if (enableDebug == snowmanUseDebugTime) return;

  snowmanUseDebugTime = enableDebug;

  if (snowmanUseDebugTime) {
    // When entering debug mode, capture current real time if possible
    struct tm realNow;
    if (getLocalTime(&realNow)) {
      snowmanDebugTime = realNow;
    }
    normalizeDebugTime();
    snowmanLastMs = millis();
  }
}

// Set the fake time explicitly in 24-hour format.
void snowmanTimeSetDebugTime(int hour24, int minute, int second) {
  snowmanDebugTime.tm_hour = hour24;
  snowmanDebugTime.tm_min  = minute;
  snowmanDebugTime.tm_sec  = second;
  normalizeDebugTime();
  snowmanLastMs = millis();
}

// Manually jump the fake time by some seconds (positive or negative).
void snowmanTimeJumpDebugSeconds(long deltaSeconds) {
  advanceDebugSeconds(deltaSeconds);
  snowmanLastMs = millis();
}

// Main function everyone else should use to get the time.
bool snowmanTimeGet(struct tm *out) {
  if (!out) return false;

  if (!snowmanUseDebugTime) {
    // Normal mode: just use NTP/system time.
    return getLocalTime(out);
  }

  // Debug mode: advance from millis and return fake time.
  updateDebugTimeFromMillis();
  *out = snowmanDebugTime;
  return true;
}
