#include "globals.h"
#include <time.h>

const int BUZZER_PIN = 10;
const int BEEP_DURATION = 120;     // ms ON
const int BEEP_PAUSE    = 120;     // ms OFF

int lastHour = -1;

void setupBuzzer() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void beepOnce() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(BEEP_DURATION);
  digitalWrite(BUZZER_PIN, LOW);
  delay(BEEP_PAUSE);
}

void hourlyBeep(int h) {
  int count = h % 12;
  if (count == 0) count = 12;

  for (int i = 0; i < count; i++) {
    beepOnce();
  }
}

void updateBuzzer() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int currentHour = timeinfo.tm_hour;

  if (currentHour != lastHour && timeinfo.tm_min == 0 && timeinfo.tm_sec < 3) {
    hourlyBeep(currentHour);
    lastHour = currentHour;
  }
}
