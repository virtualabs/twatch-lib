#include "ui/frame.h"


/**
 * widget_frame_drawfunc()
 * 
 * @brief: Callback function that renders a frame on screen.
 * @param p_widget: pointer to a `widget_t` structure.
 **/

int widget_frame_drawfunc(widget_t *p_widget)
{
  widget_frame_t *p_frame = (widget_frame_t *)p_widget->p_user_data;

  if (p_frame != NULL)
  {
    /* Draw the button. */
    widget_draw_line(
      p_widget,
      1,
      0,
      p_widget->box.width - 2,
      0,
      p_widget->style.border
    );
    widget_draw_line(
      p_widget,
      1,
      p_widget->box.height - 1,
      p_widget->box.width - 2,
      p_widget->box.height - 1,
      p_widget->style.border
    );
    widget_draw_line(
      p_widget,
      0,
      1,
      0,
      p_widget->box.height - 2,
      p_widget->style.border
    );
    widget_draw_line(
      p_widget,
      p_widget->box.width - 1,
      1,
      p_widget->box.width - 1,
      p_widget->box.height - 2,
      p_widget->style.border
    );
  }
  /* Success. */
  return TE_PROCESSED;
}


/**
 * widget_frame_init()
 * 
 * @brief: Initialize a frame widget.
 * @param p_widget_button: pointer to a `widget_frame_t` structure
 * @param p_tile: pointer to a `tile_t` structure
 * @param x: X coordinate
 * @param y: Y coordinate
 * @param width: frame width
 * @param height: frame height
 **/

void widget_frame_init(
  widget_frame_t *p_widget_frame,
  tile_t *p_tile,
  int x,
  int y,
  int width,
  int height
)
{
  /* Initialize the underlying widget. */
  widget_init(&p_widget_frame->widget, p_tile, x, y, width, height);

  /* Set user data. */
  widget_set_userdata(&p_widget_frame->widget, (void *)p_widget_frame);

  /* Set drawing function. */
  widget_set_drawfunc(&p_widget_frame->widget, widget_frame_drawfunc);
}