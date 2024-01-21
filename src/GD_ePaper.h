/*****************************************************************************
 *
 * Dalian/Waveshare Good Display B/W E Ink Display Driver
 *
 * file:     GD_ePaper.h
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
 * - Adafruit GFX Library
 * - Adafruit BusIO
 *
 *****************************************************************************/

#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>


class GD_ePaper : public Adafruit_GFX
{
public:
  static const uint16_t COLOR_BLACK = 0x0000;
  static const uint16_t COLOR_WHITE = 0xFFFF;

public:
  GD_ePaper(uint16_t displayWidth, uint16_t displayHeight, int16_t csPin, int16_t dcPin, int16_t rstPin, int16_t busyPin, const uint8_t lutW[], uint32_t lutWSize, const uint8_t lutB[], uint32_t lutBSize);

  // init pins and SPI bus, clear screen buffer (white)
  void init(SPIClass* spi = &SPI, uint32_t spiClock = 4000000);

  // clear screen buffer and display (white), blocking ~4000 ms
  void clear(uint16_t color = COLOR_WHITE);

  // prepare new screen buffer (white)
  void newScreen();

  // select display refresh method before calling update method
  // Dalian recommendation: perform full refresh every 5 display updates
  void setPartialRefresh(bool partial);

  // send new screen buffer to display and refresh
  // @param sleepAfterUpdate true: non-blocking (check progress with method waitUtilIdle), false: blocking
  // @param sleepAfterRefresh false = blocking: full ~4000 ms, partial ~1500 ms, true = non-blocking: ~8 ms
  void updateScreen(bool sleepAfterUpdate);

  // check busy line and delay until idle (blocking)
  // note: can be used after non-blocking refresh() or sleep()
  bool waitUtilIdle(uint32_t waitDuration);

  // put display into deep sleep, non-blocking
  void sleep();

  // check if display is sleeping (or intending to sleep)
  bool isSleeping() const;

public:
  // preset screen buffer with a specific color
  void fillScreen(uint16_t color) override; // Adafruit_GFX

protected:
  // UC8157 controller commands
  const uint8_t CMD_PSR   = 0x00; // PANEL SETTINGS REGISTER
  const uint8_t CMD_PWR   = 0x00; // POWER SETTINGS
  const uint8_t CMD_POF   = 0x02; // POWER OFF
  const uint8_t CMD_PWO   = 0x04; // POWER ON
  const uint8_t CMD_CPSET = 0x06; // CHARGE PUMP SETTINGS
  const uint8_t CMD_DSLP  = 0x07; // DEEP SLEEP
  const uint8_t CMD_DTM1  = 0x10; // DATA START TRANSMISSION 1 (OLD)
  const uint8_t CMD_DRF   = 0x12; // DISPLAY REFRESH
  const uint8_t CMD_DTM2  = 0x13; // DATA START TRANSMISSION 2 (NEW)
  const uint8_t CMD_AUTO  = 0x17; // AUTO SEQUENCE
  const uint8_t CMD_LUTW  = 0x23; // LOOK-UP TABLE WHITE
  const uint8_t CMD_LUTB  = 0x24; // LOOK-UP TABLE BLACK
  const uint8_t CMD_LUOPT = 0x2A; // LUT OPTION
  const uint8_t CMD_PLL   = 0x30; // PLL
  const uint8_t CMD_CDI   = 0X50; // VCOM AND DATA INTERVAL SETTING
  const uint8_t CMD_TCON  = 0X60; // TCON
  const uint8_t CMD_VDCS  = 0X82; // VCOM DC SETTINGS
  const uint8_t CMD_PTL   = 0X90; // PARTIAL WINDOW
  const uint8_t CMD_PIN   = 0X91; // PARTIAL IN
  const uint8_t CMD_POUT  = 0X92; // PARTIAL OUT
  const uint8_t CMD_PWS   = 0XE3; // POWER SAVING FOR VCOM & SOURCE

protected:
  void initDisplay();
  void powerOn();  // blocking
  void powerOff(); // blocking
  // update display with new screen buffer
  // @param sleepAfterRefresh false = blocking: full ~3700 ms, partial ~1100 ms, true = non-blocking: ~0 ms
  void refresh(bool sleepAfterRefresh);
  void reset(); // blocking
  void setSleepState();
  void swapValues(int16_t& a, int16_t& b);
  void write(uint8_t value, bool data);
  void writeCommand(uint8_t command);
  void writeData(uint8_t data);
  void writeData(const uint8_t data[], uint16_t size);
  // resend (current) screen buffer to display RAM (e.g. after deep sleep)
  // @param screenBuffer display buffer must have a size of displaySize, internal screen buffer is used if null
  void writeOldScreen(const uint8_t screenBuffer[] = nullptr);
  // send new screen buffer to display, but do not update
  // @param screenBuffer display buffer must have a size of displaySize, internal screen buffer is used if null
  void writeScreen(const uint8_t screenBuffer[] = nullptr);

protected:
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;  // Adafruit_GFX

protected:
  int16_t csPin;
  int16_t dcPin;
  int16_t rstPin;
  int16_t busyPin;
  uint16_t lutWSize; // [bytes]
  const uint8_t* lutW;
  uint16_t lutBSize;  // [bytes]
  const uint8_t* lutB;
  uint32_t displaySize; // [bytes]
  uint8_t* screenBuffer;

protected:
  SPIClass* spi = nullptr;
  SPISettings spiSettings;
  uint32_t resetDuration = 2; // [ms]
  uint32_t powerOnDuration = 135; // [ms]
  uint32_t powerOffDuration = 90; // [ms]
  uint32_t fullRefreshDuration = 3700; // [ms]
  uint32_t partialRefreshDuration = 1100; // [ms]
  bool sleeping = true;
  bool poweredOn = false;
  bool displayInitialized = false;
  bool blackBorder = false;
  bool partialRefresh = false;
};

// GDEW0102T4 - 1.02" B/W E Ink display with UC8157 controller
class GDEW0102T4 : public GD_ePaper
{
public:
  GDEW0102T4(int16_t csPin, int16_t dcPin, int16_t rstPin, int16_t busyPin);
};
