#ifndef __INC_WIDGET_FRAME_H
#define __INC_WIDGET_FRAME_H

#include "ui/ui.h"
#include "ui/widget.h"

#define FRAME_STYLE_BORDER RGB(0xf, 0xf, 0xf)
#define FRAME_STYLE_BG RGB(0x0, 0x0, 0x0)
#define FRAME_STYLE_BG_PRESSED RGB(0x3, 0x3, 0x3)
#define FRAME_STYLE_TEXT RGB(0xe, 0xe, 0xe)

typedef struct {
  widget_t widget;
} widget_frame_t;

void widget_frame_init(
  widget_frame_t *p_widget_frame,
  tile_t *p_tile,
  int x,
  int y,
  int width,
  int height
);


#endif /* __INC_WIDGET_FRAME_H */