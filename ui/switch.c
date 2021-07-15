#include "ui/switch.h"


/**
 * widget_switch_drawfunc()
 * 
 * @brief: Callback function that renders a switch on screen.
 * @param p_widget: pointer to a `widget_t` structure.
 **/

int widget_switch_drawfunc(widget_t *p_widget)
{
  int text_width, dx;
  widget_switch_t *p_switch = (widget_switch_t *)p_widget->p_user_data;

  if (p_switch != NULL)
  {
    /* Draw the borders. */
    widget_draw_line(
      p_widget,
      1,
      0,
      p_widget->box.width - 2,
      0,
      (p_switch->state==SWITCH_ON)?p_widget->style.front:p_widget->style.border
    );
    widget_draw_line(
      p_widget,
      1,
      p_widget->box.height - 1,
      p_widget->box.width - 2,
      p_widget->box.height - 1,
      (p_switch->state==SWITCH_ON)?p_widget->style.front:p_widget->style.border
    );
    widget_draw_line(
      p_widget,
      0,
      1,
      0,
      p_widget->box.height - 2,
      (p_switch->state==SWITCH_ON)?p_widget->style.front:p_widget->style.border
    );
    widget_draw_line(
      p_widget,
      p_widget->box.width - 1,
      1,
      p_widget->box.width - 1,
      p_widget->box.height - 2,
      (p_switch->state==SWITCH_ON)?p_widget->style.front:p_widget->style.border
    );

    /* Draw ON and OFF texts. */
    text_width = font_get_text_width("ON");
    dx = ((p_widget->box.width/2) - text_width)/2;
    widget_draw_text(
      p_widget,
      dx,
      (p_widget->box.height - 16)/2,
      "ON",
      p_widget->style.border
    );

    text_width = font_get_text_width("OFF");
    dx = ((p_widget->box.width/2) - text_width)/2 + (p_widget->box.width/2);
    widget_draw_text(
      p_widget,
      dx,
      (p_widget->box.height - 16)/2,
      "OFF",
      p_widget->style.border
    );

    if (p_switch->state == SWITCH_ON)
    {
      dx = p_widget->box.width/2;
    }
    else
    {
      dx = 0;
    }

    widget_fill_region(
      p_widget,
      dx,
      1,
      p_widget->box.width/2,
      p_widget->box.height-2,
      (p_switch->state==SWITCH_ON)?p_widget->style.front:p_widget->style.border
    );

  }

  /* Success. */
  return TE_PROCESSED;
}


/**
 * widget_switch_event_handler()
 * 
 * @brief: Event handler for switch
 * @param p_widget: pointer to a ̀`widget_t` structure
 * @param event: event type
 * @param x: X coordinate of the event
 * @param y: Y coordinate of the event
 * @param velocity: swipe velocity if event is a swipe event, 0 otherwise.
 * @return: WE_PROCESSED if event has been processed, WE_ERROR otherwise
 **/

int widget_switch_event_handler(widget_t *p_widget, widget_event_t event, int x, int  y, int velocity)
{
  bool b_processed = false;
  widget_switch_t *p_switch = (widget_switch_t *)p_widget->p_user_data;

  if (p_switch != NULL)
  {
    switch(event)
    {
      /* switch has been quickly pressed and released. */
      case WE_TAP:
        {
          /* Toggle switch state. */
          p_switch->state = !p_switch->state;

          /* Call our callback function, if set. */
          if (p_switch->pfn_tap_handler != NULL)
          {
            p_switch->pfn_tap_handler(p_widget);
          }

          b_processed = true;
        }
        break;

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
 * widget_switch_init()
 * 
 * @brief: Initialize a switch widget.
 * @param p_widget_switch: pointer to a `widget_switch_t` structure
 * @param p_tile: pointer to a `tile_t` structure
 * @param x: X coordinate
 * @param y: Y coordinate
 * @param width: switch width
 * @param height: switch height
 **/

void widget_switch_init(
  widget_switch_t *p_widget_switch,
  tile_t *p_tile,
  int x,
  int y,
  int width,
  int height
)
{
  /* Initialize the underlying widget. */
  widget_init(&p_widget_switch->widget, p_tile, x, y, width, height);

  /* Set properties. */
  p_widget_switch->state = SWITCH_OFF;

  /* Set user callbacks. */
  p_widget_switch->pfn_tap_handler = NULL;

  /* Set default style. */
  p_widget_switch->widget.style.background = SWITCH_STYLE_BG;
  p_widget_switch->widget.style.border = SWITCH_STYLE_BORDER;
  p_widget_switch->widget.style.front = SWITCH_STYLE_BORDER;

  /* Set user data. */
  widget_set_userdata(&p_widget_switch->widget, (void *)p_widget_switch);

  /* Set drawing function. */
  widget_set_drawfunc(&p_widget_switch->widget, widget_switch_drawfunc);

  /* Set default event handler. */
  widget_set_eventhandler(&p_widget_switch->widget, widget_switch_event_handler);

}


/**
 * widget_switch_set_handler()
 * 
 * @brief: Set switch handler that will called on press
 * @param p_widget_switch: pointer to a `widget_switch_t` structure
 * @param pfn_handler: pointer to the associated handler (function)
 **/

void widget_switch_set_handler(widget_switch_t *p_widget_switch, FTapHandler pfn_handler)
{
  p_widget_switch->pfn_tap_handler = pfn_handler;
}


/**
 * widget_switch_set_state()
 * 
 * @brief: Set switch state
 * @param p_widget_switch: pointer to a `widget_switch_t` structure
 * @param state: new switch state
 **/

void widget_switch_set_state(widget_switch_t *p_widget_switch, switch_state_t state)
{
  p_widget_switch->state = state;
}


/**
 * widget_switch_get_state()
 * 
 * @brief: Get switch state
 * @param p_widget_switch: pointer to a `widget_switch_t` structure
 * @return: current switch state
 **/

switch_state_t widget_switch_get_state(widget_switch_t *p_widget_switch)
{
  return p_widget_switch->state;
}