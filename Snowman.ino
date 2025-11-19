#include <WiFi.h>
#include "time.h"
#include "globals.h"

void initScreen();
void initClockLogic();
void updateClockLogic();
void initWiFi();
void drawWiFiStatus(const char *l1, const char *l2);

// LEDs & buzzer
void initLeds();
void updateLeds();
void initBuzzer();
void updateBuzzer();

// Time system (real or debug), implemented in time_core.ino
void timeBegin(bool startInDebug);
void timeDebugUpdate();

void setup() {
  Serial.begin(115200);
  delay(500);

  initScreen();     // TFT + canvas
  initWiFi();       // connect WiFi + NTP time

  // true = start in debug time mode
  // false = start using real NTP time
  timeBegin(true);

  initLeds();
  initBuzzer();
  initClockLogic(); // mode, button handling, drawing logic

  drawWiFiStatus("Clock ready", "Snowman :)");
}

void loop() {
  timeDebugUpdate();   // handle serial time commands (D/S/J/T...)
  updateClockLogic();  // draw analog/digital clock
  updateLeds();        // hour progress bar, etc.
  updateBuzzer();      // hourly chime
}
