#include <font.h>
#include <oled.h>
#include <stdio.h>
#include "fsl_lpi2c.h"
#include "peripherals.h"
#include <string.h>
#include "fsl_debug_console.h"

static uint8_t oled_buffer[OLED_WIDTH * OLED_HEIGHT / 8];
static uint8_t oled_cursor_x = 0;
static uint8_t oled_cursor_y = 0;
static uint8_t oled_font_size = 1;

static status_t OLED_I2C_Write(uint8_t control,
                               const uint8_t *data,
                               size_t dataSize)
{
    lpi2c_master_transfer_t transfer;

    memset(&transfer, 0, sizeof(transfer));

    transfer.slaveAddress   = OLED_ADDR;
    transfer.direction      = kLPI2C_Write;
    transfer.subaddress     = control;
    transfer.subaddressSize = 1;
    transfer.data            = (uint8_t *)data;
    transfer.dataSize       = dataSize;
    transfer.flags           = kLPI2C_TransferDefaultFlag;

//    return LPI2C_MasterTransferBlocking(
//        LP_FLEXCOMM2_PERIPHERAL,
//        &transfer
//    );
    status_t status;

    status = LPI2C_MasterTransferBlocking(
        LP_FLEXCOMM2_PERIPHERAL,
        &transfer
    );

    PRINTF("OLED I2C: control=0x%02X size=%u status=%d\r\n",
           control,
           (unsigned)dataSize,
           status);

    return status;
}

status_t OLED_Init(void)
{
    static const uint8_t init_cmds[] =
    {
        0xAE,
        0xA8, 0x3F,
        0x20, 0x00,
        0x40,
        0xD3, 0x00,
        0xA1,
        0xC8,
        0xDA, 0x12,
        0x81, 0x7F,
        0xA4,
        0xA6,
        0xD5, 0x80,
        0xD9, 0xC2,
        0xDB, 0x20,
        0x8D, 0x14,
        0x2E,
        0x21, 0x00, 0x7F,
        0x22, 0x00, 0x07,
        0xAF
    };

    return OLED_I2C_Write(
        0x00,
        init_cmds,
        sizeof(init_cmds)
    );
}



void OLED_Clear(void)
{
    memset(oled_buffer, 0x00, sizeof(oled_buffer));
}

status_t OLED_Update(void)
{
    static const uint8_t commands[] =
    {
        0x21, 0x00, 0x7F,
        0x22, 0x00, 0x07
    };

    status_t status;

    status = OLED_I2C_Write(
        0x00,
        commands,
        sizeof(commands)
    );

    if (status != kStatus_Success)
    {
        return status;
    }

    status = OLED_I2C_Write(
        0x40,
        oled_buffer,
        sizeof(oled_buffer)
    );

    return status;
}

//Cursor Setup Snippet
void OLED_SetCursor(uint8_t x, uint8_t y)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
    {
        return;
    }

    oled_cursor_x = x;
    oled_cursor_y = y;
}

//Draw Pixel Code Snippet

void OLED_DrawPixel(uint8_t x, uint8_t y)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
    {
        return;
    }

    uint16_t index = x + ((y >> 3) * OLED_WIDTH);
    uint8_t mask = 1U << (y & 0x07);

    oled_buffer[index] |= mask;
}


//Character Rendering
void OLED_DrawChar(char character)
{
    if (character < 32 || character > 127)
    {
        return;
    }

    uint8_t font_index = character - 32;

    uint8_t x = oled_cursor_x;
    uint8_t y = oled_cursor_y;

    uint8_t char_width  = 5 * oled_font_size;
    uint8_t char_height = 8 * oled_font_size;

    if (x + char_width > OLED_WIDTH ||
        y + char_height > OLED_HEIGHT)
    {
        return;
    }

    for (uint8_t column = 0; column < 5; column++)
    {
        uint8_t column_data = FONT[font_index][column];

        for (uint8_t row = 0; row < 8; row++)
        {
            if (column_data & (1U << row))
            {
                for (uint8_t sx = 0; sx < oled_font_size; sx++)
                {
                    for (uint8_t sy = 0; sy < oled_font_size; sy++)
                    {
                        OLED_DrawPixel(
                            x + (column * oled_font_size) + sx,
                            y + (row * oled_font_size) + sy
                        );
                    }
                }
            }
        }
    }

    /* Character width + 1 pixel spacing */
    oled_cursor_x += char_width + oled_font_size;
}

