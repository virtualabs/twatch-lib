#include "esp_log.h"
#include "ui/container.h"

#define TAG "ui::container"
#define WIDGET(x) (widget_t *)(&x->widget)


/**
 * widhget_container_drawfunc()
 * 
 * @brief: Container drawing function (callback)
 * @param p_widget: pointer to a `widget_t` structure
 **/

int widget_container_drawfunc(widget_t *p_widget)
{
  widget_container_item_t *p_widget_item;

  /* Retrieve our container structure. */
  widget_container_t *p_container = (widget_container_t *)p_widget->p_user_data;

  if (p_container != NULL)
  {
    /* Draw container background. */
    widget_fill_region(
      p_widget,
      1,
      1,
      p_widget->box.width-2,
      p_widget->box.height-2,
      RGB(0,0,0)
    );
    
    /* Restrict our drawing window to our box. */
    st7789_set_drawing_window(
      widget_get_abs_x(&p_container->widget)+1,
      widget_get_abs_y(&p_container->widget)+1,
      widget_get_abs_x(&p_container->widget) + p_container->widget.box.width-2,
      widget_get_abs_y(&p_container->widget) + p_container->widget.box.height-2
    );

    /* Loop on our widgets and draw them ! */
    p_widget_item = p_container->p_children;
    while(p_widget_item != NULL)
    {
      /* Position widget. */
      p_widget_item->p_widget->box.x = p_widget_item->rel_box.x + widget_get_abs_x(&p_container->widget) + p_container->offset_x;
      p_widget_item->p_widget->box.y = p_widget_item->rel_box.y + widget_get_abs_y(&p_container->widget) + p_container->offset_y;

      /* Draw widget. */
      widget_draw(p_widget_item->p_widget);

      /* next widget */
      p_widget_item = p_widget_item->p_next;
    }

    /* Restore drawing window. */
    st7789_set_drawing_window(
      0,
      0,
      SCREEN_WIDTH - 1,
      SCREEN_HEIGHT - 1
    );
  }
  return 0;
}


/**
 * widget_container_event_handler()
 * 
 * @brief: Container event handler (callback)
 * @param p_widget: pointer to a `widget_t` structure
 * @param event: event type
 * @param x: X coordinate
 * @param y: Y coordinate
 * @param velocity: velocity in case of a swipe event, 0 otherwise
 * @return: WE_PROCESSED if the event has been processed, WE_ERROR otherwise.
 **/

int widget_container_event_handler(widget_t *p_widget, widget_event_t event, int x, int y, int velocity)
{
  widget_container_item_t *p_item;
  widget_container_t *p_container = (widget_container_t *)p_widget->p_user_data;

  if (p_container != NULL)
  {
    /* Forward event to the corresponding widget. */
    if (p_container->p_children != NULL)
    {
      p_item = p_container->p_children;
      while(p_item->p_next != NULL)
      {
        /* If WE_RELEASE event, forward to all widgets. */
        if (event == WE_RELEASE)
          widget_send_event(p_item->p_widget, event, x, y, velocity);
        else
        {
          /* Does this event concern this widget ? */
          if (
            (x >= (p_item->p_widget->box.x )) && \
            (y >= (p_item->p_widget->box.y)) && \
            (x < (p_item->p_widget->box.x + p_item->p_widget->box.width)) && \
            (y < (p_item->p_widget->box.y + p_item->p_widget->box.height))
          )
          {
            /* Forward the touch event to the widget. */
            if (widget_send_event(p_item->p_widget, (widget_event_t)event, x, y, velocity) == WE_PROCESSED)
              return WE_PROCESSED;
          }
        }
        
        p_item = p_item->p_next;
      }
    }
  }

  /* Event not processed. */
  return WE_ERROR;
}


/**
 * widget_container_init()
 * 
 * @brief: Initialize a container widget
 * @param p_widget_container: pointer to a `widget_container_t` structure
 * @param p_tile: pointer to a `tile_t` structure (parent tile)
 * @param x: widget X coordinate
 * @param y: widget Y coordinate
 * @param width: widget width
 * @param height: widget height
 **/

