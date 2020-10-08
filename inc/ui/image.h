#ifndef __INC_WIDGET_IMAGE_H
#define __INC_WIDGET_IMAGE_H

#include "ui/ui.h"
#include "ui/widget.h"
#include "img.h"

typedef struct {
  widget_t widget;
  image_t *p_image;
} widget_image_t;

void widget_image_init(widget_image_t *p_widget_img, tile_t *p_tile, int x, int y, int width, int height, image_t *p_image);

#endif /* __INC_WIDGET_IMAGE_H */
