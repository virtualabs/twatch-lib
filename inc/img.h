#ifndef __INC_IMG_H
#define __INC_IMG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <inttypes.h>
#include <stdbool.h>
#include "drivers/st7789.h"

typedef enum {
  DEPTH_1BPP,
  DEPTH_12BPP
} image_depth_t;

typedef enum {
  IMAGE_RAW,
  IMAGE_RLE
} image_type_t;

/* Image structure */
typedef struct {
  /* Image size & depth */
  uint16_t width;
  uint16_t height;
  uint8_t depth;

  /* Image type. */
  uint8_t type;
} image_t;

image_t *load_image(const uint16_t *bitmap_data);
void screen_bitblt(image_t *source, int source_x, int source_y, int width, int height, int dest_x, int dest_y);

#endif /* __INC_IMG_H */
