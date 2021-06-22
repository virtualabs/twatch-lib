#ifndef __INC_TWATCH_SCREEN_H
#define __INC_TWATCH_SCREEN_H

#include "drivers/st7789.h"
#include "hal/pmu.h"

#define   SCREEN_DEFAULT_BACKLIGHT    1000

esp_err_t twatch_screen_init(void);
void twatch_screen_set_backlight(int level);
int  twatch_screen_get_backlight();
void twatch_screen_set_inverted(bool inverted);
bool twatch_screen_is_inverted(void);
void twatch_screen_set_drawing_window(int x0, int y0, int x1, int y1);
void twatch_screen_get_drawing_window(int *x0, int *y0, int *x1, int *y1);
void twatch_screen_put_pixel(int x, int y, uint16_t color);
void twatch_screen_fill_region(int x, int y, int width, int height, uint16_t color);
void twatch_screen_draw_line(int x0, int y0, int x1, int y1, uint16_t color);
void twatch_screen_update(void);

#endif /* __INC_TWATCH_SCREEN_H */