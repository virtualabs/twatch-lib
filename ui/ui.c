#include "ui/ui.h"
#include "ui/widget.h"

int ui_forward_event_to_widget(touch_event_type_t state, int x, int y, int velocity);

/**
 * Main interface structure.
 **/

ui_t g_ui;

/**
 * @brief: Initialize main UI
 **/

void ui_init(void)
{
  /* Initialize the touch screen. */
  twatch_touch_init();

  /* Set current tile as none. */
  g_ui.p_current_tile = NULL;

  /* Set animation state and parameters. */
  g_ui.state = UI_STATE_IDLE;
  g_ui.p_from_tile = NULL;
  g_ui.p_to_tile = NULL;

  /* Initialize our modal box. */
  g_ui.p_modal = NULL;
}


/**
 * @brief: Select the current tile
 * @param p_tile: pointer to a `tile_t` structure (the tile to select)
 **/

void ui_select_tile(tile_t *p_tile)
{
  if (g_ui.p_current_tile != NULL)
  {
    /* Send TE_ENTER to current tile. */
    tile_send_event(
      g_ui.p_current_tile,
      TE_EXIT,
      0,
      0,
      0
    );
  }

  /* Set current tile. */
  g_ui.p_current_tile = p_tile;

  /* Reset offsets in order to display this tile. */
  g_ui.p_current_tile->offset_x = 0;
  g_ui.p_current_tile->offset_y = 0;

  /* Send TE_ENTER to current tile. */
  tile_send_event(
    g_ui.p_current_tile,
    TE_ENTER,
    0,
    0,
    0
  );
}


void ui_swipe_right(void)
{
  tile_t *p_main_tile;
  
  /* Can we move to the left tile ? */
  if (g_ui.p_current_tile->t_type == TILE_SECONDARY)
  {
    /* Get the main tile. */
    p_main_tile = g_ui.p_current_tile;
    while (p_main_tile->p_top != NULL)
      p_main_tile = p_main_tile->p_top;
  }
  else
  {
    /* The current tile is a main tile, nothing to do. */
    p_main_tile = g_ui.p_current_tile;
  }

  if (p_main_tile->p_left != NULL)
  {
    /* Yes, setup animation. */
    g_ui.state = UI_STATE_MOVE_LEFT;
    g_ui.p_from_tile = g_ui.p_current_tile;
    g_ui.p_to_tile = p_main_tile->p_left;
    g_ui.p_to_tile->offset_x = -SCREEN_WIDTH;
    g_ui.p_to_tile->offset_y = 0;
  }
}


void ui_swipe_left(void)
{
  tile_t *p_main_tile;

  /* Can we move to the left tile ? */
  if (g_ui.p_current_tile->t_type == TILE_SECONDARY)
  {
    /* Get the main tile. */
    p_main_tile = g_ui.p_current_tile;
    while (p_main_tile->p_top != NULL)
      p_main_tile = p_main_tile->p_top;
  }
  else
  {
    /* The current tile is a main tile, nothing to do. */
    p_main_tile = g_ui.p_current_tile;
  }

  /* Can we move to the left tile ? */
  if (p_main_tile->p_right != NULL)
  {
    /* Yes, setup animation. */
    g_ui.state = UI_STATE_MOVE_RIGHT;
    g_ui.p_from_tile = g_ui.p_current_tile;
    g_ui.p_to_tile = p_main_tile->p_right;
    g_ui.p_to_tile->offset_x = SCREEN_WIDTH;
    g_ui.p_to_tile->offset_y = 0;
  }
}

void ui_swipe_up(void)
{
  /* Can we move to the bottom tile ? */
  if (g_ui.p_current_tile->p_bottom != NULL)
  {
    /* Yes, setup animation. */
    g_ui.state = UI_STATE_MOVE_DOWN;
    g_ui.p_from_tile = g_ui.p_current_tile;
    g_ui.p_to_tile = g_ui.p_current_tile->p_bottom;
    g_ui.p_to_tile->offset_x = 0;
    g_ui.p_to_tile->offset_y = SCREEN_HEIGHT;
  }
}

