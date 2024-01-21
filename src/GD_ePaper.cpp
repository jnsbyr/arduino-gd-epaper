/*****************************************************************************
 *
 * Dalian/Waveshare Good Display B/W E Ink Display Driver
 *
 * file:     GD_ePaper.cpp
 * encoding: UTF-8
 * created:  03.01.2024
 *
 *****************************************************************************
 *
 * Copyright (C) 2024 Jens B.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *****************************************************************************
 *
 * partially based on the example code from Dalian Good Display for the 1.02" E Ink display GDEW0102T4
 *
 * dependencies:
 * - Adafruit GFX Library (tested with 1.11.9)
 * - Adafruit BusIO       (tested with 1.14.5)
 *
 *****************************************************************************/

#include "GD_ePaper.h"

//#define DEBUG


GD_ePaper::GD_ePaper(uint16_t displayWidth, uint16_t displayHeight, int16_t csPin, int16_t dcPin, int16_t rstPin, int16_t busyPin, const uint8_t lutW[], uint32_t lutWSize, const uint8_t lutB[], uint32_t lutBSize) :
  Adafruit_GFX(displayWidth, displayHeight),
  csPin(csPin),
  dcPin(dcPin),
  rstPin(rstPin),
  busyPin(busyPin),
  lutWSize(lutWSize),
  lutW(lutW),
  lutBSize(lutBSize),
  lutB(lutB),
  displaySize(displayWidth*displayHeight/8),
  screenBuffer(new uint8_t[displaySize])
{}

// init pins and SPI bus, clear screen buffer (white)
void GD_ePaper::init(SPIClass* spi, uint32_t spiClock)
{
  this->spi = spi;
  this->spiSettings = SPISettings(spiClock, MSBFIRST, SPI_MODE0);

  // configure pins
  digitalWrite(csPin, HIGH); // active low
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);

  pinMode(dcPin, OUTPUT);

  digitalWrite(rstPin, HIGH); // active low
  pinMode(rstPin, OUTPUT);
  digitalWrite(rstPin, HIGH);

  pinMode(busyPin, INPUT);

  // enable SPI
  spi->begin();

  // init screen buffer
  fillScreen(GD_ePaper::COLOR_WHITE);
}

// clear screen buffer and display (white), blocking
void GD_ePaper::clear(uint16_t color)
{
#ifdef DEBUG
  Serial.println("clear");
#endif
  fillScreen(GD_ePaper::COLOR_WHITE);
  writeOldScreen(screenBuffer);
  updateScreen(false);
}

// preset screen buffer with a specific color (Adafruit_GFX)
void GD_ePaper::fillScreen(uint16_t color)
{
  uint8_t value = color == COLOR_BLACK ? 0 : 0xFF;
  for (uint16_t i=0; i<displaySize; i++)
  {
    screenBuffer[i] = value;
  }
}

// select display refresh method before calling update method
// Dalian recommendation: perform full refresh every 5 display updates
void GD_ePaper::setPartialRefresh(bool partial)
{
  partialRefresh = partial;
}

// prepare new screen buffer (white)
void GD_ePaper::newScreen()
{
  if (!displayInitialized)
  {
    writeOldScreen();
  }
  fillScreen(GD_ePaper::COLOR_WHITE);
}

// send new screen buffer to display and refresh
// @param sleepAfterUpdate true: non-blocking (check progress with method waitUtilIdle), false: blocking
// @param sleepAfterRefresh false = blocking: full ~4000 ms, partial ~1500 ms, true = non-blocking: ~8 ms
void GD_ePaper::updateScreen(bool sleepAfterUpdate)
{
#ifdef DEBUG
  Serial.println("updateScreen");
#endif
  writeScreen(screenBuffer);
  refresh(sleepAfterUpdate);
}

// check busy line and delay until idle
// note: can be used after non-blocking refresh() or sleep()
bool GD_ePaper::waitUtilIdle(uint32_t waitDuration)
{
#ifdef DEBUG
  uint32_t start = millis();
#endif
  uint32_t waiting = max((uint32_t)1U, waitDuration*2U);
  bool idle = false;
  do
  {
    delay(1);
    waiting--;
    idle = digitalRead(busyPin) == HIGH;
  } while (!idle && waiting);
#ifdef DEBUG
  if (!idle)
  {
    Serial.print("waitUtilIdle: timeout after ");
  }
  else
  {
    Serial.print("waitUtilIdle: ");
  }
  Serial.println(millis() - start);
#endif
  return idle;
}

