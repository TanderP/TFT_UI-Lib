#ifndef TFT_UI_H
#define TFT_UI_H

#include <TFT_eSPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <vector>

#define NUM_SECTIONS 4

class TFT_UI
{
public:
    TFT_UI(TFT_eSPI *tft, TFT_eSprite *sprite[4]);

    void setRenderDirection(bool vertical);
    uint16_t hexTo565(uint32_t hexColor);

    void init(int screenW, int screenH, int divW, int divH);

    void drawBackground(const uint16_t *image);

    void drawText(String text, int x, int y);
    void drawBox(int x, int y, int w, int h, int r, uint16_t color);
    void drawCircle(int x, int y, int r, uint16_t color);
    void drawBorder(int x, int y, int w, int h, int r, uint16_t color, uint8_t thickness);

    void setFontStyle(int datum, uint16_t colorText, const GFXfont *fontStyle);
    void setTextStyle(int size, u_int16_t color);
    void setTextColour(u_int16_t color);

    void drawIcon(int x, int y, int pixel_W, int pixel_H,const uint16_t *icon_data, uint16_t transparrent_color);

    void drawMenuSet(int offsetX, int offsetY);
    void drawMenuHighlight(uint16_t fillColor, uint16_t borderColor, int select);
    void drawMenu(int rows, int cols,
                  int tileW, int tileH,
                  int cornerR,
                  uint16_t fillColor,
                  uint16_t borderColor,
                  uint8_t borderThickness,
                  int spacingX, int spacingY,
                  const std::vector<String> &titles);

    uint16_t getTileColor(int tileX, int tileY);

    void drawRender();

private:
    void sendGraphics(int id, int baseX, int baseY);
    void tileRenderTask(void *parameter);
    static void bufferTileRenderTask(void *parameter);
    TFT_eSPI *_tft;
    TFT_eSprite *_sprite[NUM_SECTIONS];
    int SCREEN_W;
    int SCREEN_H;

    int dividerW;
    int dividerH;

    int TILE_W;
    int TILE_H;

    int NUM_COLS;
    int NUM_ROWS;

    bool renderVertically = true; // default: render downwards like a column

    struct TextItem
    {
        String text;
        int x;
        int y;
        int datum;
        uint16_t colorText;
        const GFXfont *fontStyle;
    };

    struct BoxItem
    {
        int x, y, w, h, r;
        uint16_t color;
    };
    struct CircleItem
    {
        int x, y, r;
        uint16_t color;
    };
    struct BorderItem
    {
        int x, y, w, h, r;
        uint16_t color;
        uint8_t thickness;
    };
    struct TileTaskData
    {
        int sectionId;
        int start;
        int end;
        TFT_UI *instance; 
    };

    struct IconItem
    {
        int x;
        int y;
        int imgW;
        int imgH;
        const uint16_t *imageData;
        uint16_t transparentColor; // e.g. 0x0000
    };

    TileTaskData taskData[NUM_SECTIONS];

    std::vector<BorderItem> borderQueue;
    std::vector<TextItem> textQueue;
    std::vector<BoxItem> boxQueue;
    std::vector<CircleItem> CircleQueue;
    std::vector<IconItem> iconQueue;

    TaskHandle_t tileTaskHandles[NUM_SECTIONS];
    SemaphoreHandle_t renderCompleteSemaphore;
    SemaphoreHandle_t tftMutex;

    const uint16_t *currentBackground = nullptr;

    int menuOffsetX = 0;
    int menuOffsetY = 0;
    int selectedOption;
    int borderThickness;
    uint16_t borderColorGlobal; // yellow
    uint16_t fillColorGlobal;   // black
    int globalDatum;
    uint16_t globalColorText;
    int globalSizeText;
    const GFXfont *globalFontStyle;

    float fps = 0.0;
};
#endif
