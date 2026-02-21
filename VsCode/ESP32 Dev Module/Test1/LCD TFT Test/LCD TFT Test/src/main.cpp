#include <Arduino.h>

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_RED); delay(800);
  tft.fillScreen(TFT_GREEN); delay(800);
  tft.fillScreen(TFT_BLUE); delay(800);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 50);
  tft.println("SPI ILI9341 Test");
}

void loop() { }
