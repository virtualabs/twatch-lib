#ifndef __INC_WIDGET_LISTBOX_H
#define __INC_WIDGET_LISTBOX_H

#include "ui/container.h"

#define LISTBOX_STYLE_BORDER RGB(0xf, 0xf, 0xf)

typedef struct {

  /* Base widget. */
  widget_t widget;

  /* Container. */
  widget_container_t container;

  /* Parameters. */
  int n_items;
  int selected_index;

  /* TODO: scrollbar. */

} widget_listbox_t;

void widget_listbox_init(widget_listbox_t *p_widget_listbox, tile_t *p_tile, int x, int y, int width, int height);
void widget_listbox_add(widget_listbox_t *p_widget_listbox, widget_t *p_widget);
void widget_listbox_remove(widget_listbox_t *p_widget_listbox, widget_t *p_widget);

#endif /* __INC_WIDGET_LISTBOX_H */