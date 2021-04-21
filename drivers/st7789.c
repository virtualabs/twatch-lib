#include "drivers/st7789.h"

#define CMD(x)    st7789_send_cmd(x)
#define DATA(x,y) st7789_send_data(x,y)
#define BYTE(x)   st7789_send_data_byte(x)
#define WAIT(x)   st7789_wait(x)

#define WIDTH     240
#define HEIGHT    240
#define BPP       12
#define FB_SIZE   ((BPP*WIDTH*HEIGHT)/8)
#define FB_CHUNK_SIZE (ST779_PARALLEL_LINES * (BPP*WIDTH)/8)

#define FB_PIXCHUNK ((uint32_t *)(&framebuffer[fb_blk_off]))
#define FB_PIXCHUNK2 ((uint32_t *)(&framebuffer[fb_blk_off+4]))

#define MIX_ALPHA(x,y,a) ((x*(15-a) + (y*a))/15)

#define P1MASK    0xFFFF0F00
#define P1MASKP   0x0000F0FF
#define P2MASK    0xFF00F0FF
#define P2MASKP   0x00FF0F00

spi_device_handle_t spi;
ledc_timer_config_t backlight_timer = {
  .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
  .freq_hz = 5000,                      // frequency of PWM signal
  .speed_mode = LEDC_HIGH_SPEED_MODE,   // timer mode
  .timer_num = LEDC_TIMER_0,            // timer index
  .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
};

ledc_channel_config_t backlight_config = {
  .channel    = LEDC_CHANNEL_0,
  .duty       = 0,
  .gpio_num   = ST7789_BL_IO,
  .speed_mode = LEDC_HIGH_SPEED_MODE,
  .hpoint     = 0,
  .timer_sel  = LEDC_TIMER_0
};

DRAM_ATTR const bool g_inv_x = true;
DRAM_ATTR const bool g_inv_y = true;

/* Drawing window. */
DRAM_ATTR static int g_dw_x0 = 0;
DRAM_ATTR static int g_dw_y0 = 0;
DRAM_ATTR static int g_dw_x1 = WIDTH - 1;
DRAM_ATTR static int g_dw_y1 = HEIGHT - 1;

__attribute__ ((aligned(4)))
DRAM_ATTR static uint8_t databuf[16];

/* Framebuffer. We need one more byte to handle pixels with 32-bit values. */
__attribute__ ((aligned(4)))
DRAM_ATTR static uint8_t framebuffer[FB_SIZE];

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} init_cmd_t;

DRAM_ATTR static const init_cmd_t st_init_cmds[]={
  {ST7789_CMD_SLPOUT, {0}, 0},
  {ST7789_CMD_WAIT, {0}, 250},
  {ST7789_CMD_COLMOD, {0x03}, 1}, /* COLMOD: 12 bits per pixel, 4K colors */
  {ST7789_CMD_WAIT, {0}, 10},
  {ST7789_CMD_CASET, {0x00, 0x00, 0x00, 0xF0}, 4},
  {ST7789_CMD_RASET, {0x00, 0x00, 0x00, 0xF0}, 4},
  {ST7789_CMD_INVON, {0x00}, 0},
  {ST7789_CMD_WAIT, {0}, 10},
  {ST7789_CMD_NORON, {0x00}, 0},
  {ST7789_CMD_WAIT, {0}, 10},
  {ST7789_CMD_DISPON, {0x00}, 0},
  {ST7789_CMD_WAIT, {0}, 250},
  {0,{0}, 0xff}
};

/**
 * @brief Wait for given milliseconds
 * @param milliseconds: nimber of milliseconds to wait
 **/

void IRAM_ATTR st7789_wait(int milliseconds)
{
  vTaskDelay(milliseconds/portTICK_PERIOD_MS);
}

void IRAM_ATTR st7789_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(ST7789_SPI_DC_IO, dc);
}

esp_err_t IRAM_ATTR st7789_send_cmd(const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret = spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);
    return ret;
}

esp_err_t IRAM_ATTR st7789_send_data(const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return ESP_FAIL;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret = spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);
    return ret;
}

esp_err_t IRAM_ATTR st7789_send_data_byte(const uint8_t byte)
{
  return st7789_send_data(&byte, 1);
}