void ui_swipe_down(void)
{
  /* Can we move to the top tile ? */
  if (g_ui.p_current_tile->p_top != NULL)
  {
    /* Yes, setup animation. */
    g_ui.state = UI_STATE_MOVE_UP;
    g_ui.p_from_tile = g_ui.p_current_tile;
    g_ui.p_to_tile = g_ui.p_current_tile->p_top;
    g_ui.p_to_tile->offset_x = 0;
    g_ui.p_to_tile->offset_y = -SCREEN_HEIGHT;
  }
}

void ui_go_right(void)
{
  ui_swipe_left();
}

void ui_go_left(void)
{
  ui_swipe_right();
}

void ui_go_up(void)
{
  ui_swipe_down();
}

void ui_go_down(void)
{
  ui_swipe_up();
}


/**
 * ui_process_events()
 * 
 * @brief: Process UI events (user interaction and animation/rendering)
 **/

void IRAM_ATTR ui_process_events(void)
{
  touch_event_t touch;
  tile_t *p_main_tile;

  /* Process touch events if we are not in an animation. */
  if (g_ui.state == UI_STATE_IDLE)
  {
    if (twatch_get_touch_event(&touch, 1) == ESP_OK)
    {
      switch(touch.type)
      {
        case TOUCH_EVENT_SWIPE_RIGHT:
          {
            /* Forward event to widgets, consider event only if no widget processed it. */
            if (ui_forward_event_to_widget(TOUCH_EVENT_SWIPE_RIGHT, touch.coords.x, touch.coords.y, touch.velocity) == WE_ERROR)
            {
              /* Swipe right. */
              ui_swipe_right();
            }
          }
          break;

        case TOUCH_EVENT_SWIPE_LEFT:
          {
            /* Forward event to widgets, consider event only if no widget processed it. */
            if (ui_forward_event_to_widget(TOUCH_EVENT_SWIPE_LEFT, touch.coords.x, touch.coords.y, touch.velocity) == WE_ERROR)
            {
              /* Swipe left. */
              ui_swipe_left();
            }
          }
          break;

        case TOUCH_EVENT_SWIPE_UP:
          {
            /* Forward event to widgets, consider event only if no widget processed it. */
            if (ui_forward_event_to_widget(TOUCH_EVENT_SWIPE_UP, touch.coords.x, touch.coords.y, touch.velocity) == WE_ERROR)
            {
              /* Swipe up. */
              ui_swipe_up();
            }
          }
          break;

        case TOUCH_EVENT_SWIPE_DOWN:
          {
            /* Forward event to widgets, consider event only if no widget processed it. */
            if (ui_forward_event_to_widget(TOUCH_EVENT_SWIPE_DOWN, touch.coords.x, touch.coords.y, touch.velocity) == WE_ERROR)
            {
              /* Swipe down. */
              ui_swipe_down();
            }
          }
          break;

        case TOUCH_EVENT_PRESS:
          {
            ui_forward_event_to_widget(TOUCH_EVENT_PRESS, touch.coords.x, touch.coords.y, 0);
          }
          break;

        case TOUCH_EVENT_RELEASE:
          {
            ui_forward_event_to_widget(TOUCH_EVENT_RELEASE, touch.coords.x, touch.coords.y, 0);
          }
          break;

        case TOUCH_EVENT_TAP:
          {
            ui_forward_event_to_widget(TOUCH_EVENT_TAP, touch.coords.x, touch.coords.y, 0);
          }
          break;

        default:
          break;

      }
    }
  }

  /* Has lateral button been short-pressed ? */
  if (twatch_pmu_is_userbtn_pressed())
  {
    /* Forward event to the current tile. */
    tile_send_event(
      g_ui.p_current_tile,
      TE_USERBTN,
      0,
      0,
      0
    );
  }


  /* Refresh screen. */
  st7789_blank();
  switch(g_ui.state)
  {
    /* Show a single tile (current tile). */
    case UI_STATE_IDLE:
      {
        /* Draw current tile. */
        tile_draw(g_ui.p_current_tile);

        /* If a modal is set, display it. */
        if (g_ui.p_modal != NULL)
        {
          tile_draw(&g_ui.p_modal->tile);
        }
      }
      break;

    /* Animate move to right tile. */
    case UI_STATE_MOVE_RIGHT:
      {
        g_ui.p_to_tile->offset_x -= UI_ANIM_DELTA;
        g_ui.p_from_tile->offset_x -= UI_ANIM_DELTA;

        /* Draw tiles. */
        tile_draw(g_ui.p_from_tile);
        tile_draw(g_ui.p_to_tile);

        /* Stop condition. */
        if (g_ui.p_to_tile->offset_x == 0)
        {
          /* Send TE_EXIT to the previous tile. */
          tile_send_event(
            g_ui.p_current_tile,
            TE_EXIT,
            0,
            0,
            0
          );

          g_ui.state = UI_STATE_IDLE;
          g_ui.p_current_tile = g_ui.p_to_tile;

          /* Send TE_ENTER to current tile. */
          tile_send_event(
            g_ui.p_current_tile,
            TE_ENTER,
            0,
            0,
            0
          );
        }
      }
      break;

    /* Animate move to left tile. */
    case UI_STATE_MOVE_LEFT:
      {
        g_ui.p_to_tile->offset_x += UI_ANIM_DELTA;
        g_ui.p_from_tile->offset_x += UI_ANIM_DELTA;

        /* Draw tiles. */
        tile_draw(g_ui.p_from_tile);
        tile_draw(g_ui.p_to_tile);

        /* Stop condition. */
        if (g_ui.p_to_tile->offset_x == 0)
        {
          /* Send TE_EXIT to the previous tile. */
          tile_send_event(
            g_ui.p_current_tile,
            TE_EXIT,
            0,
            0,
            0
          );

          g_ui.state = UI_STATE_IDLE;
          g_ui.p_current_tile = g_ui.p_to_tile;
          
          /* Send TE_ENTER to current tile. */
          tile_send_event(
            g_ui.p_current_tile,
            TE_ENTER,
            0,
            0,
            0
          );
        }
      }
      break;

    /* Animate move to right tile. */
    case UI_STATE_MOVE_DOWN:
      {
        g_ui.p_to_tile->offset_y -= UI_ANIM_DELTA;
        g_ui.p_from_tile->offset_y -= UI_ANIM_DELTA;

        /* Draw tiles. */
        tile_draw(g_ui.p_from_tile);
        tile_draw(g_ui.p_to_tile);

        /* Stop condition. */
        if (g_ui.p_to_tile->offset_y == 0)
        {
          /* Send TE_EXIT to the previous tile. */
          tile_send_event(
            g_ui.p_current_tile,
            TE_EXIT,
            0,
            0,
            0
          );

          g_ui.state = UI_STATE_IDLE;
          g_ui.p_current_tile = g_ui.p_to_tile;

          /* Send TE_ENTER to current tile. */
          tile_send_event(
            g_ui.p_current_tile,
            TE_ENTER,
            0,
            0,
            0
          );
        }
      }
      break;

    /* Animate move to left tile. */
    case UI_STATE_MOVE_UP:
      {
        g_ui.p_to_tile->offset_y += UI_ANIM_DELTA;
        g_ui.p_from_tile->offset_y += UI_ANIM_DELTA;

        /* Draw tiles. */
        tile_draw(g_ui.p_from_tile);
        tile_draw(g_ui.p_to_tile);

        /* Stop condition. */
        if (g_ui.p_to_tile->offset_y == 0)
        {
          /* Send TE_EXIT to the previous tile. */
          tile_send_event(
            g_ui.p_current_tile,
            TE_EXIT,
            0,
            0,
            0
          );

          g_ui.state = UI_STATE_IDLE;
          g_ui.p_current_tile = g_ui.p_to_tile;

          /* Send TE_ENTER to current tile. */
          tile_send_event(
            g_ui.p_current_tile,
            TE_ENTER,
            0,
            0,
            0
          );
        }
      }
      break;

    default:
      break;
  }
  st7789_commit_fb();
}

