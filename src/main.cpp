#include <Arduino.h>
#include "TFT_eSPI.h"
#include "TFT_UI/TFT_UI.h"
#include "Landscape yellow flower 480 x 320 verti 2.h"
#define BL 4
#define NUM_SECTIONS 4
//TFT_eSPI tft = TFT_eSPI();

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite0(&tft);
TFT_eSprite sprite1(&tft);
TFT_eSprite sprite2(&tft);
TFT_eSprite sprite3(&tft);
TFT_eSprite* sprites[4] = { &sprite0, &sprite1, &sprite2, &sprite3 };
TFT_UI ui(&tft, sprites);  

void setup(){
  Serial.begin(115200);

  pinMode(BL, OUTPUT);
  digitalWrite(BL, HIGH);
   tft.init();
  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Starting vertical tile rendering...", 10, 10, 2);

   ui.setRenderDirection(false); // true = vertical rendering
  ui.init(320,480,4,4);

  delay(500);
  Serial.println("Vertical tile rendering system initialized!");
}

unsigned long lastFrameTime = 0;
int fps = 0;

void loop() {
    unsigned long now = millis();
    if (now - lastFrameTime > 0) {
        fps = 1000 / (now - lastFrameTime);
    }
    lastFrameTime = now;

    ui.drawBackground(image_data_Landscapeyellowflower480x320verti2);
    ui.drawBox(0, 0, 80, 80, 10, TFT_RED);

    // Draw FPS text (top-left corner)
    String fpsText = "FPS: " + String(fps);
    ui.drawText(fpsText, 10,10);

    ui.drawRender(); // <- Make sure this is called last
}