void st7789_init_display(void)
{
  int cmd = 0;

  /* Execute initialization sequence. */
  while (st_init_cmds[cmd].databytes!=0xff) {
    if (st_init_cmds[cmd].cmd == ST7789_CMD_WAIT)
    {
      vTaskDelay(st_init_cmds[cmd].databytes / portTICK_RATE_MS);
    }
    else
    {
      st7789_send_cmd(st_init_cmds[cmd].cmd);
      st7789_send_data(st_init_cmds[cmd].data, st_init_cmds[cmd].databytes&0x1F);
    }
    cmd++;
  }
}


/**
 * @brief Initializes the ST7789 display
 * @retval ESP_OK on success, ESP_FAIL otherwise.
 **/

esp_err_t st7789_init(void)
{
  spi_bus_config_t bus_config = {
        .miso_io_num=-1,
        .mosi_io_num=ST7789_SPI_MOSI_IO,
        .sclk_io_num=ST7789_SPI_SCLK_IO,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=ST779_PARALLEL_LINES*240*2+8,
    };

  spi_device_interface_config_t devcfg={
        .clock_speed_hz=ST7789_SPI_SPEED,
        .mode=0,
        .spics_io_num=ST7789_SPI_CS_IO,
        .queue_size=7,                          //We want to be able to queue 7 transactions at a time
        .pre_cb=st7789_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
        .flags=/*SPI_DEVICE_HALFDUPLEX|*/SPI_DEVICE_NO_DUMMY
    };


  /* Initialize our SPI interface. */
  if (spi_bus_initialize(HSPI_HOST, &bus_config, ST7789_DMA_CHAN) == ESP_OK)
  {
      if (spi_bus_add_device(HSPI_HOST, &devcfg, &spi) == ESP_OK)
      {
        /* Initialize GPIOs */
        gpio_pad_select_gpio(ST7789_BL_IO);
        gpio_pad_select_gpio(ST7789_SPI_DC_IO);
        gpio_pad_select_gpio(ST7789_SPI_CS_IO);
        gpio_set_direction(ST7789_BL_IO, GPIO_MODE_OUTPUT);
        gpio_set_direction(ST7789_SPI_DC_IO, GPIO_MODE_OUTPUT);
        gpio_set_direction(ST7789_SPI_CS_IO, GPIO_MODE_OUTPUT);

        /* Configure backlight for PWM (light control) */
        ledc_timer_config(&backlight_timer);
        ledc_channel_config(&backlight_config);

        /* Set default duty cycle (0, backlight is off). */
        ledc_set_duty(backlight_config.speed_mode, backlight_config.channel, 5000);
        ledc_update_duty(backlight_config.speed_mode, backlight_config.channel);

        vTaskDelay(100 / portTICK_PERIOD_MS);

        /* Send init commands. */
        st7789_init_display();

        return ESP_OK;
      }
      else
        return ESP_FAIL;
  }
  else
    return ESP_FAIL;
}


/**
 * @brief Set screen backlight to max.
 **/

void st7789_backlight_on(void)
{
  ledc_set_duty(backlight_config.speed_mode, backlight_config.channel, 5000);
  ledc_update_duty(backlight_config.speed_mode, backlight_config.channel);
}


/**
 * @brief Set screen backlight level.
 * @param backlight_level, from 0 to 5000
 **/

void st7789_backlight_set(int backlight_level)
{
  ledc_set_duty(backlight_config.speed_mode, backlight_config.channel, backlight_level);
  ledc_update_duty(backlight_config.speed_mode, backlight_config.channel);
}

void st7789_set_drawing_window(int x0, int y0, int x1, int y1)
{
  int x,y;


  /* Ensure x0 < x1, y0 < y1. */
  x = (x0<x1)?x0:x1;
  y = (y0<y1)?y0:y1;

  if (x<0)
    x = 0;
  if (y<0)
    y = 0;

  g_dw_x1 = (x0>x1)?x0:x1;
  g_dw_y1 = (y0>y1)?y0:y1;

  if (g_dw_x1 > (WIDTH -1))
    g_dw_x1 = WIDTH - 1;
  if (g_dw_y1 > (HEIGHT -1))
    g_dw_y1 = HEIGHT - 1;

  g_dw_x0 = x;
  g_dw_y0 = y;
}