/**
 * @brief: set UI modal box.
 * @param p_modal: pointer to a `modal_t`
 **/

void ui_set_modal(modal_t *p_modal)
{
  g_ui.p_modal = p_modal;
}


/**
 * @brief: unset (hide) UI modal box.
  **/

void ui_unset_modal(void)
{
  g_ui.p_modal = NULL;
}


/**********************************************************************
 * Handle press/release events
 *********************************************************************/

/**
 * @brief: forward a press/release event to the corresponding widget.
 * @param state: touch state (press/release)
 * @param x: X coordinate of the event
 * @param y: Y coordinate of the event
 * @return: 0 if no widget claimed the event, 1 otherwise.
 **/

int ui_forward_event_to_widget(touch_event_type_t state, int x, int y, int velocity)
{
  widget_t *p_widget = widget_enum_first();
  while (p_widget != NULL)
  {
    if (state == TOUCH_EVENT_RELEASE)
      widget_send_event(p_widget, (widget_event_t)state, x, y, velocity);
    else
    {
      /* Is there a modal dialog box active ? */
      if (g_ui.p_modal != NULL)
      {
        /* No modal dialog box, handles events as usual. */
        if (p_widget->p_tile == &g_ui.p_modal->tile)
        {
          if (
            (x >= (g_ui.p_modal->tile.offset_x + p_widget->box.x)) && (y >= (g_ui.p_modal->tile.offset_y + p_widget->box.y)) && \
            (x < (g_ui.p_modal->tile.offset_x + p_widget->box.x + p_widget->box.width)) && \
            (y < (g_ui.p_modal->tile.offset_y + p_widget->box.y + p_widget->box.height))
          )
          {
            /* Forward the touch event to the widget. */
            if (widget_send_event(
              p_widget,
              (widget_event_t)state,
              x - (g_ui.p_modal->tile.offset_x + p_widget->box.x),
              y - (g_ui.p_modal->tile.offset_y + p_widget->box.y),
              velocity
            ) == WE_PROCESSED)
            {
              /* Widget processed the event, we are done. */
              return 0;
            }
          }
        }
      }
      else
      {
        /* No modal dialog box, handles events as usual. */
        if (p_widget->p_tile == g_ui.p_current_tile)
        {
          if (
            (x >= p_widget->box.x) && (y >= p_widget->box.y) && \
            (x < (p_widget->box.x + p_widget->box.width)) && \
            (y < (p_widget->box.y + p_widget->box.height))
          )
          {
            /* Forward the touch event to the widget. */
            if (widget_send_event(p_widget, (widget_event_t)state, x - p_widget->box.x, y - p_widget->box.y, velocity) == WE_PROCESSED)
            {
              /* Widget processed the event, we are done. */
              return 0;
            }
          }
        }
      }
    }

    /* Go to next widget. */
    p_widget = widget_enum_next(p_widget);
  }

  /* If we have a modal dialog box active, disable screen swipe. */
  if ((g_ui.p_modal != NULL) && (state >= TOUCH_EVENT_SWIPE_LEFT))
  {
    /*
     * Event has been "processed" by our modal dialog box, we are done.
     * This disables swipe support if a modal is active.
     */
    return 0;
  }
  else
  {
    /* No widget processed this event. */
    return 1;
  }
}


