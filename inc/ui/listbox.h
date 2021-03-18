#ifndef __INC_WIDGET_LISTBOX_H
#define __INC_WIDGET_LISTBOX_H

#include "ui/widget.h"
#include "ui/container.h"
#include "ui/scrollbar.h"

#define LISTBOX_STYLE_BORDER RGB(0xf, 0xf, 0xf)

typedef enum {
  LB_STATE_IDLE,
  LB_STATE_MOVING,
  LB_STATE_MOVING_FREE,
  LB_STATE_STOPPED
} widget_listbox_anim_state_t;

typedef struct {

  /* Base widget. */
  widget_t widget;

  /* Container. */
  widget_container_t container;

  /* Scrollbar. */
  widget_scrollbar_t scrollbar;

  /* Parameters. */
  int n_items;
  widget_t *p_selected_item;

  /* Scrolling. */
  widget_listbox_anim_state_t state;
  int move_orig_x;
  int move_orig_y;
  int move_orig_offset;
  volatile int offset;

  /* Animation */
  float speed;

} widget_listbox_t;

/* Exposed functions. */
void widget_listbox_init(widget_listbox_t *p_widget_listbox, tile_t *p_tile, int x, int y, int width, int height);
void widget_listbox_add(widget_listbox_t *p_widget_listbox, widget_t *p_widget);
void widget_listbox_remove(widget_listbox_t *p_widget_listbox, widget_t *p_widget);
int widget_listbox_count(widget_listbox_t *p_widget_listbox);

#endif /* __INC_WIDGET_LISTBOX_H */