//OLED String Printing:
void OLED_Print(const char *text)
{
    uint8_t char_width  = 5 * oled_font_size;
    uint8_t char_height = 8 * oled_font_size;
    uint8_t char_spacing = oled_font_size;

    while (*text != '\0')
    {
        if (*text == '\n')
        {
            oled_cursor_x = 0;

            if (oled_cursor_y + char_height <= OLED_HEIGHT)
            {
                oled_cursor_y += char_height;
            }
            else
            {
                return;
            }
        }
        else
        {
            /* Wrap before drawing if character won't fit */
            if (oled_cursor_x + char_width + char_spacing > OLED_WIDTH)
            {
                oled_cursor_x = 0;

                if (oled_cursor_y + char_height <= OLED_HEIGHT)
                {
                    oled_cursor_y += char_height;
                }
                else
                {
                    return;
                }
            }

            OLED_DrawChar(*text);
        }

        text++;
    }
}

//Numerical value printing setup
void OLED_PrintInt(int value)
{
    char buffer[12];

    snprintf(buffer, sizeof(buffer), "%d", value);

    OLED_Print(buffer);
}

//Emoji Bitmap Drawing
void OLED_DrawBitmap(uint8_t x,
                     uint8_t y,
                     const uint8_t *bitmap,
                     uint8_t width,
                     uint8_t height)
{
    if (bitmap == NULL)
    {
        return;
    }

    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
    {
        return;
    }

    for (uint8_t bitmap_y = 0; bitmap_y < height; bitmap_y++)
    {
        for (uint8_t bitmap_x = 0; bitmap_x < width; bitmap_x++)
        {
            uint16_t byte_index =
                (bitmap_y / 8U) * width + bitmap_x;

            uint8_t bit_mask =
                1U << (bitmap_y % 8U);

            if (bitmap[byte_index] & bit_mask)
            {
                uint8_t pixel_x = x + bitmap_x;
                uint8_t pixel_y = y + bitmap_y;

                if (pixel_x < OLED_WIDTH &&
                    pixel_y < OLED_HEIGHT)
                {
                    OLED_DrawPixel(pixel_x, pixel_y);
                }
            }
        }
    }
}

//UI Line generation func
void OLED_DrawLine(uint8_t x0,
                   uint8_t y0,
                   uint8_t x1,
                   uint8_t y1)
{
    int16_t dx = (int16_t)x1 - x0;
    int16_t dy = (int16_t)y1 - y0;

    int16_t sx = (dx >= 0) ? 1 : -1;
    int16_t sy = (dy >= 0) ? 1 : -1;

    dx = (dx >= 0) ? dx : -dx;
    dy = (dy >= 0) ? dy : -dy;

    int16_t err = dx - dy;

    while (1)
    {
        OLED_DrawPixel(x0, y0);

        if (x0 == x1 && y0 == y1)
        {
            break;
        }

        int16_t e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

//UI Rectangle Border Implement
void OLED_DrawRect(uint8_t x,
                   uint8_t y,
                   uint8_t width,
                   uint8_t height)
{
    if (width == 0 || height == 0)
    {
        return;
    }

    /* Top */
    OLED_DrawLine(x, y,
                  x + width - 1, y);

    /* Bottom */
    OLED_DrawLine(x, y + height - 1,
                  x + width - 1, y + height - 1);

    /* Left */
    OLED_DrawLine(x, y,
                  x, y + height - 1);

    /* Right */
    OLED_DrawLine(x + width - 1, y,
                  x + width - 1, y + height - 1);
}

//UI Rect Box  Fill implement
void OLED_FillRect(uint8_t x,
                   uint8_t y,
                   uint8_t width,
                   uint8_t height)
{
    if (width == 0 || height == 0)
    {
        return;
    }

    for (uint16_t py = 0; py < height; py++)
    {
        for (uint16_t px = 0; px < width; px++)
        {
            uint16_t pixel_x = x + px;
            uint16_t pixel_y = y + py;

            if (pixel_x < OLED_WIDTH &&
                pixel_y < OLED_HEIGHT)
            {
                OLED_DrawPixel((uint8_t)pixel_x,
                               (uint8_t)pixel_y);
            }
        }
    }
}

//Auto Aligned Text Print
void OLED_PrintCentered(const char *text)
{
    uint16_t text_width;
    uint16_t cursor_x;

    if (text == NULL)
    {
        return;
    }

    text_width = strlen(text) * (6 * oled_font_size);

    if (text_width >= OLED_WIDTH)
    {
        OLED_SetCursor(0, oled_cursor_y);
    }
    else
    {
        cursor_x = (OLED_WIDTH - text_width) / 2;
        OLED_SetCursor((uint8_t)cursor_x, oled_cursor_y);
    }

    OLED_Print(text);
}

//Font Sizing functionality setup
void OLED_SetFontSize(uint8_t size)
{
        oled_font_size = size;
}