/**
 * @brief Send a tile event to a tile.
 * @param p_tile: pointer to a `tile_t` structure
 * @param tile_event: tile event to send
 * @param x: X-coordinate (for UI-related event)
 * @param y: Y-coordinate (for UI-related event)
 * @param velocity: Swipe velocity (for UI-related event)
 * @return: TE_ERROR if event has not been processed, TE_PROCESSED otherwise
 **/

int tile_send_event(tile_t *p_tile, tile_event_t tile_event, int x, int y, int velocity)
{
  return p_tile->pfn_event_handler(
    p_tile,
    tile_event,
    x,
    y,
    velocity
  );
}


/**********************************************************************
 * Drawing primitives for tiles.
 *
 * These primitives are simple wrappers to screen rendering routines,
 * applying X and Y offsets.
 **********************************************************************/

/**
 * @brief Set a pixel color in framebuffer
 * @param p_tile: pointer to a `tile_t` structure
 * @param x: pixel X coordinate in tile
 * @param y: pixel Y coordinate in tile
 * @param color: pixel color (12 bits)
 **/

void tile_set_pixel(tile_t *p_tile, int x, int y, uint16_t pixel)
{
  /* Apply X/Y offsets. */
  st7789_set_pixel(x + p_tile->offset_x, y + p_tile->offset_y, pixel);
}


