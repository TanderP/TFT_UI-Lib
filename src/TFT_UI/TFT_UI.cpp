#include "TFT_UI.h"

TFT_UI::TFT_UI(TFT_eSPI *tft, TFT_eSprite *sprite[4])
{

    _tft = tft;
    for (int i = 0; i < 4; i++)
    {
        _sprite[i] = sprite[i];
        taskData[i].instance = this;
    }
    borderColorGlobal = TFT_YELLOW;
    fillColorGlobal = TFT_BLACK;
}

void TFT_UI::setRenderDirection(bool vertical)
{
    renderVertically = vertical;
}

void TFT_UI::drawBackground(const uint16_t *image)
{
    currentBackground = image;
}

void TFT_UI::drawText(String text, int x, int y)
{
    textQueue.push_back({text, x, y});
}
void TFT_UI::drawBox(int x, int y, int w, int h, int r, uint16_t color)
{
    boxQueue.push_back({x, y, w, h, r, color});
}
void TFT_UI::drawCircle(int x, int y, int r, uint16_t color)
{
    CircleQueue.push_back({x, y, r, color});
}
void TFT_UI::drawBorder(int x, int y, int w, int h, int r, uint16_t color, uint8_t thickness)
{
    borderQueue.push_back({x, y, w, h, r, color, thickness});
}

void TFT_UI::setTextStyle(int datum, uint16_t colorText, const GFXfont *fontStyle)
{
    globalDatum = datum;
    globalColorText = colorText;
    globalFontStyle = fontStyle;
}

void TFT_UI::drawMenuSet(int offsetX, int offsetY)
{
    menuOffsetX = offsetX;
    menuOffsetY = offsetY;
}

void TFT_UI::drawMenuHighlight(uint16_t fillColor, uint16_t borderColor, int select)
{
    borderColorGlobal = borderColor;
    fillColorGlobal = fillColor;
    selectedOption = select;
}

void TFT_UI::drawMenu(int rows, int cols,
                      int tileW, int tileH,
                      int cornerR,
                      uint16_t fillColor,
                      uint16_t borderColor,
                      uint8_t borderThickness,
                      int spacingX, int spacingY,
                      const std::vector<String> &titles)
{
    int totalItems = rows * cols;

    int menuWidth = cols * tileW + (cols - 1) * spacingX;
    int menuHeight = rows * tileH + (rows - 1) * spacingY;

    int startX = -menuWidth / 2 + tileW / 2 + menuOffsetX;
    int startY = -menuHeight / 2 + tileH / 2 + menuOffsetY;

    for (int i = 0; i < totalItems; i++)
    {
        int row = i / cols;
        int col = i % cols;

        int centerX = startX + col * (tileW + spacingX);
        int centerY = startY + row * (tileH + spacingY);
        int topLeftX = centerX - (tileW / 2);
        int topLeftY = centerY - (tileH / 2);

        drawBox(topLeftX, topLeftY, tileW, tileH, cornerR, fillColor);
        drawBorder(topLeftX, topLeftY, tileW, tileH, cornerR, borderColor, borderThickness);
        if (i == selectedOption)
        {
            drawBox(topLeftX, topLeftY, tileW, tileH, cornerR, fillColorGlobal);
            drawBorder(topLeftX, topLeftY, tileW, tileH, cornerR, borderColorGlobal, borderThickness);
        }

        if (i < titles.size())
        {
            drawText(titles[i], centerX, centerY);
        }
    }
}
void TFT_UI::drawRender()
{
    for (int i = 0; i < NUM_SECTIONS; i++)
    {
        xTaskNotifyGive(tileTaskHandles[i]);
    }

    for (int i = 0; i < NUM_SECTIONS; i++)
    {
        xSemaphoreTake(renderCompleteSemaphore, portMAX_DELAY);
    }

    textQueue.clear();
    boxQueue.clear();
    CircleQueue.clear();
    borderQueue.clear();
}

void TFT_UI::sendGraphics(int id, int baseX, int baseY){
      // Draw text
  _sprite[id]->setTextDatum(globalDatum);
  _sprite[id]->setFreeFont(globalFontStyle);
  _sprite[id]->setTextColor(globalColorText);

  // Draw boxes
  for (auto &box : boxQueue) {
    _sprite[id]->fillRoundRect(baseX + box.x, baseY + box.y, box.w, box.h, box.r, box.color);
  }

    for (auto &circle : CircleQueue) {
    _sprite[id]->fillCircle(baseX + circle.x, baseY + circle.y, circle.r, circle.color);
  }
  // Draw borders
// Draw borders with fixed rounded corner gaps
for (auto &border : borderQueue) {
  for (int i = 0; i < border.thickness; ++i) {
    int shrink = i;
    int radius = max(border.r - i, 0);  // Ensure radius doesn't go negative

    _sprite[id]->drawRoundRect(
      baseX + border.x + shrink,
      baseY + border.y + shrink,
      border.w - 2 * shrink,
      border.h - 2 * shrink,
      radius,
      border.color
    );
  }
}

  for (auto &item : textQueue) {
    _sprite[id]->drawString(item.text, baseX + item.x, baseY + item.y);
  }

}

