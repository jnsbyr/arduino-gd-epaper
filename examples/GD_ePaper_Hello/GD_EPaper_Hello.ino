#include "GD_ePaper.h"

#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans18pt7b.h>

// adjust pin assignment to your wiring!
#define PIN_EPD_RST     1 // out
#define PIN_EPD_DC      2 // out
#define PIN_EPD_CS      3 // out
#define PIN_EPD_BUSY    6 // in

GDEW0102T4 display(PIN_EPD_CS, PIN_EPD_DC, PIN_EPD_RST, PIN_EPD_BUSY);

#define SERIAL_SPEED 115200

void setup()
{
  delay(5000);
  Serial.begin(SERIAL_SPEED);
  while(!Serial);

  // init display configuration (pins, SPI, orientation)
  delay(2000);
  uint32_t start = millis();
  display.init();
  display.setRotation(1);
  display.setTextColor(display.COLOR_BLACK);
  Serial.print("init: ");
  Serial.println(millis() - start);

  // clear display
  //display.clear();
  //delay(2000);

  // say hello, centered on screen
  start = millis();
  display.newScreen();

  char message0[32] = "Hello!";
  display.setFont(&FreeSans18pt7b);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(message0, 0, 0, &tbx, &tby, &tbw, &tbh);
  int16_t x = ((display.width() - tbw) / 2) - tbx;
  int16_t y = ((display.height() - tbh) / 2) - tby;
  display.setCursor(x, y);
  display.print(message0);
  Serial.print("draw: ");
  Serial.println(millis() - start);

  // full screen update, non-blocking
  display.updateScreen(true);
  Serial.print("update-nb: ");
  Serial.println(millis() - start);

  delay(6000); // or sleep or do something productive

  // say bye, centered on screen
  start = millis();
  display.newScreen();

  char message1[32] = "Bye!";
  display.setFont(&FreeSans18pt7b);
  display.getTextBounds(message1, 0, 0, &tbx, &tby, &tbw, &tbh);
  x = ((display.width() - tbw) / 2) - tbx;
  y = ((display.height() - tbh) / 2) - tby;
  display.setCursor(x, y);
  display.print(message1);
  Serial.print("draw: ");
  Serial.println(millis() - start);

  // partial screen update, non-blocking
  display.setPartialRefresh(true);
  display.updateScreen(true);
  Serial.print("update-nb: ");
  Serial.println(millis() - start);

  delay(6000); // or sleep or do something productive
}

void loop()
{
  delay(1);
}