/**
 * @brief Fills a region of the screen with a specific color
 * @param p_tile: pointer to a `tile_t` structure
 * @param x: top-left X coordinate of tile
 * @param y: top-left Y coordinate of tile
 * @param width: region width
 * @param height: region height
 * @parma color: 12 bpp color
 **/

void tile_fill_region(tile_t *p_tile, int x, int y, int width, int height, uint16_t color)
{
  /* Apply X/Y offsets. */
  st7789_fill_region(
    x + p_tile->offset_x,
    y + p_tile->offset_y,
    width,
    height,
    color
  );
}


/**
 * @brief Draw a line of color `color` between (x0,y0) and (x1, y1)
 * @param p_tile: pointer to a `tile_t` structure
 * @param x0: X coordinate of the start of the line in tile
 * @param y0: Y cooordinate of the start of the line in tile
 * @param x1: X coordinate of the end of the line in tile
 * @param y1: y coordinate of the end of the line in tile
 **/

void tile_draw_line(tile_t *p_tile, int x0, int y0, int x1, int y1, uint16_t color)
{
  /* Apply X/Y offsets. */
  st7789_draw_line(
    x0 + p_tile->offset_x,
    y0 + p_tile->offset_y,
    x1 + p_tile->offset_x,
    y1 + p_tile->offset_y,
    color
  );
}

void tile_draw_circle(tile_t *p_tile, int x, int y, int r, uint16_t color)
{
  st7789_draw_circle(
    x + p_tile->offset_x,
    y + p_tile->offset_y,
    r,
    color
  );
}

void tile_draw_disc(tile_t *p_tile, int x, int y, int r, uint16_t color)
{
  st7789_draw_disc(
    x + p_tile->offset_x,
    y + p_tile->offset_y,
    r,
    color
  );
}


/**
 * tile_draw_char()
 * 
 * @brief: Draw a character at a given position with a given color.
 * @param p_tile: pointer to a `tile_t` structure
 * @param x: X coordinate
 * @param y: Y cooordinate
 * @param c: character to draw
 * @param color: character color
 **/

void tile_draw_char(tile_t *p_tile, int x, int y, char c, uint16_t color)
{
    /* Draw character with tile offset. */
    font_draw_char(p_tile->offset_x + x, p_tile->offset_y + y, c, color);
}


/**
 * tile_draw_text()
 * 
 * @brief: Draw a text at a given position with a given color.
 * @param p_tile: pointer to a `tile_t` structure
 * @param x: X coordinate
 * @param y: Y cooordinate
 * @param psz_text: text string to draw
 * @param color: character color
 **/

void tile_draw_text(tile_t *p_tile, int x, int y, char *psz_text, uint16_t color)
{
    /* Draw character with tile offset. */
    font_draw_text(p_tile->offset_x + x, p_tile->offset_y + y, psz_text, color);
}


/**
 * @brief Copy a portion of source image into destination buffer
 * @param p_tile: pointer to a `tile_t` structure
 * @param source: source image
 * @param source_x: X coordinate of the region to bitblt from the source image
 * @param source_y: Y coordinate of the region to bitblt from the source image
 * @param width: source region width
 * @param height: source region height
 * @param dest_x: X coordinate of the destination buffer
 * @param dest_y: Y coordinate of the destination buffer
 **/

