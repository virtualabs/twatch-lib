#ifndef __INC_UI_WIDGET_H
#define __INC_UI_WIDGET_H

#include "ui.h"

typedef struct widget_t;

typedef enum {
  WE_PRESS,
  WE_RELEASE,
  WE_TAP
} widget_event_t;

/* Callback definition. */
typedef int (*FDrawWidget)(struct widget_t *p_widget);
typedef int (*FEventHandler)(struct widget_t *p_widget, widget_event_t p_event);
typedef void (*FTapHandler)(struct widget_t *p_widget);

typedef struct tWidget {

  /* Parent tile. */
  tile_t *p_tile;

  /* Position */
  int offset_x;
  int offset_y;

  /* Size */
  int width;
  int height;

  /* Callbacks */
  FDrawWidget pfn_drawfunc;
  FEventHandler pfn_eventhandler;

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
void widget_set_eventhandler(widget_t *p_widget, FEventHandler pfn_eventhandler);
void widget_set_userdata(widget_t *p_widget, void *p_user_data);
int widget_draw(widget_t *p_widget);
tile_t *widget_get_tile(widget_t *p_widget);
void widget_send_event(widget_t *p_widget, widget_event_t event);

/* Enumerate widgets. */
widget_t *widget_enum_first(void);
widget_t *widget_enum_next(widget_t *p_widget);

/* Drawing primitives for tiles. */
void widget_set_pixel(widget_t *p_widget, int x, int y, uint16_t pixel);
void widget_fill_region(widget_t *p_widget, int x, int y, int width, int height, uint16_t color);
void widget_draw_line(widget_t *p_widget, int x0, int y0, int x1, int y1, uint16_t color);
void widget_bitblt(widget_t *p_widget, image_t *source, int source_x, int source_y, int width, int height, int dest_x, int dest_y);
void widget_draw_char(widget_t *p_widget, int x, int y, char c, uint16_t color);
void widget_draw_text(widget_t *p_widget, int x, int y, char *psz_text, uint16_t color);
void widget_draw_char_x2(widget_t *p_widget, int x, int y, char c, uint16_t color);
void widget_draw_text_x2(widget_t *p_widget, int x, int y, char *psz_text, uint16_t color);

#endif /* __INC_UI_WIDGET_H */
