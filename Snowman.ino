#include <WiFi.h>
#include "time.h"
#include "globals.h"

void initScreen();
void initClockLogic();
void updateClockLogic();
void initWiFi();
void drawWiFiStatus(const char *l1, const char *l2);

// Time system (real or debug), implemented in time_core.ino
void timeBegin(bool startInDebug);
void timeDebugUpdate();

void setup() {
  Serial.begin(115200);
  delay(500);

  initScreen();     // TFT + canvas + SPI_MODE3
  initWiFi();       // connect WiFi + NTP time

  // Turn this to true if you want to boot directly in debug mode
  timeBegin(true);

  initClockLogic(); // button, mode, etc.

  drawWiFiStatus("Clock ready", "Snowman :)");
}

void loop() {
  timeDebugUpdate();  // handle serial time commands
  updateClockLogic(); // draw analog/digital clock
}