void tile_bitblt(tile_t *p_tile, image_t *source, int source_x, int source_y, int width, int height, int dest_x, int dest_y)
{
  screen_bitblt(
    source,
    source_x,
    source_y,
    width,
    height,
    dest_x + p_tile->offset_x,
    dest_y + p_tile->offset_y
  );
}


/**
 * Default tile drawing routine.
 **/

int _tile_default_draw(tile_t *p_tile)
{
  /* If offset is beyond tile. */
  if ( (p_tile->offset_x >= SCREEN_WIDTH) || (p_tile->offset_y >= SCREEN_HEIGHT) )
  {
    /* No need to draw anything, tile is not visible. */
    return -1;
  }

  /* If offset is way lower than screen height or width. */
  if ( (p_tile->offset_x <= -SCREEN_WIDTH) || (p_tile->offset_y <= -SCREEN_HEIGHT) )
  {
    /* No need to draw anything, tile is not visible. */
    return -1;
  }

  /* Set drawing window. */
  st7789_set_drawing_window(
    p_tile->offset_x,
    p_tile->offset_y,
    p_tile->offset_x + p_tile->width,
    p_tile->offset_y + p_tile->height
  );

  /* Fill region with background color. */
  st7789_fill_region(
    p_tile->offset_x,
    p_tile->offset_y,
    p_tile->width,
    p_tile->height,
    p_tile->background_color
  );

  /* Draw widgets. */
  tile_draw_widgets(p_tile);

  /* Success. */
  return 0;
}


int _tile_default_event_handler(struct tile_t *p_tile, tile_event_t p_event, int x, int y, int velocity)
{
  /* Event not processed. */
  return TE_ERROR;
}


/**
 * @brief Initialize a tile object.
 * @param p_tile: pointer to a  `tile_t` structure
 * @param p_user_data: pointer to some user data to associate with this tile
 **/

void tile_init(tile_t *p_tile, void *p_user_data)
{
  /* Initialize struct members. */
  p_tile->t_type = TILE_MAIN;
  p_tile->offset_x = 0;
  p_tile->offset_y = 0;
  p_tile->width = SCREEN_WIDTH;
  p_tile->height = SCREEN_HEIGHT;
  p_tile->p_left = NULL;
  p_tile->p_right = NULL;
  p_tile->p_top = NULL;
  p_tile->p_bottom = NULL;
  p_tile->p_user_data = p_user_data;
  p_tile->background_color = RGB(0,0,0);

  /* Install our default callbacks. */
  p_tile->pfn_draw_tile = (FDrawTile)_tile_default_draw;
  p_tile->pfn_event_handler = (FTileEventHandler)_tile_default_event_handler;
}


/**
 * @brief: Set tile rendering function
 * @param p_tile: pointer to a `tile_t` structure
 * @param pfn_drawfunc: pointer to a FDrawTile function
 **/

void tile_set_drawfunc(tile_t *p_tile, FDrawTile pfn_drawfunc)
{
  if ((p_tile != NULL) && (pfn_drawfunc != NULL))
    p_tile->pfn_draw_tile = pfn_drawfunc;
}


/**
 * @brief: Set tile event handler function
 * @param p_tile: pointer to a `tile_t` structure
 * @param pfn_drawfunc: pointer to a FDrawTile function
 **/

FTileEventHandler tile_set_event_handler(tile_t *p_tile, FTileEventHandler pfn_event_handler)
{
  FTileEventHandler old_event_handler = NULL;

  if ((p_tile != NULL) && (pfn_event_handler != NULL))
  {
    /* Save old event handler. */
    old_event_handler = p_tile->pfn_event_handler;

    /* Set new event handler. */
    p_tile->pfn_event_handler = pfn_event_handler;
  }

  /* Return old event handler. */
  return old_event_handler;
}


