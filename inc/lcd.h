#ifndef __M_LCD_H__
#define __M_LCD_H__

#include <stdbool.h>
#include <stdint.h>

extern void LCD_init();
extern void LCD_drawPoint(uint16_t x, uint16_t y, uint16_t color);
extern void LCD_drwaLine(uint16_t xbegin, uint16_t ybegin, uint16_t xend,
                         uint16_t yend, uint16_t color);
extern void LCD_drawScreen(uint16_t xbegin, uint16_t ybegin, uint16_t xend,
                           uint16_t yend, uint16_t color);
extern void LCD_drawCircle(uint16_t x, uint16_t y, uint8_t r, uint16_t color);
extern void LCD_drawPicture(uint16_t x, uint16_t y, uint16_t width,
                            uint16_t height, uint8_t pic[]);
extern void LCD_drawChar(uint16_t x, uint16_t y, char ch, uint16_t color);
extern void LCD_drawString(uint8_t line, uint8_t col, const char* str,
                           uint16_t color);
extern void LCD_drawUNum(uint8_t line, uint8_t col, uint32_t num,
                         uint8_t length, uint16_t color);
extern void LCD_drawNum(uint8_t line, uint8_t col, int32_t num, uint8_t length,
                        uint16_t color);
extern void LCD_drawFloat(uint8_t line, uint8_t col, float f_num,
                          uint8_t length, uint16_t color);

enum COLOR {
    COLOR_MILK = 0xFFFF,
    COLOR_BLACK = 0x0000,
    COLOR_AZURE = 0X5A9C,
    COLOR_PURPLE = 0XF81F,
    COLOR_BLOOD = 0xF800,
    COLOR_GREEN = 0x07E0,
    COLOR_YELLOW = 0xFFE0,
    COLOR_BROWN = 0XBC40
};

#endif
