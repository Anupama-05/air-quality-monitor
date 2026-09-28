#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include "fsl_common.h"

#define OLED_WIDTH   128
#define OLED_HEIGHT   64
#define OLED_ADDR   0x3C

//OLED Setup - Startup Intialization
status_t OLED_Init(void);

#endif /* OLED_H */

//OLED Setup - Clear Screen
void OLED_Clear(void);

//OLED Setup - Update
status_t OLED_Update(void);

//Pixel drawing
void OLED_DrawPixel(uint8_t x, uint8_t y);

//Character rendering
void OLED_DrawChar(char character);

//Cursor Setup
void OLED_SetCursor(uint8_t x, uint8_t y);

//Print String to OLED.
void OLED_Print(const char *text);

//Print Integer values
void OLED_PrintInt(int value);

//Bitmap support
void OLED_DrawBitmap(uint8_t x,
                     uint8_t y,
                     const uint8_t *bitmap,
                     uint8_t width,
                     uint8_t height);

//UI Lines functionality
void OLED_DrawLine(uint8_t x0,
                   uint8_t y0,
                   uint8_t x1,
                   uint8_t y1);

//UI Rectangle Box implement
void OLED_DrawRect(uint8_t x,
                   uint8_t y,
                   uint8_t width,
                   uint8_t height);

//UI Box Fill Implement
void OLED_FillRect(uint8_t x,
                   uint8_t y,
                   uint8_t width,
                   uint8_t height);

//Auto Text Aligned Printing
void OLED_PrintCentered(const char *text);

//Font Size functionality
void OLED_SetFontSize(uint8_t size);