void st7789_get_drawing_window(int *x0, int *y0, int *x1, int *y1)
{
  *x0 = g_dw_x0;
  *y0 = g_dw_y0;
  *x1 = g_dw_x1;
  *y1 = g_dw_y1;
}

void IRAM_ATTR st7789_set_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
  databuf[0] = x0 >> 8;
  databuf[1] = x0 & 0xFF;
  databuf[2] = x1 >> 8;
  databuf[3] = x1 & 0xFF;
  CMD(ST7789_CMD_CASET);
  DATA(databuf, 4);
  databuf[0] = y0 >> 8;
  databuf[1] = y0 & 0xFF;
  databuf[2] = y1 >> 8;
  databuf[3] = y1 & 0xFF;
  CMD(ST7789_CMD_RASET);
  DATA(databuf, 4);
  CMD(ST7789_CMD_RAMWR);
}

void st7789_set_fb(uint8_t *frame)
{
  memcpy(framebuffer, frame, FB_SIZE);
}


/**
 * @brief Send framebuffer to screen.
 **/

void IRAM_ATTR st7789_commit_fb(void)
{
  int i;
  st7789_set_window(0, 0, WIDTH, HEIGHT);
  for (i=0; i<FB_SIZE/FB_CHUNK_SIZE; i++)
  {
    st7789_send_data(&framebuffer[i*FB_CHUNK_SIZE], FB_CHUNK_SIZE);
  }
}


/**
 * @brief Fill screen with default color (black)
 **/

void IRAM_ATTR st7789_blank(void)
{
  memset(framebuffer, 0, FB_SIZE);
}


/**
 * @brief Get color of a given pixel in framebuffer
 * @param x: pixel X coordinate
 * @param y: pixel Y coordinate
 * @return: pixel color (12 bits)
 **/

uint16_t IRAM_ATTR _st7789_get_pixel(int x, int y)
{
  int fb_blk, fb_blk_off;
  uint16_t pix = 0;

  /* Sanity checks. */

  if ((x < g_dw_x0) || (x > g_dw_x1) || (y<g_dw_y0) || (y>g_dw_y1))
    return pix;
  
  /* 4-pixel block index */
  fb_blk = (y*WIDTH+x)/4;

  /* Compute address of our 4-pixel block (stored on 6 bytes). */
  fb_blk_off = fb_blk*6;

  /* Modify pixel by 4-byte blocks, as ESP32 does not allow byte-level access. */
  switch(x%4)
  {
    /**
     * Case 0: pixel is stored in byte 0 and 1 of a 4-byte dword.
     * pixel is 0B RG
     * RG BX XX XX
     **/
    case 0:
      {
        pix = (*FB_PIXCHUNK & 0x000000ff) | ((*FB_PIXCHUNK & 0x0000f000) >> 4);
      }
      break;

    /**
     * Case 1: pixel is stored in byte 1 and 2 of a 4-byte dword.
     * pixel is 0B RG
     * XX XR GB XX
     **/

    case 1:
      {
        pix = ((*FB_PIXCHUNK & 0x00000f00) >> 4) | ((*FB_PIXCHUNK & 0x000f0000) >> 8) | ((*FB_PIXCHUNK & 0x00f00000) >> 20);
      }
      break;

    /**
     * Case 2: pixel is stored in byte 3 and 4 of a double 4-byte dword.
     * pixel is 0B RG
     * XX XX XX RG | BX XX XX XX
     **/

    case 2:
      {
        pix = ((*FB_PIXCHUNK & 0xff000000) >> 24) | ((*FB_PIXCHUNK2 & 0x000000f0) << 4);
      }
      break;

    /**
     * Case 3: pixel is stored in byte 4 and 5 of a double 4-byte dword.
     * pixel is 0B RG
     * XX XX XX XX | XR GB XX XX
     **/

    case 3:
      {
        pix = ((*FB_PIXCHUNK2 & 0x0000000f) << 4) | (*FB_PIXCHUNK2 & 0x00000f00) | ((*FB_PIXCHUNK2 & 0x0000f000) >> 12);
      }
      break;
  }

  /* Return color. */
  return pix;
}


/**
 * @brief Get color of a given pixel in framebuffer
 * @param x: pixel X coordinate
 * @param y: pixel Y coordinate
 * @return: pixel color (12 bits)
 **/