// put display into deep sleep, non-blocking
void GD_ePaper::sleep()
{
  writeCommand(CMD_DSLP);
  writeData(0xA5);

  // set sleeping state (although still active)
  setSleepState();
}

// check if display is sleeping (or intending to sleep)
bool GD_ePaper::isSleeping() const
{
  return sleeping;
}

void GD_ePaper::reset()
{
  digitalWrite(rstPin, LOW);
  delay(resetDuration);
  digitalWrite(rstPin, HIGH);
#ifdef DEBUG
  Serial.println("reset");
#endif
  waitUtilIdle(resetDuration);
  sleeping = false;
}

void GD_ePaper::write(uint8_t value, bool data)
{
  if (sleeping) reset();
  digitalWrite(csPin, LOW);
  digitalWrite(dcPin, data);
  spi->beginTransaction(spiSettings);
  spi->transfer(value);
  digitalWrite(csPin, HIGH);
  spi->endTransaction();
}

void GD_ePaper::writeCommand(uint8_t command)
{
#ifdef DEBUG
  Serial.print("writeCommand: ");
  Serial.println(command);
#endif
  write(command, false);
}

void GD_ePaper::writeData(uint8_t data)
{
  write(data, true);
}

void GD_ePaper::writeData(const uint8_t data[], uint16_t size)
{
  digitalWrite(csPin, LOW);
  digitalWrite(dcPin, HIGH);
  spi->beginTransaction(spiSettings);
  for (uint16_t i = 0; i < size; i++)
  {
    spi->transfer(data[i]);
  }
  digitalWrite(csPin, HIGH);
  spi->endTransaction();
}

// resend (current) screen buffer to display RAM (e.g. after deep sleep)
void GD_ePaper::writeOldScreen(const uint8_t screenBuffer[])
{
#ifdef DEBUG
  Serial.println("writeOldScreen");
#endif
  writeCommand(CMD_DTM1);
  writeData(screenBuffer != nullptr ? screenBuffer : this->screenBuffer, displaySize);
  displayInitialized = true;
}


// send new screen buffer to display, but do not update
void GD_ePaper::writeScreen(const uint8_t screenBuffer[])
{
#ifdef DEBUG
  Serial.println("writeScreen");
#endif
  if (!displayInitialized)
  {
    writeOldScreen();
  }

  initDisplay();

  if (false && partialRefresh)
  {
    writeCommand(CMD_PIN);

    writeCommand(CMD_PTL);
    writeData(0);
    writeData((WIDTH - 1) | 0x07);
    writeData(0);
    writeData(HEIGHT - 1);
    writeData(0);
  }

  writeCommand(CMD_DTM2);
  writeData(screenBuffer != nullptr ? screenBuffer : this->screenBuffer, displaySize);

  if (false && partialRefresh)
  {
    writeCommand(CMD_POUT);
  }
}

// update display with new screen buffer
// @param sleepAfterRefresh false = blocking: ~3700 ms, true = non-blocking: ~0 ms
void GD_ePaper::refresh(bool sleepAfterRefresh)
{
  if (sleepAfterRefresh)
  {
  #ifdef DEBUG
    Serial.println("refresh non-blocking");
  #endif
    writeCommand(CMD_AUTO);
    writeData(0xA7); // auto sequence: PON, DRF, POF, DSLP

    // set sleeping state (although still active)
    setSleepState();
  }
  else
  {
    if (!poweredOn) powerOn();
    writeCommand(CMD_DRF);
  #ifdef DEBUG
    Serial.println("refresh");
  #endif
    if (!waitUtilIdle(partialRefresh ? partialRefreshDuration : fullRefreshDuration)) setSleepState();
    powerOff();
  }
}

