#include "lcd_bmp.h"

#include <fatfs.h>

#include "lcd.h"

// file read buf size = 8kb
#define BUF_SIZE 8 * 1024

void LCD_drawBmp(const uint16_t x, const uint16_t y, const char *path) {
  FIL file = {};
  if (f_open(&file, path, FA_READ) != FR_OK) {
    return;
  }

  UINT used_size = 0;
  BYTE read_buf[BUF_SIZE] = {};
  if (f_read(&file, read_buf, sizeof(read_buf), &used_size) != FR_OK) {
    f_close(&file);
    return;
  }

  // pixel_data_offset = 10
  const uint32_t offset = *(uint32_t *)(read_buf + 10);
  // image_width = 18
  const uint32_t width = *(uint32_t *)(read_buf + 18);
  // image_width = 22
  const uint32_t height = *(uint32_t *)(read_buf + 22);

  uint32_t used_height = (used_size - offset) / width / 2;
  LCD_drawPicture(x, y, width, used_height, read_buf + offset - 1);

  uint16_t y_cnt = y;
  uint32_t height_cnt = used_height;
  if (height_cnt == height) {
    f_close(&file);
    return;
  }

  uint32_t read_offset = (offset - 1) + width * used_height * 2;
  while (height_cnt < height) {
    if (f_lseek(&file, read_offset) != FR_OK) {
      f_close(&file);
      return;
    }

    used_size = 0;
    if (f_read(&file, read_buf, sizeof(read_buf), &used_size) != FR_OK) {
      f_close(&file);
      return;
    }

    y_cnt += used_height;
    used_height = used_size / width / 2;
    LCD_drawPicture(x, y_cnt, width, used_height, read_buf);

    read_offset += width * used_height * 2;
    height_cnt += used_height;
  }

  f_close(&file);
  return;
}