uint16_t IRAM_ATTR st7789_get_pixel(int x, int y)
{
  int fb_blk, fb_blk_off;
  uint16_t pix = 0;

  /* Sanity checks. */

  if ((x < g_dw_x0) || (x > g_dw_x1) || (y<g_dw_y0) || (y>g_dw_y1))
    return pix;

  /* Invert if required. */
  if (g_inv_x)
    x = WIDTH - x - 1;
  if (g_inv_y)
    y = HEIGHT - y - 1;

  return _st7789_get_pixel(x, y);
}


/**
 * @brief Set a pixel color in framebuffer
 * @param x: pixel X coordinate
 * @param y: pixel Y coordinate
 * @param color: pixel color (12 bits)
 **/

void IRAM_ATTR st7789_set_pixel(int x, int y, uint16_t color)
{
  int fb_blk, fb_blk_off;
  uint16_t orig_color;
  int r,g,b;
  int a;

  /* Sanity checks. */
  if ((x < g_dw_x0) || (x > g_dw_x1) || (y<g_dw_y0) || (y>g_dw_y1))
    return;
  
  /* Invert if required. */
  if (g_inv_x)
    x = WIDTH - x - 1;
  if (g_inv_y)
    y = HEIGHT - y - 1;

  /* 4-pixel block index */
  fb_blk = (y*WIDTH+x)/4;

  /* Compute address of our 4-pixel block (stored on 6 bytes). */
  fb_blk_off = fb_blk*6;

  /* Handle alpha if required. */
  if (color & 0xf000)
  {
    /* Mix original pixel color with our color. */
    a = (color & 0xf000)>>12;
    orig_color = _st7789_get_pixel(x, y);
    b = MIX_ALPHA( ((color & 0xf00)>>8), ((orig_color & 0xf00)>>8), a);
    r = MIX_ALPHA( ((color & 0x0f0)>>4), ((orig_color & 0x0f0)>>4), a);
    g = MIX_ALPHA( (color & 0x00f), (orig_color & 0x00f), a);
    color = RGB(r,g,b);
  }

  /* Modify pixel by 4-byte blocks, as ESP32 does not allow byte-level access. */
  switch(x%4)
  {
    /**
     * Case 0: pixel is stored in byte 0 and 1 of a 4-byte dword.
     * pixel is 0B RG
     * RG BX XX XX
     **/
    case 0:
      {
        *FB_PIXCHUNK = (*FB_PIXCHUNK & 0xffff0f00) | (color & 0x00ff) | ((color&0xf00)<<4);
      }
      break;

    /**
     * Case 1: pixel is stored in byte 1 and 2 of a 4-byte dword.
     * pixel is 0B RG
     * XX XR GB XX
     **/

    case 1:
      {
        *FB_PIXCHUNK = (*FB_PIXCHUNK & 0xfff00f0ff) | ((color&0xf0)<<4) | ((color&0xf)<<20) | ((color&0xf00)<<8);
      }
      break;

    /**
     * Case 2: pixel is stored in byte 3 and 4 of a double 4-byte dword.
     * pixel is 0B RG
     * XX XX XX RG | BX XX XX XX
     **/

    case 2:
      {
        *FB_PIXCHUNK = (*FB_PIXCHUNK & 0x00ffffff) | (color&0xff)<<24;
        *FB_PIXCHUNK2 = (*FB_PIXCHUNK2 & 0xffffff0f) | (color&0xf00)>>4;
      }
      break;

    /**
     * Case 3: pixel is stored in byte 4 and 5 of a double 4-byte dword.
     * pixel is 0B RG
     * XX XX XX XX | XR GB XX XX
     **/

    case 3:
      {
        *FB_PIXCHUNK2 = (*FB_PIXCHUNK2 & 0xffff00f0) | (color&0xf0)>>4 | (color&0xf)<<12 | (color&0xf00);
      }
      break;
  }
}

/**
 * @brief Set a pixel color in framebuffer
 * @param x: pixel X coordinate
 * @param y: pixel Y coordinate
 * @param color: pixel color (12 bits)
 **/

