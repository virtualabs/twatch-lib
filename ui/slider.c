#include "ui/slider.h"

/**
 * widget_slider_drawfunc()
 *
 * @brief: render function for slider bar widget
 * @param p_widget: pointer to a `widget_t` structure
 **/

void widget_slider_drawfunc(widget_t *p_widget)
{
  int x, y;
  widget_slider_t *p_slider = (widget_slider_t *)p_widget->p_user_data;

  if (p_slider != NULL)
  {
    /* Put the bar in the middle */
    x = SLIDER_CURSOR_RADIUS + (p_slider->value - p_slider->min) * (p_widget->box.width - 2*SLIDER_CURSOR_RADIUS) / (p_slider->max - p_slider->min - SLIDER_CURSOR_RADIUS*2);
    y = p_widget->box.height/2;

    /* Draw the slider bar. */
    widget_draw_line(
      p_widget,
      SLIDER_CURSOR_RADIUS,
      y,
      p_widget->box.width-SLIDER_CURSOR_RADIUS*2,
      y,
      SLIDER_STYLE_BORDER
    );


    /* Draw bar left rounded edge of the bar. */
    widget_draw_disc(p_widget, SLIDER_CURSOR_RADIUS + SLIDER_FILL_RADIUS/2, y, SLIDER_FILL_RADIUS, SLIDER_STYLE_CURSOR);

    /* Draw the rest of the bar */
    widget_fill_region(
      p_widget,
      SLIDER_CURSOR_RADIUS + SLIDER_FILL_RADIUS/2,
      y - SLIDER_FILL_RADIUS,
      x,
      SLIDER_FILL_RADIUS*2 + 1,
      SLIDER_STYLE_FILL
    );

    /* Draw cursor. */
    widget_draw_disc(p_widget, x, y, SLIDER_CURSOR_RADIUS, SLIDER_STYLE_CURSOR);
  }
}

/**
 * widget_slider_event_handler()
 *
 * @brief: Event handler for slider bar
 * @param p_widget: pointer to a ̀`widget_t` structure
 * @param event: event type
 * @param x: X coordinate of the event
 * @param y: Y coordinate of the event
 * @param velocity: swipe velocity if event is a swipe event, 0 otherwise.
 * @return: WE_PROCESSED if event has been processed, WE_ERROR otherwise
 **/

int widget_slider_event_handler(widget_t *p_widget, widget_event_t event, int x, int  y, int velocity)
{
  bool b_processed = false;
  widget_slider_t *p_slider = (widget_slider_t *)p_widget->p_user_data;

  if (p_slider != NULL && p_slider->pfn_tap_handler != NULL)
  {
    switch(event)
    {
      case WE_PRESS:
        {
          /* Compute value from x position */
          if ((x >= SLIDER_CURSOR_RADIUS) && (x<=(p_widget->box.width - SLIDER_CURSOR_RADIUS)))
          {
            int old = p_slider->value;
            int new = x * (p_slider->max - p_slider->min) / (p_widget->box.width-2*SLIDER_CURSOR_RADIUS) + p_slider->min;
            p_slider->value = new;
            p_slider->pfn_tap_handler(&p_slider->widget /*, new, old */);
          }
          else if (x<SLIDER_CURSOR_RADIUS)
          {
            p_slider->value = p_slider->min;
          }
          else if (x > (p_widget->box.width - SLIDER_CURSOR_RADIUS))
          {
            p_slider->value = p_slider->max;
          }
          b_processed = true;
        }
        break;

      /* Catch swipe event to avoid interferance with value setting */
      case WE_SWIPE_LEFT:
      case WE_SWIPE_RIGHT:
        {
          b_processed = true;
        }

      default:
        break;
    }

    /* Notify UI if event has been processed or not. */
    return b_processed?WE_PROCESSED:WE_ERROR;
  }

  /* Event not processed. */
  return WE_ERROR;
}

/**
 * widget_slider_init()
 *
 * @brief: Initialize a slider bar widget
 * @param p_widget_slider: Pointer to a `widget_slider_t` structure containing widget info
 * @param p_tile: Pointer to a `tile_t` structure if widget belongs to a tile. Can be NULL.
 * @param x: Widget x coordinate.
 * @param y: Widget y coordinate.
 * @param width: Widget width in pixels.
 * @param height: Widget height in pixels.
 **/
void widget_slider_init(widget_slider_t *p_widget_slider, tile_t *p_tile, int x, int y, int width, int height)
{
  /* Initialize the underlying widget. */
  widget_init(&p_widget_slider->widget, p_tile, x, y, width, height);

  /* Set properties. */
  p_widget_slider->min = SLIDER_MIN_DEFAULT;
  p_widget_slider->max = SLIDER_MAX_DEFAULT;
  p_widget_slider->value = SLIDER_VALUE_DEFAULT;

  /* Set user data. */
  widget_set_userdata(&p_widget_slider->widget, (void *)p_widget_slider);

  /* Set event handler. */
  widget_set_eventhandler(&p_widget_slider->widget, widget_slider_event_handler);

  /* Set drawing function. */
  widget_set_drawfunc(&p_widget_slider->widget, widget_slider_drawfunc);

}

/**
 * widget_slider_configure()
 *
 * @brief: Configure slider bar.
 * @param p_widget_slider: pointer to a `widget_slider_t` structure representing the widget.
 * @param min: Slider bar minimum value.
 * @param max: Slider bar maximum value.
 * @param value: Slider bar actual value.
 **/

void widget_slider_configure(widget_slider_t *p_widget_slider, int min, int max, int value, int step)
{
  int tmp;

  /* Ensure max>min */
  if (max < min)
  {
    tmp = min;
    min = max;
    max = tmp;
  }

  /* Set min/max values. */
  p_widget_slider->min = min;
  p_widget_slider->max = max;
  p_widget_slider->pfn_tap_handler = NULL;

  /* Set current value. */
  widget_slider_set_value(p_widget_slider, value);
}

/**
 * widget_slider_set_handler()
 *
 * @brief: Set slider handler that will called on press
 * @param p_widget_slider: pointer to a `widget_slider_t` structure
 * @param pfn_handler: pointer to the associated handler (function)
 **/

void widget_slider_set_handler(widget_slider_t *p_widget_slider, FTapHandler pfn_handler)
{
  p_widget_slider->pfn_tap_handler = pfn_handler;
}

/**
 * widget_slider_set_value()
 *
 * @brief: Set slider bar current value.
 * @param p_widget_slider: pointer to a `widget_slider_t` structure.
 * @param value: slider bar value to set.
 **/

void widget_slider_set_value(widget_slider_t *p_widget_slider, int value)
{
  if (value < p_widget_slider->min)
  {
    p_widget_slider->value = p_widget_slider->min;
  }
  else if (value > p_widget_slider->max)
  {
    p_widget_slider->value = p_widget_slider->max;
  }
  else {
    p_widget_slider->value = value;
  }
}

/**
 * widget_slider_get_value()
 *
 * @brief: Get slider bar value.
 * @param p_widget_slider: Pointer to a `widget_slider_t` structure.
 * @return: Slider bar value.
 **/

int widget_slider_get_value(widget_slider_t *p_widget_slider)
{
  return p_widget_slider->value;
}