void GD_ePaper::initDisplay()
{
  if (partialRefresh)
  {
  #ifdef DEBUG
    Serial.println("initDisplay (partial)");
  #endif
/*
    writeCommand(CMD_PWR);
    writeData(0x03); // internal VDH/VDL (default), internal VGH/VGL (default)
    writeData(0x00); // VGHL_LVL -16,+16 (default)
    writeData(0x26); // VDH_LVL 0x26 = +10 (default), 0x2B = +11
    writeData(0x26); // VDL_LVL 0x26 = -10 (default), 0x2B = -11

    writeCommand(CMD_CPSET);
    writeData(0x3F); // CPINT 50 ms (default), CPS Stength 4 (default), CPFRQ 8kHz (default)

    writeCommand(CMD_LUOPT);
    writeData(0x00); // no, all gate on (default)
    writeData(0x00); // SEL05: 10 s, SEL2030: 4.8 s (default)

    writeCommand(CMD_TCON);
    writeData(0x22); // 24 µs non-overlap, default

    writeCommand(CMD_PWS);
    writeData(0x33); // VCOM 3 periods (default), source 6 µs (default)

    writeCommand(CMD_PLL);
    //writeData(0x13); // 50Hz (default)
    writeData(0x05); // 15 Hz

    writeCommand(CMD_VDCS);
    writeCommand(0x00); // 0x00 = -0.10 V (default)
    //writeCommand(0x12); // 0x12 = -1.00 V (from Dalian example)
*/

    writeCommand(0xD2); // undocumented command from Dalian example
    writeData(0x3F);

    writeCommand(CMD_PSR);
    writeData(0x6F); // RES (01 = 80x128), REG (1 = LUT from registers)

    writeCommand(CMD_CDI);
    writeData(0xF2); // VBD 11 (no border, default), DDX 11 (NEW 0/1 where changed), CDI 010 (5 hsync, default)

    writeCommand(CMD_PLL);
    writeData(0x05); // 0x05 = 15 Hz, 0x13 = 50 Hz (default)

    writeCommand(CMD_LUTW);
    writeData(lutW, lutWSize);
    writeCommand(CMD_LUTB);
    writeData(lutB, lutBSize);
  }
  else
  {
  #ifdef DEBUG
    Serial.println("initDisplay (full)");
  #endif
    writeCommand(CMD_PSR);
    writeData(0x5F); // RES (01 = 80x128), REG (0 = HW LUT, default), default 0x4F, refresh with undocumented 0x5F from Dalian example is faster

    writeCommand(CMD_CDI);
    writeData(blackBorder? 0x57 : 0x97); // VBD (border 10 = LUTW, 01 = LUTB), DDX 01 (NEW 1 = LUTW, default), CDI 111 (2 hsync)

    writeCommand(CMD_PLL);
    writeData(0x13); // 50 Hz (default)
  }
}

void GD_ePaper::powerOn()
{
  writeCommand(CMD_PWO);
#ifdef DEBUG
  Serial.println("powerOn");
#endif
  waitUtilIdle(powerOnDuration);
  poweredOn = true;
}

void GD_ePaper::powerOff()
{
  writeCommand(CMD_POF);
#ifdef DEBUG
  Serial.println("powerOff");
#endif
  waitUtilIdle(powerOffDuration);
  poweredOn = false;
}

void GD_ePaper::setSleepState()
{
  sleeping = true;
  poweredOn = false;
  displayInitialized = false;
}

void GD_ePaper::swapValues(int16_t& a, int16_t& b)
{
  a ^= b;
  b ^= a;
  a ^= b;
}

void GD_ePaper::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

  // translate canvas coordinates to display coordinates depending on screen rotation
  switch (getRotation())
  {
    case 1: // 90° (landscape)
      swapValues(x, y);
      x = WIDTH  - x - 1;
      break;
    case 2: // 180° (portait rotated)
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      break;
    case 3: // 270° (landscape rotated)
      swapValues(x, y);
      y = HEIGHT - y - 1;
      break;
  }

  // update pixel in screen buffer
  uint16_t i = x/8 + WIDTH/8*y;
  if (color == COLOR_BLACK)
  {
    screenBuffer[i] &= ~(1 << (7 - (x & 7))); // clear bit
  }
  else
  {
    screenBuffer[i] |= (1 << (7 - (x & 7))); // set bit
  }
}

const uint8_t GDEW0102T4_LUT_W[] PROGMEM =
{
  0x60, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x80, 0x0F, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const uint8_t GDEW0102T4_LUT_B[] PROGMEM =
{
  0x90, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x40, 0x0F, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// 1.02" B/W with UC8157 controller
GDEW0102T4::GDEW0102T4(int16_t csPin, int16_t dcPin, int16_t rstPin, int16_t busyPin) :
  GD_ePaper(80, 128, csPin, dcPin, rstPin, busyPin, GDEW0102T4_LUT_W, sizeof(GDEW0102T4_LUT_W), GDEW0102T4_LUT_B, sizeof(GDEW0102T4_LUT_B))
{};