void IRAM_ATTR _st7789_set_pixel(int x, int y, uint16_t color)
{
  int fb_blk, fb_blk_off;
  uint32_t *ppixel;
  uint32_t *ppixel2;
  uint16_t orig_color;
  uint8_t r,g,b,a;

  /* Sanity checks. */
  if ((x < g_dw_x0) || (x > g_dw_x1) || (y<g_dw_y0) || (y>g_dw_y1))
    return;

  /* 4-pixel block index */
  fb_blk = (y*WIDTH+x)/4;

  /* Compute address of our 4-pixel block (stored on 6 bytes). */
  fb_blk_off = fb_blk*6;

 /* Handle alpha if required. */
  if (color & 0xf000)
  {
    /* Mix original pixel color with our color. */
    a = (color & 0xf000)>>12;
    orig_color = _st7789_get_pixel(x, y);
    b = MIX_ALPHA( ((color & 0xf00)>>8), ((orig_color & 0xf00)>>8), a);
    r = MIX_ALPHA( ((color & 0x0f0)>>4), ((orig_color & 0x0f0)>>4), a);
    g = MIX_ALPHA( (color & 0x00f), (orig_color & 0x00f), a);
    color = RGB(r,g,b);
  }

  /* Modify pixel by 4-byte blocks, as ESP32 does not allow byte-level access. */
  switch(x%4)
  {
    /**
     * Case 0: pixel is stored in byte 0 and 1 of a 4-byte dword.
     * pixel is 0B RG
     * RG BX XX XX
     **/
    case 0:
      {
        ppixel = (uint32_t *)(&framebuffer[fb_blk_off]);
        *ppixel = (*ppixel & 0xffff0f00) | (color & 0x00ff) | ((color&0xf00)<<4);
      }
      break;

    /**
     * Case 1: pixel is stored in byte 1 and 2 of a 4-byte dword.
     * pixel is 0B RG
     * XX XR GB XX
     **/

    case 1:
      {
        ppixel = (uint32_t *)(&framebuffer[fb_blk_off]);
        *ppixel = (*ppixel & 0xfff00f0ff) | ((color&0xf0)<<4) | ((color&0xf)<<20) | ((color&0xf00)<<8);
      }
      break;

    /**
     * Case 2: pixel is stored in byte 3 and 4 of a double 4-byte dword.
     * pixel is 0B RG
     * XX XX XX RG | BX XX XX XX
     **/

    case 2:
      {
        ppixel = (uint32_t *)(&framebuffer[fb_blk_off]);
        ppixel2 = (uint32_t *)(&framebuffer[fb_blk_off+4]);
        *ppixel = (*ppixel & 0x00ffffff) | (color&0xff)<<24;
        *ppixel2 = (*ppixel2 & 0xffffff0f) | (color&0xf00)>>4;
      }
      break;

    /**
     * Case 3: pixel is stored in byte 4 and 5 of a double 4-byte dword.
     * pixel is 0B RG
     * XX XX XX XX | XR GB XX XX
     **/

    case 3:
      {
        ppixel = (uint32_t *)(&framebuffer[fb_blk_off+4]);
        *ppixel = (*ppixel & 0xffff00f0) | (color&0xf0)>>4 | (color&0xf)<<12 | (color&0xf00);
      }
      break;
  }
}


/**
 * @brief Fills a region of the screen with a specific color
 * @param x: top-left X coordinate
 * @param y: top-left Y coordinate
 * @param width: region width
 * @param height: region height
 * @parma color: 12 bpp color
 **/

void st7789_fill_region(int x, int y, int width, int height, uint16_t color)
{
  int _y;

  /* X and y cannot be less than zero. */
  if (x<g_dw_x0)
  {
    /* Fix width, exit if region is out of screen. */
    width -= (g_dw_x0 - x);
    if (width <= 0)
      return;
    x = g_dw_x0;
  }

  if (y<g_dw_y0)
  {
    /* Fix height, exit if region is out of screen. */
    height -= (g_dw_y0 - y);
    if (height <= 0)
      return;
    y = g_dw_y0;
  }

  /* Region must not exceed screen size. */
  if ((x+width) > g_dw_x1)
    width = g_dw_x1-x;
  if ((y+height) > g_dw_y1)
    height = g_dw_y1-y;

  if (width>0)
  {
    for (_y=y; _y<(y+height); _y++)
    {
      /* If alpha channel set, use classic draw line function to blend pixels. */
      if (color&0xf000)
        st7789_draw_line(x, _y, x+width-1, _y, color);
      else
        /* Otherwise use fast line drawing routine. */
        st7789_draw_fastline(x, _y, x+width-1, color);
    }
  }
}


