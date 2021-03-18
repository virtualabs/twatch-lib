#ifndef __INC_WIDGET_BUTTON_H
#define __INC_WIDGET_BUTTON_H

#include "ui/ui.h"
#include "ui/widget.h"

#define BUTTON_STYLE_BORDER RGB(0xf, 0xf, 0xf)
#define BUTTON_STYLE_BG RGB(0x0, 0x0, 0x0)
#define BUTTON_STYLE_BG_PRESSED RGB(0x3, 0x3, 0x3)
#define BUTTON_STYLE_TEXT RGB(0xe, 0xe, 0xe)

typedef enum {
  BUTTON_PRESSED,
  BUTTON_RELEASED
} button_state_t;

typedef struct {
  widget_t widget;

  /* Button state. */
  button_state_t state;

  /* Button label. */
  char *psz_label;

  /* Events handler. */
  FTapHandler pfn_tap_handler;

} widget_button_t;

void widget_button_init(
  widget_button_t *p_widget_button,
  tile_t *p_tile,
  int x,
  int y,
  int width,
  int height,
  char *psz_label
);

void widget_button_set_handler(widget_button_t *p_widget_button, FTapHandler pfn_handler);
void widget_button_set_text(widget_button_t *p_widget_button, char *psz_text);

#endif /* __INC_WIDGET_BUTTON_H */
