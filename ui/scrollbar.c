#include "ui/scrollbar.h"


/**
 * widget_scrollbar_drawfunc()
 * 
 * @brief: Scrollbar drawing function (callback)
 * @param p_widget: pointer to a `widget_t` structure
 **/

int widget_scrollbar_drawfunc(widget_t *p_widget)
{
  int cursor_size;
  int cursor_offset;
  widget_scrollbar_t *p_scrollbar = (widget_scrollbar_t *)p_widget->p_user_data;

  if (p_scrollbar != NULL)
  {
    /* Draw the scrollbar border. */
    widget_draw_line(
      p_widget,
      1,
      0,
      p_widget->box.width - 2,
      0,
      SCROLLBAR_STYLE_BORDER
    );
    widget_draw_line(
      p_widget,
      1,
      p_widget->box.height - 1,
      p_widget->box.width - 2,
      p_widget->box.height - 1,
      SCROLLBAR_STYLE_BORDER
    );
    widget_draw_line(
      p_widget,
      0,
      1,
      0,
      p_widget->box.height - 2,
      SCROLLBAR_STYLE_BORDER
    );
    widget_draw_line(
      p_widget,
      p_widget->box.width - 1,
      1,
      p_widget->box.width - 1,
      p_widget->box.height - 2,
      SCROLLBAR_STYLE_BORDER
    );

    /* Draw cursor, only vertical at the moment (TODO !). */
    if (p_scrollbar->max < p_scrollbar->widget.box.height)
    {
      cursor_size = p_scrollbar->widget.box.height / 2;
    }
    else
    {
      cursor_size = (p_scrollbar->widget.box.height * (p_scrollbar->widget.box.height/2))/p_scrollbar->max;
    }
    if (cursor_size < 10)
      cursor_size = 10;
    
    cursor_offset = (p_scrollbar->value * (p_scrollbar->widget.box.height - cursor_size)) / (p_scrollbar->max - p_scrollbar->min);
    widget_fill_region(&p_scrollbar->widget, 1, 1 + cursor_offset, p_scrollbar->widget.box.width-2, cursor_size, RGB(0xf, 0xf,0xf));
  }
  /* Success. */
  return TE_PROCESSED;
}

/**
 * widget_scrollbar_init()
 * 
 * @brief: Initialize a scrollbar widget
 * @param p_widget_scrollbar: pointer to a `widget_scrollbar_t` structure
 * @param p_tile: pointer to a `tile_t` structure (parent tile)
 * @param x: widget X coordinate
 * @param y: widget Y coordinate
 * @param widht: widget width
 * @param height: widget height
 * @param type: scrollbar type
 **/

void widget_scrollbar_init(
  widget_scrollbar_t *p_widget_scrollbar,
  tile_t *p_tile,
  int x,
  int y,
  int width,
  int height,
  widget_scrollbar_type_t type
)
{
  /* Initialize the underlying widget. */
  widget_init(&p_widget_scrollbar->widget, p_tile, x, y, width, height);

  /* Save parameters. */
  p_widget_scrollbar->min = 0;
  p_widget_scrollbar->max = 100;
  p_widget_scrollbar->value = 0;
  p_widget_scrollbar->type = type;

  /* Set user data. */
  widget_set_userdata(&p_widget_scrollbar->widget, (void *)p_widget_scrollbar);

  /* Set drawing function. */
  widget_set_drawfunc(&p_widget_scrollbar->widget, widget_scrollbar_drawfunc);
}