# ESP32_talking_clock
How to create a talking clock with ESP32 and a 1.28" TFT display.

**Ingredients:**

ESP32, any type

1.28" round OLED display, 240 x 240 pixels, for example https://www.amazon.com/Teyleten-Robot-Display-Interface-240x240/dp/B0B7TFRNN1

MAX98357 I2S Amplifier Module

Loudspeaker

Set of MP3 files and an image in the /data folder

When you use this, be sure to edit the 
  - network credentials
  - memory partition scheme of the ESP32 to be No OTA (2MB app / 2MB SPIFFS)
  - upload the /data folder on the ESP32

This thing uses the following fonts:
#include "Orbitron_VariableFont_wght10pt7b.h"

#include "Orbitron_VariableFont_wght15pt7b.h"

#include "Orbitron_VariableFont_wght20pt7b.h"

See https://icircuit.net/adding-custom-fonts-to-tft_espi/4090 on how to make more fonts.

**Libraries:**
<Arduino_GFX_Library.h>

<WiFi.h>

<ezTime.h>

<FS.h>

<SPIFFS.h>

<Audio.h> <<-- this is the ESP32audio-I2S-master

**Connections:**

**Amplifier**

#define I2S_DOUT 25        // I2S Data out pin

#define I2S_BCLK 27        // I2S Bit clock

#define I2S_LRC  26        // I2S Left/Right clock


**Display**

#define TFT_MISO -1

#define TFT_MOSI 21

#define TFT_SCLK 22

#define TFT_CS   5

#define TFT_DC   12

#define TFT_RST  4


