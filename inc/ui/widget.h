#ifndef __INC_UI_WIDGET_H
#define __INC_UI_WIDGET_H

#include "ui.h"
#include "style.h"
#include "events.h"

#define WIDGET_OFFSET_X(w) ((w->p_tile==NULL)?(w->offset_x):(w->offset_x + w->p_tile->offset_x))
#define WIDGET_OFFSET_Y(w) ((w->p_tile==NULL)?(w->offset_y):(w->offset_y + w->p_tile->offset_y))

#define WE_ERROR      (1)
#define WE_PROCESSED  (0)

struct widget_t;

typedef enum {
  /* Default events. */
  WE_PRESS = SYS_EVENTS_BASE,
  WE_RELEASE,
  WE_TAP,
  WE_SWIPE_LEFT,
  WE_SWIPE_RIGHT,
  WE_SWIPE_UP,
  WE_SWIPE_DOWN,

  /* Listbox events. */
  LB_ITEM_SELECTED=LB_EVENTS_BASE,
  LB_ITEM_DESELECTED
} widget_event_t;

/* Forward declaration */
typedef struct tWidget widget_t;

/* Callback definition. */
typedef int (*FDrawWidget)(widget_t *p_widget);
typedef int (*FEventHandler)(widget_t *p_widget, widget_event_t p_event, int x, int y, int velocity);
typedef void (*FTapHandler)(widget_t *p_widget);

typedef struct {
  int x;
  int y;
  int width;
  int height;
} widget_box_t;

typedef struct tWidget {

  /* Parent tile. */
  tile_t *p_tile;

  /* Widget box. */
  widget_box_t box;

  /* Style. */
  widget_style_t style;

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
FEventHandler widget_set_eventhandler(widget_t *p_widget, FEventHandler pfn_eventhandler);
void widget_set_userdata(widget_t *p_widget, void *p_user_data);
int widget_draw(widget_t *p_widget);
tile_t *widget_get_tile(widget_t *p_widget);
int widget_send_event(widget_t *p_widget, widget_event_t event, int x, int y, int velocity);

/* Style management. */
void widget_set_style(widget_t *p_widget, widget_style_t *p_style);
void widget_set_bg_color(widget_t *p_widget, uint16_t color);
void widget_set_border_color(widget_t *p_widget, uint16_t color);
void widget_set_front_color(widget_t *p_widget, uint16_t color);


/* Widget position. */
void widget_get_abs_box(widget_t *p_widget, widget_box_t *p_box);
int widget_get_abs_x(widget_t *p_widget);
int widget_get_abs_y(widget_t *p_widget);

/* Enumerate widgets. */
widget_t *widget_enum_first(void);
widget_t *widget_enum_next(widget_t *p_widget);

/* Drawing primitives for tiles. */
void widget_set_pixel(widget_t *p_widget, int x, int y, uint16_t pixel);
void widget_fill_region(widget_t *p_widget, int x, int y, int width, int height, uint16_t color);
void widget_draw_line(widget_t *p_widget, int x0, int y0, int x1, int y1, uint16_t color);
void widget_draw_circle(widget_t *p_widget, int x, int y, int r, uint16_t color);
void widget_draw_disc(widget_t *p_widget, int x, int y, int r, uint16_t color);
void widget_bitblt(widget_t *p_widget, image_t *source, int source_x, int source_y, int width, int height, int dest_x, int dest_y);
void widget_draw_char(widget_t *p_widget, int x, int y, char c, uint16_t color);
void widget_draw_text(widget_t *p_widget, int x, int y, char *psz_text, uint16_t color);
void widget_draw_char_x2(widget_t *p_widget, int x, int y, char c, uint16_t color);
void widget_draw_text_x2(widget_t *p_widget, int x, int y, char *psz_text, uint16_t color);

#endif /* __INC_UI_WIDGET_H */
