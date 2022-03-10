#include "ui/listbox.h"
#include "esp_log.h"

#define TAG "listbox"


/**
 * widget_listbox_animate()
 * 
 * @brief: Animate the listbox based on its scrolling speed
 * @param p_listbox: pointer to a `widget_listbox_t`
 **/

void widget_listbox_animate(widget_listbox_t *p_listbox)
{
  /* Determine a dy based on current scrolling speed. */
  if (p_listbox->speed < 0)
  {
    if (p_listbox->offset < 0)
    {
      p_listbox->offset -= (int)(p_listbox->speed/4);
      if (p_listbox->offset >= 0)
      {
        /* Animation done, switch to idle. */
        p_listbox->offset = 0;
        p_listbox->state = LB_STATE_IDLE;
      }
    }

    /* Adapt speed. */
    p_listbox->speed = (9.8*p_listbox->speed)/10.0;
  }
  else if (p_listbox->speed > 0)
  {
    if (p_listbox->offset > -(p_listbox->scrollbar.max - 20))
    {
      p_listbox->offset -= (int)(p_listbox->speed/4);
      if (p_listbox->offset <= -(p_listbox->scrollbar.max - 20))
      {
        /* Animation done, switch to idle. */
        p_listbox->offset = -(p_listbox->scrollbar.max - 20);
        p_listbox->state = LB_STATE_IDLE;
      }
    }

    /* Adapt speed. */
    p_listbox->speed = (9.8*p_listbox->speed)/10.0;
  }
  else
  {
    /* Animation done, switch to idle. */
    p_listbox->offset = -(p_listbox->scrollbar.max - 20);
    p_listbox->state = LB_STATE_IDLE;
  }
}


/**
 * widget_listbox_drawfunc()
 * 
 * @brief: Specific rendering function for our listbox widget.
 * @param p_widget: pointer to a `widget_t` structure
 **/

