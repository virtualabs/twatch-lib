#ifndef __INC_WIDGET_SLIDER_H
#define __INC_WIDGET_SLIDER_H

#include "ui/ui.h"
#include "ui/widget.h"

#define SLIDER_STYLE_BORDER   RGB(0xd, 0xd, 0xd)
#define SLIDER_STYLE_TEXT     RGB(0xe, 0xe, 0xe)
#define SLIDER_STYLE_CURSOR   RGB(0x5, 0x9, 0xf)
#define SLIDER_STYLE_FILL     RGB(0x5, 0x9, 0xf)
#define SLIDER_CURSOR_RADIUS    10
#define SLIDER_FILL_RADIUS       2

#define SLIDER_MIN_DEFAULT     0
#define SLIDER_MAX_DEFAULT   100
#define SLIDER_VALUE_DEFAULT   0

typedef struct {
  widget_t widget;

  /* SLIDER state. */
  int min;
  int max;
  int value;

  /* SLIDER label. */
  char *psz_label;

  /* Events handler. */
  FTapHandler pfn_tap_handler;

} widget_slider_t;

void widget_slider_init(widget_slider_t *p_widget_slider, tile_t *p_tile, int x, int y, int width, int height);
void widget_slider_configure(widget_slider_t *p_widget_slider, int min, int max, int value, int step);
void widget_slider_set_handler(widget_slider_t *p_widget_slider, FTapHandler pfn_handler);
void widget_slider_set_value(widget_slider_t *p_widget_slider, int value);
int  widget_slider_get_value(widget_slider_t *p_widget_slider);

#endif /* __INC_WIDGET_SLIDER_H */