void widget_container_init(widget_container_t *p_widget_container, tile_t *p_tile, int x, int y, int width, int height)
{
  /* Initialize the underlying widget. */
  widget_init(&p_widget_container->widget, p_tile, x, y, width, height);

  /* Set properties (no children) */
  p_widget_container->p_children = NULL;
  p_widget_container->offset_x = 0;
  p_widget_container->offset_y = 0;
  
  /* Set user data. */
  widget_set_userdata(&p_widget_container->widget, (void *)p_widget_container);

  /* Set drawing function. */
  widget_set_drawfunc(&p_widget_container->widget, widget_container_drawfunc);

  /* Set default event handler. */
  widget_set_eventhandler(&p_widget_container->widget, widget_container_event_handler);
}


/**
 * widget_container_add()
 * 
 * @brief: add a widget to a container
 * @param p_widget_container: pointer to a `widget_container_t` structure
 * @param p_widget: pointer to a `widget_t` structure
 **/

void widget_container_add(widget_container_t *p_widget_container, widget_t *p_widget)
{
  widget_container_item_t *p_item, *p_new_item;

  if (p_widget_container->p_children != NULL)
  {
    /* Go to the last widget item. */
    p_item = p_widget_container->p_children;
    while(p_item->p_next != NULL)
      p_item = p_item->p_next;

    /* Add our widget. */
    p_new_item = (widget_container_item_t *)malloc(sizeof(widget_container_item_t));
    if (p_new_item == NULL)
    {
      /* Error. */
      ESP_LOGE(TAG, "cannot allocate memory for a new container item");
    }
    else
    {
      /* Initialize our new item. */
      p_new_item->p_widget = p_widget;
      p_new_item->p_next = NULL;
      memcpy(&p_new_item->rel_box, &p_widget->box, sizeof(widget_box_t));

      /* Add item at the end of our chained list. */
      p_item->p_next = p_new_item;
    }
  }
  else
  {
    /* Add our widget. */
    p_new_item = (widget_container_item_t *)malloc(sizeof(widget_container_item_t));
    if (p_new_item == NULL)
    {
      /* Error. */
      ESP_LOGE(TAG, "cannot allocate memory for a new container item");
    }
    else
    {
      /* Initialize our new item. */
      p_new_item->p_widget = p_widget;
      p_new_item->p_next = NULL;
      memcpy(&p_new_item->rel_box, &p_widget->box, sizeof(widget_box_t));

      /* Add item at the end of our chained list. */
      p_widget_container->p_children = p_new_item;
    }
  }

}


/**
 * widget_container_remove()
 * 
 * @brief: Remove a widget from a container.
 * @param p_widget_container: pointer to a `widget_container_t` structure
 * @param p_widget: pointer to a `widget_t` structure of the widget to remove
 **/

void widget_container_remove(widget_container_t *p_widget_container, widget_t *p_widget)
{
  widget_container_item_t *p_item, *p_item_before;

  if (p_widget_container->p_children != NULL)
  {
    /* Go to the last widget item. */
    p_item_before = NULL;
    p_item = p_widget_container->p_children;
    while(p_item != NULL)
    {
      /* Should we remove this widget ? */
      if( p_item->p_widget == p_widget)
      {
        /* Skip the item to remove. */
        if (p_item_before != NULL)
        {
          p_item_before->p_next = p_item->p_next;
        }
        else
        {
          p_widget_container->p_children = p_item->p_next;
        }

        /* Remove item. */
        free(p_item);

        /* Exit while loop. */
        break;
      }
      
      /* Keep track of previous widget. */
      p_item_before = p_item;
      p_item = p_item->p_next;
    }
  }
}


/**
 * widget_debug_list()
 * 
 * @brief: Display the content of a container (list of widgets).
 * @param p_widget_container: pointer to a `widget_container_t` structure
 **/

void widget_debug_list(widget_container_t *p_widget_container)
{
  int n;
  widget_container_item_t *p_item;

  if (p_widget_container->p_children != NULL)
  {
    /* Display list items. */
    n = 0;
    p_item = p_widget_container->p_children;
    while (p_item != NULL)
    {
      printf("item #%d (@%08x): widget=@%08x, next=@%08x\r\n", n++, (uint32_t)p_item, (uint32_t)p_item->p_widget, (uint32_t)p_item->p_next);
      p_item = p_item->p_next;
    }
  }
  else
  {
    printf("list is empty !\r\n");
  }
}
