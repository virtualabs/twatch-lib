#ifndef __INC_DRIVER_ST7789_H
#define __INC_DRIVER_ST7789_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "hal/spi_types.h"

#define ST7789_DMA_CHAN       2
#define ST7789_SPI_MOSI_IO    GPIO_NUM_19
#define ST7789_SPI_SCLK_IO    GPIO_NUM_18
#define ST7789_SPI_CS_IO      GPIO_NUM_5
#define ST7789_SPI_DC_IO      GPIO_NUM_27

#ifdef CONFIG_TWATCH_V1
  #define ST7789_BL_IO          GPIO_NUM_12
#elif CONFIG_TWATCH_V2
  #define ST7789_BL_IO          GPIO_NUM_25
#endif

#define ST7789_SPI_SPEED      /*80000000L*/SPI_MASTER_FREQ_80M
#define ST779_PARALLEL_LINES  80

#define ST7789_CMD_NOP        0x00
#define ST7789_CMD_SWRESET    0x01
#define ST7789_CMD_RDDID      0x04
#define ST7789_CMD_RDDST      0x09
#define ST7789_CMD_SLPIN      0x10
#define ST7789_CMD_SLPOUT     0x11
#define ST7789_CMD_PTLON      0x12
#define ST7789_CMD_NORON      0x13
#define ST7789_CMD_INVOFF     0x20
#define ST7789_CMD_INVON      0x21
#define ST7789_CMD_DISPOFF    0x28
#define ST7789_CMD_DISPON     0x29
#define ST7789_CMD_CASET      0x2A
#define ST7789_CMD_RASET      0x2B
#define ST7789_CMD_RAMWR      0x2C
#define ST7789_CMD_RAMRD      0x2E
#define ST7789_CMD_PTLAR      0x30
#define ST7789_CMD_MADCTL     0x36
#define ST7789_CMD_COLMOD     0x3A
#define ST7789_CMD_WRDISBV    0x51
#define ST7789_CMD_WRCTRLD    0x53
#define ST7789_CMD_FRMCTR1    0xB1
#define ST7789_CMD_FRMCTR2    0xB2
#define ST7789_CMD_FRMCTR3    0xB3
#define ST7789_CMD_INVCTR     0xB4
#define ST7789_CMD_DISSET5    0xB6
#define ST7789_CMD_GCTRL      0xB7
#define ST7789_CMD_GTADJ      0xB8
#define ST7789_CMD_VCOMS      0xBB
#define ST7789_CMD_LCMCTRL    0xC0
#define ST7789_CMD_IDSET      0xC1
#define ST7789_CMD_VDVVRHEN   0xC2
#define ST7789_CMD_VRHS       0xC3
#define ST7789_CMD_VDVS       0xC4
#define ST7789_CMD_VMCTR1     0xC5
#define ST7789_CMD_FRCTRL2    0xC6
#define ST7789_CMD_CABCCTRL   0xC7
#define ST7789_CMD_RDID1      0xDA
#define ST7789_CMD_RDID2      0xDB
#define ST7789_CMD_RDID3      0xDC
#define ST7789_CMD_RDID4      0xDD
#define ST7789_CMD_GMCTRP1    0xE0
#define ST7789_CMD_GMCTRN1    0xE1
#define ST7789_CMD_PWCTR6     0xFC
#define ST7789_CMD_WAIT       0xFF

#define RGB(r,g,b) ((g&0xf) | ((r&0xf)<<4) | ((b&0x0f)<<8))
#define RGBA(r,g,b,a) ((g&0xf) | ((r&0xf)<<4) | ((b&0x0f)<<8) | ((a<16)?(15-a)<<12:0))

esp_err_t st7789_init(void);
void st7789_backlight_on(void);
void st7789_backlight_set(int backlight_level);
int  st7789_backlight_get();
void st7789_set_drawing_window(int x0, int y0, int x1, int y1);
void st7789_get_drawing_window(int *x0, int *y0, int *x1, int *y1);
void st7789_set_inverted(bool inverted);
bool st7789_is_inverted(void);
void st7789_blank(void);
void st7789_commit_fb(void);
void st7789_set_pixel(int x, int y, uint16_t pixel);
uint16_t st7789_get_pixel(int x, int y);
void st7789_fill_region(int x, int y, int width, int height, uint16_t color);
void st7789_draw_line(int x0, int y0, int x1, int y1, uint16_t color);
void st7789_draw_fastline(int x0, int y, int x1, uint16_t color);
void st7789_draw_circle(int xc, int yc, int r, uint16_t color);
void st7789_draw_disc(int xc, int yc, int r, uint16_t color);
void st7789_copy_line(int x, int y, uint16_t *p_line, int nb_pixels);

#endif /* __INC_DRIVER_ST7789_H */
