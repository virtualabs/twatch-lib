#include "hal/screen.h"

RTC_DATA_ATTR static int g_default_backlight = SCREEN_DEFAULT_BACKLIGHT;

/**
 * screen_init()
 * 
 * @brief: Initialize screen.
 * @return: ESP_OK on success, ESP_FAIL otherwise.
 **/
esp_err_t twatch_screen_init(void)
{
  esp_err_t result;

  ESP_LOGI("[screen]", "init backlight");

  /* Init backlight. */
  st7789_init_backlight();

  /* Disable backlight during screen bootup. */
  st7789_backlight_set(0);

  /* Power-on screen through PMU. */
  ESP_LOGI("[screen]", "init pmu screen power");
  twatch_pmu_screen_power(true);

  /* Initialize screen. */
  ESP_LOGI("[screen]", "init st7789");
  result = st7789_init();

  if (result == ESP_OK)
  {
    /* Blank screen. */
    st7789_blank();
    st7789_commit_fb();

    /* Power on backlight with default setting. */
    st7789_backlight_set(SCREEN_DEFAULT_BACKLIGHT);
  }
  else
    ESP_LOGE("[screen]", "Cannot initialize screen");

  /* Return result. */
  return result;
}

/**
 * twatch_screen_set_backlight()
 *
 * @brief: Set the screen backlight level.
 * @param: level of the backlight.
 **/
void twatch_screen_set_backlight(int level)
{
  st7789_backlight_set(level);
}

/**
 * twatch_screen_get_backlight()
 *
 * @brief: Get the screen backlight level.
 * @return: level of the backlight.
 **/
int twatch_screen_get_backlight()
{
  return st7789_backlight_get();
}

/**
 * twatch_screen_set_default_backlight()
 *
 * @brief: Set the screen default backlight level.
 * @param: level of the backlight.
 **/
void twatch_screen_set_default_backlight(int level)
{
  g_default_backlight = level;
}

/**
 * twatch_screen_get_backlight()
 *
 * @brief: Get the screen backlight level.
 * @return: level of the backlight.
 **/
int twatch_screen_get_default_backlight()
{
  return g_default_backlight;
}



/**
 * screen_set_drawing_window()
 *
 * @brief: Sets the screen drawing window. 
 * @param x0: top-left corner X coordinate
 * @param y0: top-left corner Y coordinate
 * @param x1: bottom-right corner X coordinate
 * @param y1: bottom-right corner Y coordinate
 **/

void twatch_screen_set_drawing_window(int x0, int y0, int x1, int y1)
{
  /* Forward to ST7789 driver. */
  st7789_set_drawing_window(x0, y0, x1, y1);
}


/**
 * screen_set_drawing_window()
 *
 * @brief: Sets the screen drawing window. 
 * @param x0: pointer to an `int` that will contain the top-left corner X coordinate
 * @param y0: pointer to an `int` that will contain the top-left corner Y coordinate
 * @param x1: pointer to an `int` that will contain the bottom-right corner X coordinate
 * @param y1: pointer to an `int` that will contain the bottom-right corner Y coordinate
 **/

void twatch_screen_get_drawing_window(int *x0, int *y0, int *x1, int *y1)
{
  /* Forward to ST7789 driver. */
  st7789_get_drawing_window(x0, y0, x1, y1);
}


/**
 * screen_put_pixel()
 *
 * @brief: Sets a pixel on screen. 
 * @param x: pixel X coordinate
 * @param y: pixel Y coordinate
 * @param color: pixel color
 **/

void twatch_screen_put_pixel(int x, int y, uint16_t color)
{
  /* Forward to ST7789 driver. */
  st7789_set_pixel(x, y, color);
}


/**
 * screen_fill_region()
 *
 * @brief: Fills a screen region with a single color
 * @param x: region top-left corner X coordinate
 * @param y: region top-left corner Y coordinate
 * @param width: region width in pixels
 * @param height: region height in pixels
 * @param color: pixel color
 **/

void twatch_screen_fill_region(int x, int y, int width, int height, uint16_t color)
{
  /* Forward to ST7789 driver. */
  st7789_fill_region(x, y, width, height, color);
}


/**
 * screen_draw_line()
 *
 * @brief: Draws a line on screen using fast implementation by default.
 * @param x0: line starting point X coordinate
 * @param y0: line starting point Y coordinate
 * @param x1: line ending point X coordinate
 * @param y1: line ending point Y coordinate
 * @param color: pixel color
 **/

void twatch_screen_draw_line(int x0, int y0, int x1, int y1, uint16_t color)
{
  /* Forward to ST7789 driver. */
  st7789_draw_line(x0, y0, x1, y1, color);
}


/**
 * screen_update()
 *
 * @brief: Transmit the current framebuffer to the screen memory.
 **/

void twatch_screen_update(void)
{
  /* Transmit framebuffer to screen. */
  st7789_commit_fb();
}


/**
 * twatch_screen_set_inverted()
 * 
 * @brief: Invert the screen (or not).
 * @param inverted: true to invert, false to use standard mode.
 **/

void twatch_screen_set_inverted(bool inverted)
{
  st7789_set_inverted(inverted);
}


/**
 * twatch_screen_is_inverted()
 * 
 * @brief: Check if screen is inverted (rotated) or not.
 * @return: true if inverted, false otherwise.
 **/

bool twatch_screen_is_inverted(void)
{
  return st7789_is_inverted();
}