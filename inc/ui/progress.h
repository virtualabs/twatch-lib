#ifndef __INC_WIDGET_PROGRESS_H
#define __INC_WIDGET_PROGRESS_H

#include "ui/ui.h"
#include "ui/widget.h"

#define PROGRESS_MIN_DEFAULT  0
#define PROGRESS_MAX_DEFAULT  100
#define PROGRESS_VALUE_DEFAULT 0
#define PROGRESS_STYLE_BORDER RGB(0xf, 0xf, 0xf)
#define PROGRESS_STYLE_BG RGB(0x0, 0x0, 0x0)

typedef struct {
  widget_t widget;
  int min;
  int max;
  int value;
} widget_progress_t;

void widget_progress_init(widget_progress_t *p_widget_progress, tile_t *p_tile, int x, int y, int width, int height);
void widget_progress_configure(widget_progress_t *p_widget_progress, int min, int max, int value);
void widget_progress_set_value(widget_progress_t *p_widget_progress, int value);
int widget_progress_get_value(widget_progress_t *p_widget_progress);

#endif /* #define __INC_WIDGET_PROGRESS_H */