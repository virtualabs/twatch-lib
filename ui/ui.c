#include "ui/ui.h"

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
}


/**
 * @brief: Select the current tile
 * @param p_tile: pointer to a `tile_t` structure (the tile to select)
 **/

void ui_select_tile(tile_t *p_tile)
{
  /* Set current tile as none. */
  g_ui.p_current_tile = p_tile;
}


void ui_process_events(void)
{
  touch_event_t touch;

  /* Process touch events if we are not in an animation. */
  if (g_ui.state == UI_STATE_IDLE)
  {
    if (twatch_get_touch_event(&touch, 10) == ESP_OK)
    {
      if (touch.type == TOUCH_EVENT_SWIPE_RIGHT)
      {
        /* Can we move to the left tile ? */
        if (g_ui.p_current_tile->p_left != NULL)
        {
          /* Yes, setup animation. */
          g_ui.state = UI_STATE_MOVE_LEFT;
          g_ui.p_from_tile = g_ui.p_current_tile;
          g_ui.p_to_tile = g_ui.p_current_tile->p_left;
          g_ui.p_to_tile->offset_x = -SCREEN_WIDTH;
          g_ui.p_to_tile->offset_y = 0;
        }
      }

      if (touch.type == TOUCH_EVENT_SWIPE_LEFT)
      {
        /* Can we move to the left tile ? */
        if (g_ui.p_current_tile->p_right != NULL)
        {
          /* Yes, setup animation. */
          g_ui.state = UI_STATE_MOVE_RIGHT;
          g_ui.p_from_tile = g_ui.p_current_tile;
          g_ui.p_to_tile = g_ui.p_current_tile->p_right;
          g_ui.p_to_tile->offset_x = SCREEN_WIDTH;
          g_ui.p_to_tile->offset_y = 0;
        }
      }
    }
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
          g_ui.state = UI_STATE_IDLE;
          g_ui.p_current_tile = g_ui.p_to_tile;
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
          g_ui.state = UI_STATE_IDLE;
          g_ui.p_current_tile = g_ui.p_to_tile;
        }
      }
      break;

    default:
      break;
  }
  st7789_commit_fb();
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
  uint8_t width, height;

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

  /* Compute visible width and height. */
  if (p_tile->offset_x <= 0)
    width = SCREEN_WIDTH + p_tile->offset_x;
  else
    width = SCREEN_WIDTH - p_tile->offset_x;

  if (p_tile->offset_y <= 0)
    height = SCREEN_HEIGHT + p_tile->offset_y;
  else
    height = SCREEN_HEIGHT - p_tile->offset_y;

  /* Fill region with background color. */
  st7789_fill_region(
    p_tile->offset_x,
    p_tile->offset_y,
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    p_tile->background_color
  );

  /* TODO: Debug only, remove
  st7789_fill_region(
    p_tile->offset_x+60,
    p_tile->offset_y+60,
    120,
    120,
    p_tile->background_color
  );
  */

  /* Success. */
  return 0;
}



/**
 * @brief Initialize a tile object.
 * @param p_tile: pointer to a  `tile_t` structure
 * @param p_user_data: pointer to some user data to associate with this tile
 **/

void tile_init(tile_t *p_tile, void *p_user_data)
{
  /* Initialize struct members. */
  p_tile->offset_x = 0;
  p_tile->offset_y = 0;
  p_tile->p_left = NULL;
  p_tile->p_right = NULL;
  p_tile->p_user_data = p_user_data;
  p_tile->background_color = RGB(0,0,0);

  /* Install our default callbacks. */
  p_tile->pfn_draw_tile = (FDrawTile)_tile_default_draw;
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


void *tile_get_user_data(tile_t *p_tile)
{
  return p_tile->p_user_data;
}

/**
 * @brief: draw tile (in memory)
 * @param p_tile: tile to draw in memory
 **/

int tile_draw(tile_t *p_tile)
{
  if (p_tile != NULL)
  {
    if (p_tile->pfn_draw_tile != NULL)
      return p_tile->pfn_draw_tile(p_tile);
  }

  /* Failure. */
  return -2;
}

void tile_link_right(tile_t *p_tile, tile_t *p_right_tile)
{
  /* Sanity check. */
  if ((p_tile == NULL) || (p_right_tile == NULL))
    return;

  /* Link tiles. */
  p_tile->p_right = p_right_tile;
  p_right_tile->p_left = p_tile;
}


void tile_link_left(tile_t *p_tile, tile_t *p_left_tile)
{
  /* Sanity check. */
  if ((p_tile == NULL) || (p_left_tile == NULL))
    return;

  /* Link tiles. */
  p_tile->p_left = p_left_tile;
  p_left_tile->p_right = p_tile;
}