/**
 * @brief Draw fast an horizontal line of color `color` between (x0,y) and (x1, y)
 * @param x0: X coordinate of the start of the line
 * @param y: Y cooordinate of the start of the line
 * @param x1: X coordinate of the end of the line
 * @param color: line color.
 **/

void IRAM_ATTR st7789_draw_fastline(int x0, int y, int x1, uint16_t color)
{
  int _x0,_x1,_y;
  int d=0;
  int n=0;
  int s = 0;
  uint8_t temp[4];

  if (g_inv_x)
  {
    _x0 = WIDTH - x0 - 1;
    _x1 = WIDTH - x1 - 1;

  }
  else
  {
    _x0 = x0;
    _x1 = x1;
  }

  /* Reorder. */
  if (_x0>_x1)
  {
    n=_x1;
    _x1 = _x0;
    _x0 = n;
  }


  if (g_inv_y)
    _y = HEIGHT - y - 1;
  else
    _y = y;


  temp[0] = (color & 0x00ff);
  temp[1] = color>>4;
  temp[2] = ((color&0xf00) >> 8) | ((color&0xf)<<4);

  /* Draw first pixel if line start in the middle of a nibble. */
  if (_x0%2 != 0)
  {
    _st7789_set_pixel(_x0, _y, color);
    d++;
  }

  /* copy pixels by 2 pixels. */
  n = ((_x1 - _x0 + 1) - d)/2;
  s = ((_x0 + d)*3)/2 + ((_y*WIDTH*3)/2);
  for (int x=0; x<n; x++)
  {
    framebuffer[s + x*3] = temp[0];
    framebuffer[s + x*3 + 1] = temp[1];
    framebuffer[s + x*3 + 2] = temp[2];
  }

  /* Do we need to set the last pixel ? */
  if ((n*2) < ((_x1-_x0 + 1)-d))
    _st7789_set_pixel(_x1, _y, color);
}


/**
 * @brief Copy line p_line to the output position
 * @param x: X coordinate of the start of the line
 * @param y: Y cooordinate of the start of the line
 * @param p_line: pointer to an array of colors (uint16_t)
 * @param nb_pixels: number of pixels to copy
 **/

void IRAM_ATTR st7789_copy_line(int x, int y, uint16_t *p_line, int nb_pixels)
{
  int _x,_y,_p=0;
  int d=0;
  int n=0;
  int s = 0;


  /* If X coordinate < 0, apply an offset to the line. */
  if (x<0)
  {
    nb_pixels += x;
    _p=-x;
    x=0;
  }

  /* If Y coordinate < 0, no need to draw. */
  if (y<0)
    return;

  /* Invert X coordinate if required. */
  if (g_inv_x)
    _x = WIDTH - x - 1;
  else
    _x=x;

  /* Invert Y coordinate if required. */
  if (g_inv_y)
    _y = HEIGHT - y -1;
  else
    _y=y;

  /* Adjust number of pixels if image goes out of screen. */
  if ((x + nb_pixels) >= WIDTH)
    nb_pixels = WIDTH - x;


  /* Draw first pixel if line start in the middle of a nibble. */
  if ((_x+d)%2 != 0)
  {
    _st7789_set_pixel(_x, _y, p_line[d]);
    d++;
  }

  /* copy pixels by 2 pixels. */
  n = (nb_pixels - d)/2;
  s = ((_x + d)*3)/2 + ((_y*WIDTH*3)/2);

  /* Fill line with the corresponding pixels. */
  if (g_inv_x)
  {
    /* Draw pixels in reverse order. */
    _p += d;
    for (int x=0;x<n;x++)
    {
      framebuffer[s] = (p_line[_p+1] & 0x00ff);
      framebuffer[s+1] = (p_line[_p+1] >> 4) | ((p_line[_p]>>4)&0xff);
      framebuffer[s+2] = ((p_line[_p]&0xf00) >> 8) | ((p_line[_p]&0xf)<<4);
      _p += 2;
      s -= 3;
    }
  }
  else
  {
    /* Draw pixels in normal order. */
    for (int x=0; x<n; x++)
    {
      framebuffer[s + x*3] = (p_line[2*x+d] & 0x00ff);
      framebuffer[s + x*3 + 1] = (p_line[2*x+d] >> 4) | ((p_line[2*x+d+1]>>4)&0xff);
      framebuffer[s + x*3 + 2] = ((p_line[2*x+d+1]&0xf00) >> 8) | ((p_line[2*x+d+1]&0xf)<<4);
    }
  }

  /* Do we need to set the last pixel ? */
  if (2*n < (nb_pixels-d))
    _st7789_set_pixel(_x, _y, p_line[nb_pixels-1]);
}

