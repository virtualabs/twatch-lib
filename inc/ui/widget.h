#ifndef __INC_UI_WIDGET_H
#define __INC_UI_WIDGET_H

#include "ui.h"

typedef struct widget_t;

typedef int (*FDrawWidget)(struct widget_t *p_widget);

typedef struct tWidget {

  /* Parent tile. */
  tile_t *p_tile;

  /* Position */
  int offset_x;
  int offset_y;

  /* Size */
  int width;
  int height;

  /* Callback */
  FDrawWidget pfn_drawfunc;

  /* User data */
  void *p_user_data;

  /* Next widget. */
  struct tWidget *p_next;

} widget_t;


/**
 * Exports
 **/

void widget_init(widget_t *p_widget, tile_t *p_tile, int x, int y, int width, int height);
void widget_set_drawfunc(widget_t *p_widget, FDrawWidget pfn_drawfunc);
void widget_set_userdata(widget_t *p_widget, void *p_user_data);
int widget_draw(widget_t *p_widget);
tile_t *widget_get_tile(widget_t *p_widget);

/* Enumerate widgets. */
widget_t *widget_enum_first(void);
widget_t *widget_enum_next(widget_t *p_widget);

/* Drawing primitives for tiles. */
void widget_set_pixel(widget_t *p_widget, int x, int y, uint16_t pixel);
void widget_fill_region(widget_t *p_widget, int x, int y, int width, int height, uint16_t color);
void widget_draw_line(widget_t *p_widget, int x0, int y0, int x1, int y1, uint16_t color);
void widget_bitblt(widget_t *p_widget, image_t *source, int source_x, int source_y, int width, int height, int dest_x, int dest_y);

#endif /* __INC_UI_WIDGET_H */
