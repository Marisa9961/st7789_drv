#include "lcd.h"

#include "lcd_font.h"

SPI_HandleTypeDef* const LCD_SPI = &hspi3;

static const uint16_t LCD_W_ = 320;
static const uint16_t LCD_H_ = 172;

static const uint8_t FONT_W_ = 16;
static const uint8_t FONT_H_ = 32;

/**
 * @brief  存储屏幕状态表
 */
static char ch_table[5][20] = {0};

#define LCD_setCS_()                                           \
    do {                                                       \
        writeGPIO(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); \
    } while (0)

#define LCD_resetCS_()                                           \
    do {                                                         \
        writeGPIO(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); \
    } while (0)

#define LCD_setDC_()                                           \
    do {                                                       \
        writeGPIO(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); \
    } while (0)

#define LCD_resetDC_()                                           \
    do {                                                         \
        writeGPIO(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET); \
    } while (0)

#define LCD_freshRST_()                                            \
    do {                                                           \
        writeGPIO(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET); \
        delayMs(1);                                                \
        writeGPIO(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);   \
        delayMs(120);                                              \
    } while (0)

static void LCD_writeREG_(const uint8_t data) {
    LCD_resetDC_();
    LCD_resetCS_();
    tranSPI(LCD_SPI, &data, sizeof(data));
    LCD_setCS_();
    LCD_setDC_();
}

static void LCD_writeDAT_(const uint8_t data[], uint8_t num) {
    LCD_resetCS_();
    tranSPI(LCD_SPI, data, num);
    LCD_setCS_();
}

static void LCD_setColor_(const uint16_t color) {
    uint8_t color_array[2] = {color >> 8, color};
    LCD_resetCS_();
    tranSPI(LCD_SPI, color_array, sizeof(color_array));
    LCD_setCS_();
}

static void LCD_setAddress_(uint16_t xbegin, uint16_t ybegin, uint16_t xend,
                            uint16_t yend) {
    xend--;
    yend--;

    ybegin += 34U;
    yend += 34U;

    uint8_t x[4] = {xbegin >> 8U, xbegin, xend >> 8U, xend};
    uint8_t y[4] = {ybegin >> 8U, ybegin, yend >> 8U, yend};

    LCD_writeREG_(COL_ADDRESS_);
    LCD_writeDAT_(x, 4);
    LCD_writeREG_(ROW_ADDRESS_);
    LCD_writeDAT_(y, 4);
    LCD_writeREG_(MEM_WRITE);
}

extern void LCD_init() {
    LCD_freshRST_();
    LCD_writeREG_(SLEEP_OFF_);
    delayMs(120);

    LCD_writeREG_(MEM_CTRL_);
    LCD_writeDAT_(MEM_CTRL_DAT_, 1);

    LCD_writeREG_(PIXEL_FORMAT_);
    LCD_writeDAT_(PIXEL_FORMAT_DAT_, 1);

    LCD_writeREG_(PORCH_CTRL_);
    LCD_writeDAT_(PORCH_CTRL_DAT_, 5);

    LCD_writeREG_(GATE_CTRL_);
    LCD_writeDAT_(GATE_CTRL_DAT_, 1);

    LCD_writeREG_(VCOM_CTRL_);
    LCD_writeDAT_(VCOM_CTRL_DAT_, 1);

    LCD_writeREG_(LCM_CTRL_);
    LCD_writeDAT_(LCM_CTRL_DAT_, 1);

    LCD_writeREG_(VRH_VDV_ENABLE_);
    LCD_writeDAT_(VRH_VDV_ENABLE_DAT_, 1);

    LCD_writeREG_(VRH_CTRL_);
    LCD_writeDAT_(VRH_CTRL_DAT_, 1);

    LCD_writeREG_(VDV_CTRL_);
    LCD_writeDAT_(VDV_CTRL_DAT_, 1);

    LCD_writeREG_(FRAME_RATE_);
    LCD_writeDAT_(FRAME_RATE_DAT_, 1);

    LCD_writeREG_(PWR_CTRL_);
    LCD_writeDAT_(PWR_CTRL_DAT_, 2);

    LCD_writeREG_(P_GAM_CTRL_);
    LCD_writeDAT_(P_GAM_CTRL_DAT_, 14);

    LCD_writeREG_(N_GAM_CTRL_);
    LCD_writeDAT_(N_GAM_CTRL_DAT_, 14);

    LCD_writeREG_(INVS_ON_);
    LCD_writeREG_(SLEEP_OFF_);
    delayMs(120);

    LCD_writeREG_(PLAY_ON_);
    LCD_drawScreen(0, 0, LCD_W_, LCD_H_, COLOR_BLACK);
}

