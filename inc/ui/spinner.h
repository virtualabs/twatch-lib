#ifndef __INC_WIDGET_SPINNER_H
#define __INC_WIDGET_SPINNER_H

#include "ui/ui.h"
#include "ui/widget.h"
#include "twatch.h"

typedef struct {
  widget_t widget;
  int counter;
  int animation_step;
} widget_spinner_t;

void widget_spinner_init(widget_spinner_t *p_widget_label, tile_t *p_tile, int x, int y, int width, int height);

#endif /* __INC_WIDGET_SPINNER_H */