/**
 * @brief Draw a line of color `color` between (x0,y0) and (x1, y1)
 * @param x0: X coordinate of the start of the line
 * @param y0: Y cooordinate of the start of the line
 * @param x1: X coordinate of the end of the line
 * @param y1: y coordinate of the end of the line
 **/
void st7789_draw_line(int x0, int y0, int x1, int y1, uint16_t color)
{
  int x, y, dx, dy;
  float e, ex, ey;

  dy = y1 - y0;
  dx = x1 - x0;

  /* Vertical line ? */
  if (dx == 0)
  {
    /* Make sure y0 <= y1. */
    if (y0>y1)
    {
      dy = y0;
      y0 = y1;
      y1 = dy;
    }

    for (y=y0; y<=y1; y++)
      st7789_set_pixel(x0, y, color);
  }
  else
  {
    /* Horizontal line ? */
    if (dy == 0)
    {
      /* Make sure x0 <= x1. */
      if (x0>x1)
      {
        dx = x0;
        x0 = x1;
        x1 = dx;
      }

      /*
       * Use st7789_fill_region() rather than st7789_draw_fastline()
       * as it will check boundaries, adapt coordinates and relies
       * on fast line drawing.
       * 
       * If color alpha channel is set, use a classic for loop to
       * set pixels. We cannot use the fast way as we need to blend
       * pixels.
       */
      if (color & 0xf000)
      {
        for (x=x0; x<=x1; x++)
          st7789_set_pixel(x, y0, color);
      }
      else
        st7789_fill_region(x0, y0, x1 - x0 + 1, 1, color);
    }
    else
    {
      y = y0;
      e = 0.0;
      ex = dy/dx;
      ey = -1.0;
      for (x=x0; x<=x1; x++)
      {
        st7789_set_pixel(x, y, color);
        e += ex;
        if (e >= 0.5)
        {
          y++;
          e = e+ey;
        }
      }
    }
  }
}


/**
 * st7789_draw_circle()
 * 
 * @brief: Draw a circle of a given radius at given coordinates
 * @param xc: circle center X coordinate
 * @param yc: circle center Y coordinate
 * @param r: circle radius
 * @param color: circle color
 **/

void st7789_draw_circle(int xc, int yc, int r, uint16_t color)
{
  int x = 0;
  int y = r;
  int d = r - 1;

  /* Does not support alpha channel. */
  color &= 0x0fff;

  while (y >= x)
  {
    st7789_set_pixel(xc + x, yc + y, color);
    st7789_set_pixel(xc + y, yc + x, color);
    st7789_set_pixel(xc - x, yc + y, color);
    st7789_set_pixel(xc - y, yc + x, color);
    st7789_set_pixel(xc + x, yc - y, color);
    st7789_set_pixel(xc + y, yc - x, color);
    st7789_set_pixel(xc - x, yc - y, color);
    st7789_set_pixel(xc - y, yc - x, color);

    if (d >= (2*x))
    {
      d = d - 2*x - 1;
      x++;
    }
    else if (d < (2*(r-y)))
    {
      d = d + 2*y - 1;
      y--;
    }
    else
    {
      d = d + 2*(y - x - 1);
      y--;
      x++;
    }
  }
}


/**
 * st7789_draw_disc()
 * 
 * @brief: Draw a disc of a given radius at given coordinates
 * @param xc: disc center X coordinate
 * @param yc: disc center Y coordinate
 * @param r: disc radius
 * @param color: disc color
 **/

void st7789_draw_disc(int xc, int yc, int r, uint16_t color)
{
  int i;

  for (i=0;i<r;i++)
  {
    if (i==0)
      st7789_set_pixel(xc, yc, color);
    else
      st7789_draw_circle(xc, yc, i, color);
  }
}