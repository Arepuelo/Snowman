#include <WiFi.h>
#include "time.h"
#include "globals.h"

// Prototypes from other .ino files:
void initScreen();
void initClockLogic();
void updateClockLogic();
void initWiFi();
void drawWiFiStatus(const char *l1, const char *l2);

void setup() {
  Serial.begin(115200);
  delay(500);

  initScreen();    // TFT + canvas + SPI_MODE3
  initWiFi();      // connect WiFi + NTP time
  initClockLogic(); // button, mode, etc.

  drawWiFiStatus("Clock ready", "Snowman :)");
}

void loop() {
  updateClockLogic();  // draw analog/digital clock
}
