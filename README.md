# Dalian/Waveshare Good Display E Ink Display Driver for Arduino

## Features

 - optimized for low power applications
 - blocking and non-blocking operation
 - support immediate partial update after display deep sleep
 - use SPI block transfer for multi-byte data
 - draw and print operations provided by Adafruit GFX library
 - tested with the GDEW0102T4 display and the SAMD21 MCU
 - test build for ATmega 328P, ESP8266 and ESP32 MCU

### Table of contents

[1. Motivation](#motivation)  
[2. Results](#results)  
[3. Documentation](#documentation)  
[4. Examples](#examples)  
[5. Contributing](#contributing)  
[6. Licenses and Credits](#licenses-and-credits)

## Motivation

There are already Arduino libraries available that support ePaper displays - one of them is the [GxEPD2 library](https://github.com/ZinggJM/GxEPD2). It has a modular structure and supports a broad selection of the ePaper displays from Dalian Good Display and the Waveshare variants. 

But the GxEPD2 library has 2 drawbacks:

- Not suitable for low power applications due to its blocking architecture combined with the long processing time of an ePaper display. There is a callback available that gets triggered during blocking operations which provides a way to continue processing other tasks, but this is more a workaround than a solution.
- The library builds on top the Adafruit GFX library which is BSD licensed, but the GxEPD2 library itself is GPL 3.0 licensed - this prevents its usage in some projects.

## Results

Due to the technology involved ePaper displays typically require several seconds to update their screen. This cannot be changed by a device driver, but there is no need to block the main CPU during that time, as the display controller is capable to take care of the whole operation by itself.

- A full refresh of the GDEW0102T4 display takes around 4500 ms using the GxEPD2 library and blocks the CPU all this time. 
- This library needs less than 4000 ms for the same screen image and blocks the CPU for less than 10 ms. 

Image rendering time is not included in these values and adds around 10 ms per screen, depending on the number and type of the rendering operations and the screen size.

The current version of this library only supports the 1.02" B/W E Ink display GDEW0102T4, but adding support for other ePaper displays is partially prepared.

## Documentation

See method description in the [header file](src/GD_ePaper.h) and [example](examples/GD_ePaper_Hello/GD_EPaper_Hello.ino).

## Examples

An implementation example can be found in the [examples](examples) subdirectory.

## Contributing

Contributors are welcome. Please create a merge request if you want to fix a bug or add a feature. I will try to help if a new display model should be supported, but my options are limited if I do not have access to this model.

## Licenses and Credits

Copyright (c) 2024 [Jens B.](https://github.com/jnsbyr/)

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](http://www.apache.org/licenses/LICENSE-2.0)

The code was edited with [Visual Studio Code](https://code.visualstudio.com).

The badges in this document are provided by [img.shields.io](https://img.shields.io/).

This library depends on the Arduino platform, the Arduino SPI library and the following project:

#### Adafruit GFX Library (requiring Adafruit BusIO Library) ####

Copyright (c) 2012 [Adafruit Industries](https://github.com/adafruit/Adafruit-GFX-Library/)

[![License: BSD 2](https://img.shields.io/badge/License-BSD_2--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)
