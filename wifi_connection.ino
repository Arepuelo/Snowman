#include <WiFi.h>
#include "time.h"
#include "globals.h"

// ------------- CONFIG: credentials -------------
const char *WIFI_SSID = "Chani";
const char *WIFI_PASS = "imbecile";

// NTP config (Colombia: UTC-5, no DST)
const long  gmtOffset_sec = -5 * 3600;
const int   daylightOffset_sec = 0;
const char *ntpServer = "pool.ntp.org";

// From wifi_screen.ino
void drawWiFiStatus(const char *l1, const char *l2);

void initWiFi() {
  drawWiFiStatus("WiFi...", "Connecting");
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());
    drawWiFiStatus("WiFi OK", "Getting time...");
  } else {
    Serial.println("WiFi FAILED");
    drawWiFiStatus("WiFi FAIL", ":(");
    return;
  }

  // Configure NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println("Time synced from NTP.");
    drawWiFiStatus("Time OK", "Ready!");
    delay(800);
  } else {
    Serial.println("Failed to get time.");
    drawWiFiStatus("Time FAIL", ":(");
    delay(800);
  }
}
