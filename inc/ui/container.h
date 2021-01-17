#ifndef __INC_WIDGET_CONTAINER_H
#define __INC_WIDGET_CONTAINER_H

#include "ui/ui.h"
#include "ui/widget.h"

typedef struct _widget_container_item {
  widget_t *p_widget;
  widget_box_t rel_box;
  widget_t *p_next;
} widget_container_item_t;

typedef struct {
  widget_t widget;

  /* Children widgets. */
  widget_container_item_t *p_children;
  
} widget_container_t;

void widget_container_init(widget_container_t *p_widget_container, tile_t *p_tile, int x, int y, int width, int height);
void widget_container_add(widget_container_t *p_widget_container, widget_t *p_widget);
void widget_container_remove(widget_container_t *p_widget_container, widget_t *p_widget);

/* debug purpose. */
void widget_debug_list(widget_container_t *p_widget_container);

#endif /* __INC_WIDGET_CONTAINER_H */