/**
 * tile_get_user_data()
 * 
 * @brief: get tile user data
 * @param p_tile: pointer to a `tile_t` structure
 **/

void *tile_get_user_data(tile_t *p_tile)
{
  return p_tile->p_user_data;
}


/**
 * tile_draw()
 * 
 * @brief: draw a tile
 * @param p_tile: pointer to a `tile_t` structure
 **/

int IRAM_ATTR tile_draw(tile_t *p_tile)
{
  if (p_tile != NULL)
  {
    if (p_tile->pfn_draw_tile != NULL)
      return p_tile->pfn_draw_tile(p_tile);
  }

  /* Failure. */
  return -2;
}


/**
 * tile_link_right()
 * 
 * @brief: Connect a tile to the right of another tile
 * @param p_tile: pointer to a `tile_t` structure (parent tile)
 * @param p_right_tile: pointer to a `tile_t` structure to connect to the right
 **/

void tile_link_right(tile_t *p_tile, tile_t *p_right_tile)
{
  /* Sanity check. */
  if ((p_tile == NULL) || (p_right_tile == NULL) || (p_right_tile->t_type != TILE_MAIN))
    return;

  /* Link tiles. */
  p_tile->p_right = p_right_tile;
  p_right_tile->p_left = p_tile;
}


/**
 * tile_link_left()
 * 
 * @brief: Connect a tile to the left of another tile
 * @param p_tile: pointer to a `tile_t` structure (parent tile)
 * @param p_left_tile: pointer to a `tile_t` structure to connect to the left
 **/

void tile_link_left(tile_t *p_tile, tile_t *p_left_tile)
{
  /* Sanity check. */
  if ((p_tile == NULL) || (p_left_tile == NULL)  || (p_left_tile->t_type != TILE_MAIN))
    return;

  /* Link tiles. */
  p_tile->p_left = p_left_tile;
  p_left_tile->p_right = p_tile;
}


/**
 * tile_link_top()
 * 
 * @brief: Connect a tile to the top of another tile
 * @param p_tile: pointer to a `tile_t` structure (parent tile)
 * @param p_top_tile: pointer to a `tile_t` structure to connect to the top
 **/

void tile_link_top(tile_t *p_tile, tile_t *p_top_tile)
{
  /* Sanity check. */
  if ((p_tile == NULL) || (p_top_tile == NULL))
    return;

  /* Link tiles. */
  p_tile->p_top = p_top_tile;
  p_top_tile->p_bottom = p_tile;

  /* Set tile as secondary. */
  p_top_tile->t_type = TILE_SECONDARY;
}


/**
 * tile_link_bottom()
 * 
 * @brief: Connect a tile to the bottom of another tile
 * @param p_tile: pointer to a `tile_t` structure (parent tile)
 * @param p_bottom_tile: pointer to a `tile_t` structure to connect to the bottom
 **/

void tile_link_bottom(tile_t *p_tile, tile_t *p_bottom_tile)
{
  /* Sanity check. */
  if ((p_tile == NULL) || (p_bottom_tile == NULL))
    return;

  /* Link tiles. */
  p_tile->p_bottom = p_bottom_tile;
  p_bottom_tile->p_top = p_tile;

  /* Set tile as secondary. */
  p_bottom_tile->t_type = TILE_SECONDARY;
}


/**************************************************
 * Widget related functions
 *************************************************/

/**
 * tile_draw_widgets()
 * 
 * @brief: Draw widgets belonging to a given tile.
 * @param p_tile: pointer to a `tile_t` structure
 **/

void IRAM_ATTR tile_draw_widgets(tile_t *p_tile)
{
  widget_t *p_widget;

  /*
  * Iterate over all the widgets, and display only those belonging to
  * the specified tile.
  */
  p_widget = widget_enum_first();
  while (p_widget != NULL)
  {
    if (widget_get_tile(p_widget) == p_tile)
      widget_draw(p_widget);

    /* Go to next widget. */
    p_widget = widget_enum_next(p_widget);
  }
}
