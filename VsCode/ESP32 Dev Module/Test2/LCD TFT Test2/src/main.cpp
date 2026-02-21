#include <Arduino.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

void setup() {
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH); // Backlight
  
  tft.init();
  tft.setRotation(1);
  
  tft.fillScreen(TFT_RED);
  delay(1000);
  
  tft.fillScreen(TFT_GREEN);
  delay(1000);
  
  tft.fillScreen(TFT_BLUE);
  delay(1000);
  
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 50);
  tft.println("ILI9341");
  tft.setCursor(30, 100);
  tft.println("WORKING!");
  
  delay(2000);
  
  tft.fillScreen(TFT_BLACK);
  delay(500);
}