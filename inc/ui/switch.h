#ifndef __INC_WIDGET_SWITCH_H
#define __INC_WIDGET_SWITCH_H

#include "ui/ui.h"
#include "ui/widget.h"

#define SWITCH_STYLE_BORDER RGB(0xf, 0xf, 0xf)
#define SWITCH_STYLE_BG RGB(0x0, 0x0, 0x0)
#define SWITCH_STYLE_BG_PRESSED RGB(0x3, 0x3, 0x3)
#define SWITCH_STYLE_TEXT RGB(0xe, 0xe, 0xe)

typedef enum {
  SWITCH_ON,
  SWITCH_OFF
} switch_state_t;

typedef struct {
  widget_t widget;

  /* Switch state. */
  switch_state_t state;

  /* Events handler. */
  FTapHandler pfn_tap_handler;

} widget_switch_t;

void widget_switch_init(
  widget_switch_t *p_widget_switch,
  tile_t *p_tile,
  int x,
  int y,
  int width,
  int height
);

void widget_switch_set_handler(widget_switch_t *p_widget_switch, FTapHandler pfn_handler);
void widget_switch_set_state(widget_switch_t *p_widget_switch, switch_state_t state);
switch_state_t widget_switch_get_state(widget_switch_t *p_widget_switch);

#endif /* __INC_WIDGET_SWITCH_H */