#ifndef __INC_WIDGET_LABEL_H
#define __INC_WIDGET_LABEL_H

#include "ui/ui.h"
#include "ui/widget.h"
#include "twatch.h"


#define LABEL_STYLE_TEXT RGB(0xf, 0xf, 0xf)

typedef enum {
  LABEL_FONT_NORMAL,
  LABEL_FONT_SMALL
} widget_label_fontsize_t;

typedef struct {
  widget_t widget;

  /* Button label. */
  char *psz_label;

  /* Font size. */
  widget_label_fontsize_t font_size;

} widget_label_t;

void widget_label_init(widget_label_t *p_widget_label, tile_t *p_tile, int x, int y, int width, int height, char *psz_label);
void widget_label_set_text(widget_label_t *p_widget_label, char *psz_label);
void widget_label_set_fontsize(widget_label_t *p_widget_label, widget_label_fontsize_t font_size);

#endif /* __INC_WIDGET_LABEL_H */