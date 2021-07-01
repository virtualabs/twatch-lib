#include "ui/ui.h"

/**
 * _modal_event_handler()
 * 
 * @brief: Modal dialog box default event handler
 * @param p_tile: pointer to a `tile_t` structure
 * @param x: x-coordinate
 * @param y: y-coordinate
 * @param velocity: touch event velocity
 * @return TE_PROCESSED if event has been processed, TE_ERROR otherwise
 **/

int _modal_event_handler(tile_t *p_tile, tile_event_t p_event, int x, int y, int velocity)
{
  switch(p_event)
  {
    case TE_MODAL_CLOSE:
      {
        ui_unset_modal();
      }
      break;

    default:
      break;
  }

  /* Event not processed. */
  return TE_ERROR;
}

/**
 * _modal_drawfunc()
 * 
 * @brief: Callback function that renders a modal dialog box on screen.
 * @param p_tile: pointer to a `tile_t` structure.
 **/

int _modal_drawfunc(tile_t *p_tile)
{
  if (p_tile != NULL)
  {

    /* Set drawing window. */
    st7789_set_drawing_window(
      p_tile->offset_x,
      p_tile->offset_y,
      p_tile->offset_x + p_tile->width,
      p_tile->offset_y + p_tile->height
    );

    tile_fill_region(
      p_tile,
      1,
      1,
      p_tile->width-2,
      p_tile->height-2,
      p_tile->background_color
    );

    /* Draw the modal. */
    tile_draw_line(
      p_tile,
      1,
      0,
      p_tile->width - 2,
      0,
      RGB(0xf,0xf,0xf)
    );
    tile_draw_line(
      p_tile,
      1,
      p_tile->height - 1,
      p_tile->width - 2,
      p_tile->height - 1,
      RGB(0xf,0xf,0xf)
    );
    tile_draw_line(
      p_tile,
      0,
      1,
      0,
      p_tile->height  - 2,
      RGB(0xf,0xf,0xf)
    );
    tile_draw_line(
      p_tile,
      p_tile->width - 1,
      1,
      p_tile->width - 1,
      p_tile->height  - 2,
      RGB(0xf,0xf,0xf)
    );
  }

  /* Draw widgets. */
  tile_draw_widgets(p_tile);

  /* Success. */
  return TE_PROCESSED;
}

/**
 * modal_init()
 * 
 * @brief: initialize a modal dialog box.
 * @param p_modal: pointer to a `modal_t` structure
 * @param x: x-coordinate
 * @param y: y-coordinate
 * @param width: modal width
 * @param height: modal height
 **/

void modal_init(modal_t *p_modal, int x, int y, int width, int height)
{
  /* Initialize our p_modal structure. */
  tile_init(&p_modal->tile, (void *)p_modal);
  p_modal->tile.offset_x = x;
  p_modal->tile.offset_y = y;
  p_modal->tile.width = width;
  p_modal->tile.height = height;

  /* Set drawing function. */
  tile_set_drawfunc(&p_modal->tile, _modal_drawfunc);
  tile_set_event_handler(&p_modal->tile, _modal_event_handler);
}