#include "globals.h"

// Draw two lines of text centered-ish on screen
void drawWiFiStatus(const char *line1, const char *line2) {
  canvas.fillScreen(ST77XX_BLACK);

  canvas.setTextSize(2);
  canvas.setTextColor(ST77XX_WHITE);

  if (line1) {
    canvas.setCursor(10, 80);
    canvas.print(line1);
  }

  if (line2) {
    canvas.setCursor(10, 110);
    canvas.print(line2);
  }

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), 240, 240);
}
