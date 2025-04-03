#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <ezTime.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Audio.h>

#include "Orbitron_VariableFont_wght10pt7b.h"
#include "Orbitron_VariableFont_wght15pt7b.h"
#include "Orbitron_VariableFont_wght20pt7b.h"

// Display Pinout
#define TFT_MISO -1
#define TFT_MOSI 21
#define TFT_SCLK 22
#define TFT_CS   5
#define TFT_DC   12
#define TFT_RST  4

// Audio I2S Pinout
#define I2S_DOUT 27
#define I2S_BCLK 25
#define I2S_LRC  26

// WiFi Credentials
const char* ssid = "YOUR_SSID_HERE";
const char* password = "YOUR_PASSWORD_HERE";

// Create display bus instance
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
Arduino_GC9A01 *display = new Arduino_GC9A01(bus, TFT_RST, 0, true);

// Audio instance
Audio audio;

// Define colors
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define DARKBLUE  0x0010
#define BLACK     0x0000
#define GREEN     0x07E0
#define BLUE      0x001F

Timezone myTZ;
String lastDisplayedTime = "";
bool playingSequence = false;
int lastAnnounceMinute = -1;

// Function to draw glowing text
void drawGlowingText(const char* text, int yOffset, uint16_t textColor, uint8_t fontSize) {
  if (fontSize == 10) {
    display->setFont(&Orbitron_VariableFont_wght10pt7b);
  } else if (fontSize == 15) {
    display->setFont(&Orbitron_VariableFont_wght15pt7b);
  } else if (fontSize == 20) {
    display->setFont(&Orbitron_VariableFont_wght20pt7b);
  } else {
    display->setFont(&Orbitron_VariableFont_wght10pt7b);
  }

  display->setTextSize(1);
  int16_t x1, y1;
  uint16_t w, h;

  display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int x = (display->width() - w) / 2;
  int y = (display->height() / 2) + yOffset;

  display->setTextColor(DARKBLUE);
  display->setCursor(x + 1, y + 1);
  display->print(text);

  display->setTextColor(textColor);
  display->setCursor(x, y);
  display->print(text);
}

// Function to draw the sci-fi background
void drawSciFiBackground() {
  int centerX = display->width() / 2;
  int centerY = display->height() / 2;

  // Create glowing donut effect
  for (int i = 60; i > 0; i--) {
    uint16_t color = display->color565(0, 0, min(255, i * 4));
    display->drawCircle(centerX, centerY, i, color);
  }

  // Draw 12 radiating lines
  for (int i = 0; i < 360; i += 30) {
    float angle = i * PI / 180;
    int x2 = centerX + cos(angle) * 100;
    int y2 = centerY + sin(angle) * 100;
    display->drawLine(centerX, centerY, x2, y2, BLUE);
  }
}

// Time announcement function
void playTimeInWords(int hour, int minute) {
  String filesToPlay[5];
  int index = 0;

  // Hour handling
  if (hour < 10) {
    filesToPlay[index++] = "0";
    filesToPlay[index++] = String(hour);
  } else if (hour >= 10 && hour <= 19) {
    filesToPlay[index++] = String(hour);
  } else {
    filesToPlay[index++] = String(hour / 10 * 10);
    if (hour % 10 != 0) {
      filesToPlay[index++] = String(hour % 10);
    }
  }

  // Minute handling
  if (minute < 10) {
    filesToPlay[index++] = "0";
    filesToPlay[index++] = String(minute);
  } else if (minute >= 10 && minute <= 19) {
    filesToPlay[index++] = String(minute);
  } else {
    filesToPlay[index++] = String(minute / 10 * 10);
    if (minute % 10 != 0) {
      filesToPlay[index++] = String(minute % 10);
    }
  }

  // Play all the files
  for (int i = 0; i < index; i++) {
    char fileName[10];
    snprintf(fileName, sizeof(fileName), "/%s.mp3", filesToPlay[i].c_str());
    
    if (!audio.connecttoFS(SPIFFS, fileName)) {
      Serial.print("Failed to open ");
      Serial.println(fileName);
    }

    while (audio.isRunning()) {
      audio.loop();
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize display
  if (!display->begin()) {
    Serial.println("Display begin failed!");
    while (1);
  }
  display->fillScreen(BLACK);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // Load tuner image (from original visual clock)
  File file = SPIFFS.open("/tuner2.bin", "r");
  if (!file) {
    Serial.println("Failed to open tuner image");
    return;
  }

  for (int y = 0; y < 240; y++) {
    for (int x = 0; x < 240; x++) {
      uint16_t pixel;
      file.read((uint8_t*)&pixel, 2);
      display->drawPixel(x, y, pixel);
    }
  }
  file.close();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Set up Audio for I2S
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(14);

  // Play robot beep
  if (!audio.connecttoFS(SPIFFS, "/robotbeep.mp3")) {
    Serial.println("Failed to open robotbeep.mp3");
  }
  while (audio.isRunning()) {
    audio.loop();
  }

  // Initialize time
  waitForSync();
  myTZ.setLocation(F("Europe/Helsinki"));
  Serial.println("Time synchronized");

  // Initial display setup
  display->fillScreen(BLACK);
  drawSciFiBackground();
}

void loop() {
  audio.loop();

  String currentTime = myTZ.dateTime("H:i:s");
  String currentDate = myTZ.dateTime("d.m.Y");
  int currentHour = myTZ.hour();
  int currentMinute = myTZ.minute();

  // Update visual display
  if (currentTime != lastDisplayedTime) {
    display->fillRoundRect(30, 90, 173, 60, 10, BLACK);
    drawGlowingText(currentTime.c_str(), -5, GREEN, 15);
    drawGlowingText(currentDate.c_str(), 20, MAGENTA, 10);
    lastDisplayedTime = currentTime;
  }

  // Announce time on every minute change
  if (currentMinute != lastAnnounceMinute) {
    if (!playingSequence) { 
      playingSequence = true;
      playTimeInWords(currentHour, currentMinute);
      lastAnnounceMinute = currentMinute;
      playingSequence = false;
    }
  }

  events(); // Keep ezTime running
  delay(100);
}
