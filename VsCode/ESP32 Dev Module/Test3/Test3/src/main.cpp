#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <TFT_eSPI.h>

// =================================================
//                      OBJECTS
// =================================================
Adafruit_INA219 ina219;
TFT_eSPI tft = TFT_eSPI();

// =================================================
//                  74HC595 PINS
// =================================================
#define DATA_PIN   27   // DS
#define CLOCK_PIN  14   // SH_CP
#define LATCH_PIN  13   // ST_CP

// =================================================
//                 ROTARY ENCODER PINS
// =================================================
#define ENC_SW   4
#define ENC_DT   21
#define ENC_CLK  22

// =================================================
//                OUTPUT CONTROL
// =================================================
#define OUTPUT_COUNT 8
uint8_t outputByte = 0x00;   // وضعیت 8 خروجی
int selectedOutput = 0;

// =================================================
//                ENCODER VARIABLES
// =================================================
volatile int8_t lastCLK;
volatile uint32_t lastEncTime = 0;
bool buttonPressed = false;
uint32_t lastBtnTime = 0;

// =================================================
//                     COLORS
// =================================================
#define CUSTOM_GRAY 0x7BEF

// =================================================
//              FUNCTION PROTOTYPES
// =================================================
void shiftOut595(uint8_t data);
void drawHeader();
void drawOutputs();
void updateINA219();
void IRAM_ATTR encoderISR();
void handleButton();

// =================================================
//                      SETUP
// =================================================
void setup() {
  Serial.begin(115200);
  delay(300);

  // ---------- 74HC595 ----------
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  digitalWrite(DATA_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);
  digitalWrite(LATCH_PIN, LOW);
  shiftOut595(outputByte);   // همه خروجی‌ها خاموش

  // ---------- I2C + INA219 ----------
  Wire.begin(32, 33);
  if (!ina219.begin()) {
    Serial.println("❌ INA219 NOT FOUND");
    while (1);
  }

  // ---------- LCD ----------
  pinMode(15, OUTPUT);       // Backlight
  digitalWrite(15, HIGH);
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // ---------- Rotary Encoder ----------
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_CLK), encoderISR, CHANGE);
  lastCLK = digitalRead(ENC_CLK);

  // ---------- UI ----------
  drawHeader();
  drawOutputs();
}

// =================================================
//                       LOOP
// =================================================
void loop() {
  updateINA219();
  handleButton();
  delay(200);
}

// =================================================
//                 HEADER (TOP BAR)
// =================================================
void drawHeader() {
  tft.fillRect(0, 0, 240, 18, TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(5, 4);
  tft.println("INA219 + 74HC595 OUTPUT CTRL");
}

// =================================================
//                INA219 DISPLAY
// =================================================
void updateINA219() {
  float v = ina219.getBusVoltage_V();
  float i = ina219.getCurrent_mA();
  float p = ina219.getPower_mW() / 1000.0;

  tft.fillRect(0, 18, 240, 14, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(5, 20);
  tft.printf("V:%.2fV  I:%.0fmA  P:%.2fW", v, i, p);
}

// =================================================
//              DRAW OUTPUT SELECTOR
// =================================================
void drawOutputs() {
  int x0 = 15;
  int y0 = 40;
  int index = 0;

  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < 4; col++) {
      if (index >= OUTPUT_COUNT) return;

      int x = x0 + col * 55;
      int y = y0 + row * 55;

      bool state = (outputByte >> index) & 0x01;
      uint16_t fillColor = state ? TFT_GREEN : TFT_RED;

      tft.fillRoundRect(x, y, 45, 45, 6, fillColor);

      // Selector highlight
      if (index == selectedOutput) {
        tft.drawRoundRect(x - 3, y - 3, 51, 51, 6, TFT_YELLOW);
      }

      // Output number
      tft.setTextColor(TFT_BLACK);
      tft.setTextSize(2);
      tft.setCursor(x + 16, y + 14);
      tft.print(index + 1);

      index++;
    }
  }

  // Footer hint
  tft.fillRect(0, 160, 240, 18, TFT_BLACK);
  tft.setTextColor(CUSTOM_GRAY, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 165);
  tft.println("Rotate: Select   Press: Toggle");
}

// =================================================
//                74HC595 SHIFT
// =================================================
void shiftOut595(uint8_t data) {
  digitalWrite(LATCH_PIN, LOW);

  for (int i = 7; i >= 0; i--) {
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_PIN, (data >> i) & 0x01);
    digitalWrite(CLOCK_PIN, HIGH);
  }

  digitalWrite(LATCH_PIN, HIGH);
}

// =================================================
//               ROTARY ENCODER ISR
// =================================================
void IRAM_ATTR encoderISR() {
  uint32_t now = millis();
  if (now - lastEncTime < 5) return;

  int clk = digitalRead(ENC_CLK);
  if (clk != lastCLK) {
    if (digitalRead(ENC_DT) != clk) selectedOutput++;
    else selectedOutput--;

    if (selectedOutput < 0) selectedOutput = OUTPUT_COUNT - 1;
    if (selectedOutput >= OUTPUT_COUNT) selectedOutput = 0;

    drawOutputs();
  }

  lastCLK = clk;
  lastEncTime = now;
}

// =================================================
//                 ENCODER BUTTON
// =================================================
void handleButton() {
  if (digitalRead(ENC_SW) == LOW && !buttonPressed) {
    if (millis() - lastBtnTime > 300) {
      buttonPressed = true;

      // Toggle selected output
      outputByte ^= (1 << selectedOutput);
      shiftOut595(outputByte);
      drawOutputs();

      Serial.print("Output ");
      Serial.print(selectedOutput + 1);
      Serial.print(" = ");
      Serial.println((outputByte >> selectedOutput) & 0x01);

      lastBtnTime = millis();
    }
  }

  if (digitalRead(ENC_SW) == HIGH) {
    buttonPressed = false;
  }
}
