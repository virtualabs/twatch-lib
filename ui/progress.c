#include "ui/progress.h"


/**
 * widget_progress_drawfunc()
 * 
 * @brief: render function for progress bar widget
 * @param p_widget: pointer to a `widget_t` structure
 **/

int widget_progress_drawfunc(widget_t *p_widget)
{
  widget_progress_t *p_progress = (widget_progress_t *)p_widget->p_user_data;

  if (p_progress != NULL)
  {
    /* Draw the progress bar. */
    widget_draw_line(
      p_widget,
      1,
      0,
      p_widget->box.width - 2,
      0,
      PROGRESS_STYLE_BORDER
    );
    widget_draw_line(
      p_widget,
      1,
      p_widget->box.height - 1,
      p_widget->box.width - 2,
      p_widget->box.height - 1,
      PROGRESS_STYLE_BORDER
    );
    widget_draw_line(
      p_widget,
      0,
      1,
      0,
      p_widget->box.height - 2,
      PROGRESS_STYLE_BORDER
    );
    widget_draw_line(
      p_widget,
      p_widget->box.width - 1,
      1,
      p_widget->box.width - 1,
      p_widget->box.height - 2,
      PROGRESS_STYLE_BORDER
    );

    /* Draw progress bar background. */
    widget_fill_region(
      p_widget,
      1,
      1,
      p_widget->box.width-2,
      p_widget->box.height-2,
      PROGRESS_STYLE_BG
    );

    /* Draw bar . */
    widget_fill_region(
      p_widget,
      2,
      2,
      (p_progress->value - p_progress->min) * (p_widget->box.width-4) / (p_progress->max - p_progress->min),
      p_widget->box.height-4,
      RGB(0xe,0xe,0xe)
    );
  }
  /* Success. */
  return TE_PROCESSED;
}


/**
 * widget_progress_init()
 * 
 * @brief: Initialize a progress bar widget
 * @param p_widget_progress: Pointer to a `widget_progress_t` structure containing widget info
 * @param p_tile: Pointer to a `tile_t` structure if widget belongs to a tile. Can be NULL.
 * @param x: Widget x coordinate.
 * @param y: Widget y coordinate.
 * @param width: Widget width in pixels.
 * @param height: Widget height in pixels.
 **/

void widget_progress_init(widget_progress_t *p_widget_progress, tile_t *p_tile, int x, int y, int width, int height)
{
  /* Initialize the underlying widget. */
  widget_init(&p_widget_progress->widget, p_tile, x, y, width, height);

  /* Set properties. */
  p_widget_progress->min = PROGRESS_MIN_DEFAULT;
  p_widget_progress->max = PROGRESS_MAX_DEFAULT;
  p_widget_progress->value = PROGRESS_VALUE_DEFAULT;

  /* Set user data. */
  widget_set_userdata(&p_widget_progress->widget, (void *)p_widget_progress);

  /* Set drawing function. */
  widget_set_drawfunc(&p_widget_progress->widget, widget_progress_drawfunc);

}


/**
 * widget_progress_configure()
 * 
 * @brief: Configure progress bar.
 * @param p_widget_progress: pointer to a `widget_progress_t` structure representing the widget.
 * @param min: Progress bar minimum value.
 * @param max: Progress bar maximum value.
 * @param value: Progress bar actual value.
 **/

void widget_progress_configure(widget_progress_t *p_widget_progress, int min, int max, int value)
{
  int x;

  /* Ensure max>min */
  if (max < min)
  {
    x = min;
    min = max;
    max = x;
  }

  /* Set min/max values. */
  p_widget_progress->min = min;
  p_widget_progress->max = max;

  /* Set current value. */
  widget_progress_set_value(p_widget_progress, value);
}


/**
 * widget_progress_set_value()
 * 
 * @brief: Set progress bar current value.
 * @param p_widget_progress: pointer to a `widget_progress_t` structure.
 * @param value: progress bar value to set.
 **/

void widget_progress_set_value(widget_progress_t *p_widget_progress, int value)
{
  if ((value >= p_widget_progress->min) && (value <= p_widget_progress->max))
  {
    /* Set current value. */
    p_widget_progress->value = value;
  }
  else
    /* Wrong value, default to min value. */
    p_widget_progress->value = p_widget_progress->min;
}


/**
 * widget_progress_get_value()
 * 
 * @brief: Get progress bar value.
 * @param p_widget_progress: Pointer to a `widget_progress_t` structure.
 * @return: Progress bar value.
 **/

int widget_progress_get_value(widget_progress_t *p_widget_progress)
{
  return p_widget_progress->value;
}