/**
 * @brief  画点
 * @param x x坐标
 * @param y y坐标
 * @param color 颜色
 */
extern void LCD_drawPoint(uint16_t x, uint16_t y, uint16_t color) {
    LCD_setAddress_(x, y, x, y);
    LCD_setColor_(color);
}

/**
 * @brief  设置增量(计算直线增量方向)
 * @param inc 方向
 * @param delta 坐标增量
 */
static void LCD_setIncrease_(int16_t* inc, int16_t* delta) {
    if (*delta > 0) {
        *inc = 1;
    } else if (*delta == 0) {
        *inc = 0;
    } else {
        *inc = -1;
        *delta = -(*delta);
    }
}

/**
 * @brief  画线
 * @param xbegin x轴起始
 * @param ybegin y轴起始
 * @param xend x轴结束
 * @param yend y轴结束
 * @param color 颜色
 */
extern void LCD_drwaLine(uint16_t xbegin, uint16_t ybegin, uint16_t xend,
                         uint16_t yend, uint16_t color) {
    uint16_t x = xbegin;
    uint16_t y = ybegin;
    int16_t x_delta = xend - xbegin;
    int16_t y_delta = yend - ybegin;
    int16_t distance = (x_delta >= y_delta) ? x_delta : y_delta;

    int16_t x_err = 0;
    int16_t y_err = 0;
    int16_t x_inc = 0;
    int16_t y_inc = 0;
    LCD_setIncrease_(&x_inc, &x_delta);
    LCD_setIncrease_(&y_inc, &y_delta);

    for (uint16_t i = 0; i <= distance; i++) {
        LCD_drawPoint(x, y, color);
        x_err += x_delta;
        y_err += y_delta;
        if (x_err > distance) {
            x_err -= distance;
            x += x_inc;
        }
        if (y_err > distance) {
            y_err -= distance;
            y += y_inc;
        }
    }
}

/**
 * @brief  填充矩形
 * @param xbegin x轴起始
 * @param ybegin y轴起始
 * @param xend x轴结束
 * @param yend y轴结束
 * @param color 颜色
 */
extern void LCD_drawScreen(uint16_t xbegin, uint16_t ybegin, uint16_t xend,
                           uint16_t yend, uint16_t color) {
    LCD_setAddress_(xbegin, ybegin, xend, yend);
    for (uint16_t i = ybegin; i < yend; i++) {
        for (uint16_t j = xbegin; j < xend; j++) {
            LCD_setColor_(color);
        }
    }
}

/**
 * @brief  绘制实心圆
 * @param x x坐标
 * @param y y坐标
 * @param r 半径
 * @param color 颜色
 */
extern void LCD_drawCircle(uint16_t x, uint16_t y, uint8_t r, uint16_t color) {
    for (uint8_t i = 0; i < r; i++) {
        uint16_t length = (uint16_t)sqrt(r * r - i * i);
        uint16_t xbegin = x - length;
        uint16_t yupper = y + i;
        uint16_t xend = x + length;
        uint16_t ylower = y - i;
        LCD_drwaLine(xbegin, yupper, xend, yupper, color);
        LCD_drwaLine(xbegin, ylower, xend, ylower, color);
    }
}

/**
 * @brief  绘制图片
 * @param x x坐标
 * @param y y坐标
 * @param width 图片宽度
 * @param height 图片高度
 * @param pic 图片数组
 */
extern void LCD_drawPicture(uint16_t x, uint16_t y, uint16_t width,
                            uint16_t height, uint8_t pic[]) {
    LCD_setAddress_(x, y, x + width, y + height);
    uint32_t k = 0;
    for (uint32_t i = 0; i < width; i++) {
        for (uint32_t j = 0; j < height; j++) {
            LCD_writeDAT_(pic + 2 * k, 2);
            k++;
        }
    }
}

/**
 * @brief  绘制字符(不可重叠)
 * @param x x坐标
 * @param y y坐标
 * @param ch 字符
 * @param color 颜色
 */
extern void LCD_drawChar(uint16_t x, uint16_t y, char ch, uint16_t color) {
    uint8_t col = (x - 2) / FONT_W_;
    uint8_t line = (LCD_H_ - y - 4) / FONT_H_ - 1;
    if (ch == ch_table[line][col]) {
        return;
    } else {
        ch_table[line][col] = ch;
    }
    LCD_setAddress_(x, y, x + FONT_W_, y + FONT_H_);

    const uint16_t x_init = x;
    const uint8_t ch_order = ch - ' ';
    for (uint8_t i = 0; i < 64; i++) {
        uint8_t temp_buff = ascii_1632[ch_order][i];
        for (uint8_t j = 0; j < 8; j++) {
            if ((temp_buff & 0x01)) {
                LCD_drawPoint(x, y, color);
            } else {
                LCD_drawPoint(x, y, COLOR_BLACK);
            }
            temp_buff >>= 1;
            x++;

            if ((x - x_init) == FONT_W_) {
                x = x_init;
                y++;
                break;
            }
        }
    }
}