void TFT_UI::tileRenderTask(void *parameter){
  TileTaskData *data = (TileTaskData *)parameter;
  int sectionId = data->sectionId;

  _sprite[sectionId]->setColorDepth(16);
  _sprite[sectionId]->createSprite(TILE_W, TILE_H);

  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (renderVertically) {
      for (int ty = data->start; ty < data->end; ty++) {
        for (int tx = 0; tx < NUM_COLS; tx++) {
          int screenX = tx * TILE_W;
          int screenY = ty * TILE_H;

          uint16_t *buf = (uint16_t *)_sprite[sectionId]->getPointer();

          if (currentBackground) {
            const uint16_t *src = &currentBackground[(ty * TILE_H) * SCREEN_W + (tx * TILE_W)];
            for (int y = 0; y < TILE_H; y++) {
              for (int x = 0; x < TILE_W; x++) {
                uint16_t pixel = src[y * SCREEN_W + x];
                buf[y * TILE_W + x] = (pixel >> 8) | (pixel << 8);
              }
            }
          } else {
            for (int y = 0; y < TILE_H; y++) {
              for (int x = 0; x < TILE_W; x++) {
                buf[y * TILE_W + x] = getTileColor(screenX + x, screenY + y);
              }
            }
          }

          int centerX = (SCREEN_W / 2) - screenX;
          int centerY = (SCREEN_H / 2) - screenY;
          if (abs(centerX) < SCREEN_W && abs(centerY) < SCREEN_H) {
            sendGraphics(sectionId, centerX, centerY);
          }

          if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
            _sprite[sectionId]->pushSprite(screenX, screenY);
            xSemaphoreGive(tftMutex);
          }
        }
      }
    } else {
      for (int tx = data->start; tx < data->end; tx++) {
        for (int ty = 0; ty < NUM_ROWS; ty++) {
          int screenX = tx * TILE_W;
          int screenY = ty * TILE_H;

          uint16_t *buf = (uint16_t *)_sprite[sectionId]->getPointer();

          if (currentBackground) {
            const uint16_t *src = &currentBackground[(ty * TILE_H) * SCREEN_W + (tx * TILE_W)];
            for (int y = 0; y < TILE_H; y++) {
              for (int x = 0; x < TILE_W; x++) {
                uint16_t pixel = src[y * SCREEN_W + x];
                buf[y * TILE_W + x] = (pixel >> 8) | (pixel << 8);
              }
            }
          } else {
            for (int y = 0; y < TILE_H; y++) {
              for (int x = 0; x < TILE_W; x++) {
                buf[y * TILE_W + x] = getTileColor(screenX + x, screenY + y);
              }
            }
          }

          int centerX = (SCREEN_W / 2) - screenX;
          int centerY = (SCREEN_H / 2) - screenY;
          if (abs(centerX) < SCREEN_W && abs(centerY) < SCREEN_H) {
            sendGraphics(sectionId, centerX, centerY);
          }

          if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
            _sprite[sectionId]->pushSprite(screenX, screenY);
            xSemaphoreGive(tftMutex);
          }
        }
      }
    }

    xSemaphoreGive(renderCompleteSemaphore);
  }
}

void TFT_UI::bufferTileRenderTask(void *parameter){
TileTaskData *data = static_cast<TileTaskData*>(parameter);
  TFT_UI* self = data->instance;
  self->tileRenderTask(parameter);  // ✅ now safe
}
void TFT_UI::init(int screenW , int screenH, int divW , int divH){
    SCREEN_W = screenW;
  SCREEN_H = screenH;
  dividerW = divW;
  dividerH = divH;

  TILE_W = SCREEN_W / dividerW;
  TILE_H = SCREEN_H / dividerH;

  NUM_COLS = SCREEN_W / TILE_W;
  NUM_ROWS = SCREEN_H / TILE_H;

  renderCompleteSemaphore = xSemaphoreCreateCounting(NUM_SECTIONS, 0);
  tftMutex = xSemaphoreCreateMutex();

  int totalTiles = renderVertically ? NUM_ROWS : NUM_COLS;
  int perSection = totalTiles / NUM_SECTIONS;
  int extra = totalTiles % NUM_SECTIONS;

  for (int i = 0; i < NUM_SECTIONS; i++) {
    taskData[i].sectionId = i;
    taskData[i].start = i * perSection;
    taskData[i].end = (i + 1) * perSection;
    if (i == NUM_SECTIONS - 1) {
      taskData[i].end += extra;
    }

    String taskName = "TileTask" + String(i);
    xTaskCreatePinnedToCore(
      bufferTileRenderTask,
      taskName.c_str(),
      4096,
      &taskData[i],
      1,
      &tileTaskHandles[i],
      i % 2
    );

    Serial.printf("Created %s for %s %d - %d\n",
      taskName.c_str(),
      renderVertically ? "rows" : "cols",
      taskData[i].start,
      taskData[i].end - 1
    );
  }
}
uint16_t TFT_UI::getTileColor(int tileX, int tileY)
{
uint8_t r = (tileX * 70) % 256;
  uint8_t g = (tileY * 120) % 256;
  uint8_t b = (tileX * tileY * 45) % 256;
  return _tft->color565(r, g, b);
}
uint16_t TFT_UI::hexTo565(uint32_t hexColor) {
  uint8_t r = (hexColor >> 16) & 0xFF;
  uint8_t g = (hexColor >> 8) & 0xFF;
  uint8_t b = hexColor & 0xFF;

  return _tft->color565(r, g, b);
}
