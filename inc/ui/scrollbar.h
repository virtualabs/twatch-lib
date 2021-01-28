#ifndef __INC_WIDGET_SCROLLBAR_H
#define __INC_WIDGET_SCROLLBAR_H

#include "ui/ui.h"
#include "ui/widget.h"
#include "twatch.h"

#define SCROLLBAR_STYLE_BORDER RGB(0xf, 0xf, 0xf)
#define SCROLLBAR_MIN_HANDLE_SIZE 10

typedef enum {
  SCROLLBAR_HORIZONTAL,
  SCROLLBAR_VERTICAL
} widget_scrollbar_type_t;

typedef struct {
  /* Base widget. */
  widget_t widget;

  /* Scrollbar limits and value. */
  int min;
  int max;
  int value;

  widget_scrollbar_type_t type;

} widget_scrollbar_t;

void widget_scrollbar_init(
  widget_scrollbar_t *p_widget_scrollbar,
  tile_t *p_tile,
  int x,
  int y,
  int width,
  int height,
  widget_scrollbar_type_t type
);

#endif /* __INC_WIDGET_SCROLLBAR_H */