/**
 * @brief  绘制字符串
 * @param line 行(1~5)
 * @param col 列(1~20)
 * @param str 字符串
 * @param color 颜色
 */
extern void LCD_drawString(uint8_t line, uint8_t col, const char* str,
                           uint16_t color) {
    uint16_t x = (col - 1) * FONT_W_ + 2;
    uint16_t y = LCD_H_ - (line * FONT_H_) - 4;
    while (*str != '\0') {
        LCD_drawChar(x, y, *str, color);
        x += FONT_W_;
        str++;
    }
}

/**
 * @brief  获取x与y的实际坐标
 */
#define getX(col, length) (col - 1 + length - 1) * FONT_W_ + 2
#define getY(line) LCD_H_ - (line * FONT_H_) - 4

/**
 * @brief  绘制无符号数
 * @param line 行(1-5)
 * @param col 列(1-20)
 * @param num 数字
 * @param length 长度
 * @param color 颜色
 */
extern void LCD_drawUNum(uint8_t line, uint8_t col, uint32_t num,
                         uint8_t length, uint16_t color) {
    uint16_t x = getX(col, length);
    uint16_t y = getY(line);
    for (uint8_t i = 0; i < length; i++) {
        char ch = (char)(num % 10) + '0';
        if (num == 0) {
            LCD_drawChar(x, y, '0', color);
        } else {
            num /= 10;
            LCD_drawChar(x, y, ch, color);
        }
        x -= FONT_W_;
    }
}

/**
 * @brief  获取绝对值
 * @param num 整形
 * @return int32_t 
 */
static int32_t LCD_abs(int32_t num) {
    if (num < 0) {
        num = -num;
    }
    return num;
}

/**
 * @brief  绘制有符号数
 * @param line 行(1-5)
 * @param col 列(1-20)
 * @param num 数字
 * @param length 长度
 * @param color 颜色
 */
extern void LCD_drawNum(uint8_t line, uint8_t col, int32_t num, uint8_t length,
                        uint16_t color) {
    uint16_t x = getX(col, length);
    uint16_t y = getY(line);
    bool ZF = num == 0 ? true : false;
    bool SF = num < 0 ? true : false;

    for (uint8_t i = 0; i < length; i++) {
        char ch = LCD_abs(num % 10) + '0';
        if (num != 0) {
            num /= 10;
            LCD_drawChar(x, y, ch, color);
        } else if (ZF) {
            LCD_drawChar(x, y, '0', color);
            ZF = false;
        } else if (SF) {
            LCD_drawChar(x, y, '-', color);
            SF = false;
        } else {
            LCD_drawChar(x, y, ' ', color);
        }
        x -= FONT_W_;
    }
}

/**
 * @brief  获取绝对值
 * @param f_num 浮点数
 * @return float_t 
 */
static float_t LCD_fabs(float_t f_num) {
    if (f_num < 0) {
        f_num = -f_num;
    }
    return f_num;
}

/**
 * @brief  绘制浮点数
 * @param line 行(1-5)
 * @param col 列(1-20)
 * @param f_num 数字
 * @param length 长度(>5)
 * @param color 颜色
 */
extern void LCD_drawFloat(uint8_t line, uint8_t col, float f_num,
                          uint8_t length, uint16_t color) {
    uint16_t x = getX(col, length);
    uint16_t y = getY(line);
    bool ZF = LCD_fabs(f_num) < 1 ? true : false;
    bool SF = f_num < 0 ? true : false;
    int32_t num = (int32_t)(f_num * 1000);

    for (uint8_t i = 0; i < length; i++) {
        char ch = LCD_abs(num % 10) + '0';
        if (num != 0) {
            num /= 10;
            LCD_drawChar(x, y, ch, color);
        } else if (ZF) {
            LCD_drawChar(x, y, '0', color);
            if (i == 3) {
                ZF = false;
            }
        } else if (SF) {
            LCD_drawChar(x, y, '-', color);
            SF = false;
        } else {
            LCD_drawChar(x, y, ' ', color);
        }

        x -= FONT_W_;
        if (i == 2) {
            LCD_drawChar(x, y, '.', color);
            x -= FONT_W_;
        }
    }
}
