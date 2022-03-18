#ifndef __INC_WIDGET_STYLE_H
#define __INC_WIDGET_STYLE_H

#include "ui/ui.h"

#define STYLE_BG_DEFAULT      RGB(0, 0, 0)
#define STYLE_BORDER_DEFAULT  RGB(0xf, 0xf, 0xf)
#define STYLE_FRONT_DEFAULT   RGB(0xf, 0xf, 0xf)

typedef enum {
  WIDGET_HIDDEN,
  WIDGET_SHOW
} __attribute__ ((__packed__)) widget_visibility_t;

typedef struct {
  uint16_t background;
  uint16_t border;
  uint16_t front;
  widget_visibility_t visible;
} widget_style_t;

#endif /* __INC_WIDGET_STYLE_H */