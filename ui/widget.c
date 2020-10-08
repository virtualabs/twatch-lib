#include "ui/ui.h"
#include "ui/widget.h"

/* Global widget list. */
widget_t *gp_widgets = NULL;
widget_t *gp_last_widget = NULL;

void register_widget(widget_t *p_widget)
{
  if (gp_widgets == NULL)
  {
    gp_widgets = p_widget;
    gp_last_widget = p_widget;
  }
  else
  {
    gp_last_widget->p_next = p_widget;
    gp_last_widget = p_widget;
  }
}

widget_t *widget_enum_first(void)
{
  return gp_widgets;
}

widget_t *widget_enum_next(widget_t *p_widget)
{
  /* Sanity check. */
  if (p_widget == NULL)
    return NULL;

  /* Go to next widget. */
  if (p_widget->p_next != NULL)
    return p_widget->p_next;
  else
    return NULL;
}




/**
 * @brief: Initialize a `widget_t` structure.
 * @param p_tile: pointer to the widget's parent tile
 * @param x: widget X coordinate
 * @param y: widget Y coordinate
 * @param width: widget width
 * @param height: widget height
 **/

void widget_init(widget_t *p_widget, tile_t *p_tile, int x, int y, int width, int height)
{
  /* Set parent tile. */
  p_widget->p_tile = p_tile;

  /* Set properties. */
  p_widget->offset_x = x;
  p_widget->offset_y = y;
  p_widget->width = width;
  p_widget->height = height;
  p_widget->p_next = NULL;

  /* Set drawing func to NULL. */
  p_widget->pfn_drawfunc = NULL;
  p_widget->p_user_data = NULL;

  /* Add widget to our list. */
  register_widget(p_widget);
}

/**
 * @brief: Set drawing function for a given widget.
 * @param p_widget: target widget
 * @param pfn_drawfunc: pointer to a FDrawWidget procedure
 **/

void widget_set_drawfunc(widget_t *p_widget, FDrawWidget pfn_drawfunc)
{
  /* Sanity check. */
  if (p_widget == NULL)
    return;

  /* Set callback function. */
  p_widget->pfn_drawfunc = pfn_drawfunc;
}


/**
 * @brief: Set user data for a given widget
 * @param p_widget: target widget
 * @param p_user_data: pointer to some user data to link to the widget
 **/

void widget_set_userdata(widget_t *p_widget, void *p_user_data)
{
  /* Sanity check. */
  if (p_widget == NULL)
    return;

  /* Set user data pointer. */
  p_widget->p_user_data = p_user_data;
}


tile_t *widget_get_tile(widget_t *p_widget)
{
  if (p_widget != NULL)
    return p_widget->p_tile;
  else
    return NULL;
}


int widget_draw(widget_t *p_widget)
{
  if (p_widget != NULL)
  {
    if (p_widget->pfn_drawfunc != NULL)
      return p_widget->pfn_drawfunc(p_widget);
  }

  /* Failure. */
  return -2;
}

/**********************************************************************
 * Drawing primitives for widgets.
 *
 * These primitives are simple wrappers to screen rendering routines,
 * applying X and Y offsets.
 **********************************************************************/

/**
 * @brief Set a pixel color in framebuffer
 * @param p_tile: pointer to a `widget_t` structure
 * @param x: pixel X coordinate in tile
 * @param y: pixel Y coordinate in tile
 * @param color: pixel color (12 bits)
 **/

void widget_set_pixel(widget_t *p_widget, int x, int y, uint16_t pixel)
{
  /* Apply X/Y offsets. */
  st7789_set_pixel(
    x + p_widget->offset_x + p_widget->p_tile->offset_x,
    y + p_widget->offset_y + p_widget->p_tile->offset_y,
    pixel
  );
}


/**
 * @brief Fills a region of the screen with a specific color
 * @param p_tile: pointer to a `widget_t` structure
 * @param x: top-left X coordinate of tile
 * @param y: top-left Y coordinate of tile
 * @param width: region width
 * @param height: region height
 * @parma color: 12 bpp color
 **/

void widget_fill_region(widget_t *p_widget, int x, int y, int width, int height, uint16_t color)
{
  /* Apply X/Y offsets. */
  st7789_fill_region(
    x + p_widget->offset_x + p_widget->p_tile->offset_x,
    y + p_widget->offset_y + p_widget->p_tile->offset_y,
    width,
    height,
    color
  );
}


/**
 * @brief Draw a line of color `color` between (x0,y0) and (x1, y1)
 * @param p_tile: pointer to a `widget_t` structure
 * @param x0: X coordinate of the start of the line in tile
 * @param y0: Y cooordinate of the start of the line in tile
 * @param x1: X coordinate of the end of the line in tile
 * @param y1: y coordinate of the end of the line in tile
 **/

void widget_draw_line(widget_t *p_widget, int x0, int y0, int x1, int y1, uint16_t color)
{
  /* Apply X/Y offsets. */
  st7789_draw_line(
    x0 + p_widget->offset_x + p_widget->p_tile->offset_x,
    y0 + p_widget->offset_y + p_widget->p_tile->offset_y,
    x1 + p_widget->offset_x + p_widget->p_tile->offset_x,
    y1 + p_widget->offset_y + p_widget->p_tile->offset_y,
    color
  );
}


/**
 * @brief Copy a portion of source image into destination buffer
 * @param p_tile: pointer to a `widget_t` structure
 * @param source: source image
 * @param source_x: X coordinate of the region to bitblt from the source image
 * @param source_y: Y coordinate of the region to bitblt from the source image
 * @param width: source region width
 * @param height: source region height
 * @param dest_x: X coordinate of the destination buffer
 * @param dest_y: Y coordinate of the destination buffer
 **/

void widget_bitblt(widget_t *p_widget, image_t *source, int source_x, int source_y, int width, int height, int dest_x, int dest_y)
{
  screen_bitblt(
    source,
    source_x,
    source_y,
    width,
    height,
    dest_x + p_widget->offset_x + p_widget->p_tile->offset_x,
    dest_y + p_widget->offset_y + p_widget->p_tile->offset_y
  );
}