int widget_listbox_drawfunc(widget_t *p_widget)
{
  widget_listbox_t *p_listbox = (widget_listbox_t *)p_widget->p_user_data;

  if (p_listbox != NULL)
  {
    /* Position container (position not updated while animating). */
    p_listbox->container.widget.box.x = widget_get_abs_x(p_widget) + 1;
    p_listbox->container.widget.box.y = widget_get_abs_y(p_widget) + 1;

    /* Animate container if required. */
    if (p_listbox->state == LB_STATE_IDLE)
    {
      p_listbox->container.offset_y = p_listbox->offset;
      p_listbox->scrollbar.value = -p_listbox->offset;
    }
    else
    {
      widget_listbox_animate(p_listbox);
      p_listbox->container.offset_y = p_listbox->offset;
      p_listbox->scrollbar.value = -p_listbox->offset;
    }

    /* Position scrollbar (position not updated while animating). */
    p_listbox->scrollbar.widget.box.x = widget_get_abs_x(p_widget) + p_listbox->widget.box.width - 10;
    p_listbox->scrollbar.widget.box.y = widget_get_abs_y(p_widget);

    /* Draw container. */
    widget_draw((widget_t *)&p_listbox->container);

    /* Draw scrollbar. */
    widget_draw((widget_t *)&p_listbox->scrollbar);

    /* Draw listbox borders. */
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
 * widget_listbox_event_handler()
 * 
 * @brief: UI event handler for lisbox
 * @param p_widget: pointer to a `widget_t` structure
 * @param event: UI event
 * @param x: x coordinate (if event is a touch/scroll event)
 * @param y: y coordinate (if event is a touch/scroll event)
 * @param velocity: scroll velocity, 0 if not required
 * @return: WE_ERROR if event has not been processed, WE_PROCESSED otherwise.
 **/

int widget_listbox_event_handler(widget_t *p_widget, widget_event_t event, int x, int  y, int velocity)
{
  bool b_processed = false;
  widget_container_item_t *p_item = NULL;
  widget_listbox_t *p_listbox = (widget_listbox_t *)p_widget->p_user_data;

  if (p_listbox != NULL)
  {
    switch(event)
    {
      /* An item of this listbox has been tapped. */
      case WE_TAP:
        {
          if (p_listbox->state == LB_STATE_STOPPED)
          {
            /* Stop animation. */
            p_listbox->state = LB_STATE_IDLE;
          }
          else
          {
            /* Check if coordinates match a widget. */
            p_item = p_listbox->container.p_children;
            if (p_item != NULL)
            {
              do
              {
                /* Check if tap happened in this widget. */
                if (
                  (p_item->p_widget->box.x <= (x + p_widget->box.x)) &&
                  ((p_item->p_widget->box.x + p_item->p_widget->box.width) > (x + p_widget->box.x)) &&
                  (p_item->p_widget->box.y <= (y+p_widget->box.y)) &&
                  ((p_item->p_widget->box.y + p_item->p_widget->box.height) > (y+p_widget->box.y))
                )
                {
                  /* Deselect previous item if any. */
                  if (p_listbox->p_selected_item != NULL)
                    widget_send_event(p_listbox->p_selected_item, LB_ITEM_DESELECTED, 0, 0, 0);

                  /* Select this item. */
                  widget_send_event(p_item->p_widget, LB_ITEM_SELECTED, 0, 0, 0);
                  p_listbox->p_selected_item = p_item->p_widget;

                  /* Notify hooks with specific event. */
                  widget_send_event(p_widget, LB_ITEM_SELECTED, 0, 0, 0);
                }
                p_item = p_item->p_next;
              }
              while (p_item != NULL);
            }
          }

          /* Event has been processed. */
          b_processed = true;
        }
        break;

      /* Listbox has been pressed, stop scrolling. */
      case WE_PRESS:
        {
          if (p_listbox->state == LB_STATE_MOVING_FREE)
          {
            p_listbox->state = LB_STATE_STOPPED;
            b_processed = true;
          }
        }
        break;

      /* User has swiped up. */
      case WE_SWIPE_UP:
        {
          ESP_LOGI(TAG, "[listbox] event: swipe up (%d)", velocity);
          if (p_listbox->offset == -(p_listbox->scrollbar.max - 20))
          {
            /* No animation. */
            p_listbox->state = LB_STATE_IDLE;
          }
          else
          {
            /* Animate. */
            p_listbox->state = LB_STATE_MOVING;
            p_listbox->speed = velocity;

          }
          b_processed = true;
        }
        break;

      /* User has swiped down. */
      case WE_SWIPE_DOWN:
        {
          ESP_LOGI(TAG, "[listbox] event: swipe down (%d)", velocity);
          if (p_listbox->offset == 0)
          {
            /* No animation. */
            p_listbox->state = LB_STATE_IDLE;
          }
          else
          {
            /* Animate. */
            p_listbox->state = LB_STATE_MOVING;
            p_listbox->speed = -velocity;

          }
          b_processed = true;
        }
        break;

      case WE_RELEASE:
        {
          if (p_listbox->state == LB_STATE_MOVING)
          {
            p_listbox->state = LB_STATE_MOVING_FREE;
          }
        }
        break;

      default:
      {
        p_listbox->state = LB_STATE_IDLE;
        b_processed = false;
      }
      break;
    }

    /* Notify UI if event has been processed or not. */
    return b_processed?WE_PROCESSED:WE_ERROR;
  }

  /* Event not processed. */
  return WE_ERROR;
}


/**
 * update_scrollbar()
 * 
 * @brief: Update listbox' scrollbar
 * @param p_widget_listbox: pointer to a `widget_listbox_t` structure
 **/

void update_scrollbar(widget_listbox_t *p_widget_listbox)
{
  widget_container_item_t *p_item = NULL;
  int height = 0;

  /* Compute total height and update scrollbar values. */
  if (p_widget_listbox->container.p_children != NULL)
  {
    p_item = p_widget_listbox->container.p_children;
    height = p_item->rel_box.y;
    while (p_item->p_next != NULL)
    {
      height += p_item->rel_box.height;
      p_item = p_item->p_next;
    }

    if (p_item != NULL)
    {
      height += p_item->rel_box.height;
    }
  }

  p_widget_listbox->scrollbar.max = height;
}


/**
 * widget_listbox_init()
 * 
 * @brief: Initialize a listbox widget
 * @param p_widget_listbox: Pointer to a `widget_listbox_t` structure
 * @param p_tile: Pointer to a `tile_t` structure
 * @param x: X position of the widget
 * @param y: Y position of the widget
 * @param width: widget width
 * @param height: widget height
 **/

void widget_listbox_init(widget_listbox_t *p_widget_listbox, tile_t *p_tile, int x, int y, int width, int height)
{
  /* Initialize our widget. */
  widget_init(&p_widget_listbox->widget, p_tile, x, y, width, height);

  /* Initialize our container (width = listbox width - scrollbar width). */
  widget_container_init(
    &p_widget_listbox->container,
    NULL,
    x + 1,
    y + 1,
    width - 12,
    height - 2
  );

  widget_scrollbar_init(
    &p_widget_listbox->scrollbar,
    NULL,
    width-10,
    0,
    10,
    height,
    SCROLLBAR_VERTICAL
  );

  /* No items. */
  p_widget_listbox->n_items = 0;
  p_widget_listbox->p_selected_item = NULL;

  /* Animation. */
  p_widget_listbox->state = LB_STATE_IDLE;
  p_widget_listbox->move_orig_x = 0;
  p_widget_listbox->move_orig_y = 0;
  p_widget_listbox->offset = 0;

  /* Set user data. */
  widget_set_userdata(&p_widget_listbox->widget, (void *)p_widget_listbox);

  /* Set drawing function. */
  widget_set_drawfunc(&p_widget_listbox->widget, widget_listbox_drawfunc);

  /* Set default event handler. */
  widget_set_eventhandler(&p_widget_listbox->widget, widget_listbox_event_handler);
}


/**
 * widget_listbox_add()
 * 
 * @brief: Add widget to listbox
 * @param p_widget_listbox: Pointer to a `widget_listbox_t` structure
 * @param p_widget: Pointer to a `widget_t` structure
 **/

void widget_listbox_add(widget_listbox_t *p_widget_listbox, widget_t *p_widget)
{
  widget_container_item_t *p_item = NULL;

  /* Does our container contains at least one item ? */
  if (p_widget_listbox->container.p_children != NULL)
  {
    /* Yes, append our widget after the last item. */
    p_item = p_widget_listbox->container.p_children;
    while (p_item->p_next != NULL)
      p_item = p_item->p_next;
  }
  
  /* Update our new widget position and width based on previous widget. */
  p_widget->box.x = 2;
  if (p_item != NULL)
  {
    p_widget->box.y = p_item->rel_box.y + p_item->rel_box.height;
  }
  else
  {
    p_widget->box.y = 2;
  }

  p_widget->box.width = p_widget_listbox->container.widget.box.width - 4;

  /* Add widget to container. */
  widget_container_add(&p_widget_listbox->container, p_widget);

  /* Update scrollbar. */
  update_scrollbar(p_widget_listbox);
}


/**
 * widget_listbox_remove()
 * 
 * @brief: Remove widget from listbox.
 * @param p_widget_listbox: pointer to a `widget_listbox_t` structure.
 * @param p_widget: pointer to a `widget_t`structure representing the widget to remove.
 **/

void widget_listbox_remove(widget_listbox_t *p_widget_listbox, widget_t *p_widget)
{
  widget_container_item_t *p_item = NULL;

  /* Remove widget. */
  widget_container_remove(&p_widget_listbox->container, p_widget);

  /* Update widgets relative box. */
  if (p_widget_listbox->container.p_children != NULL)
  {
    p_item = p_widget_listbox->container.p_children;
    while (p_item != NULL)
    {
      /* Ensure first item starts at the correct position. */
      if (p_item == p_widget_listbox->container.p_children)
      {
        p_item->rel_box.y = 2;
      }
      
      /* Following item must be updated, if any. */
      if (p_item->p_next != NULL)
      {
        p_item->p_next->rel_box.y = p_item->rel_box.y + p_item->rel_box.height;
      }

      /* Process next item. */
      p_item = p_item->p_next;
    }
  }

  /* Update scrollbar. */
  update_scrollbar(p_widget_listbox);
}


/**
 * widget_listbox_count()
 * 
 * @brief: Return the number of items in a listbox
 * @param p_widget_listbox: pointer to a `widget_listbox_t` structure.
 * @return: number of items present in the listbox
 **/

int widget_listbox_count(widget_listbox_t *p_widget_listbox)
{
  int n=0;
  widget_container_item_t *p_item = NULL;

  if (p_widget_listbox->container.p_children != NULL)
  {
    n = 1;
    p_item = p_widget_listbox->container.p_children;
    while (p_item->p_next != NULL)
    {
      n++;
      p_item = p_item->p_next;
    }
  }

  return n;
}