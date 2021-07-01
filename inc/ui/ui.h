#ifndef __INC_TWATCH_UI_H
#define __INC_TWATCH_UI_H

#include <stdint.h>
#include "driver/timer.h"
#include "drivers/st7789.h"

#include "hal/touch.h"
#include "hal/pmu.h"
#include "hal/screen.h"

#include "img.h"
#include "font/font16.h"

/* TODO: move these constants into a 'screen' subpart ;) */
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240
#define UI_ANIM_DELTA 40

#define TE_ERROR      (1)
#define TE_PROCESSED  (0)

typedef enum {
  TE_ENTER=0xF00,
  TE_EXIT,
  TE_USERBTN,
  TE_MODAL_CLOSE
} tile_event_t;

typedef struct tTile tile_t;

typedef int (*FTileEventHandler)(tile_t *p_tile, tile_event_t p_event, int x, int y, int velocity);

typedef enum {
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN
} ui_move_dir;

typedef enum {
  UI_STATE_IDLE,
  UI_STATE_MOVE_LEFT,
  UI_STATE_MOVE_RIGHT,
  UI_STATE_MOVE_UP,
  UI_STATE_MOVE_DOWN
} ui_state_machine;

typedef enum {
  TILE_MAIN,
  TILE_SECONDARY
} ui_tile_type_t;

typedef enum {
  SCREEN_MODE_NORMAL,
  SCREEN_MODE_DIMMED
} screen_mode_t;


typedef int (*FDrawTile)(tile_t *p_tile);

/**
 * Tile base structure
 **/

typedef struct tTile {

  /* Background color, 12bpp */
  uint16_t background_color;

  /* TODO: create a tile box ? */

  /* Offsets */
  int offset_x;
  int offset_y;

  /* Tile size. */
  int width;
  int height;

  /* Type */
  ui_tile_type_t t_type;

  /* Links to other tiles */
  struct tTile *p_right;
  struct tTile *p_left;
  struct tTile *p_top;
  struct tTile *p_bottom;

  /* User data */
  void *p_user_data;

  /**
   * Callbacks
   **/

  /**
   * Draw screen callback. This callback is called when the UI needs to refresh
   * this screen. Only the visible parts will be updated, depending on the offsets.
   **/
  FDrawTile pfn_draw_tile;
  FTileEventHandler pfn_event_handler;

} tile_t;

/* Include modal.h here in order to have tile_t structure defined. */
#include "ui/modal.h"

/**
 * User interface main structure
 **/

typedef struct {

  /* State machine (animation). */
  ui_state_machine state;
  tile_t *p_from_tile;
  tile_t *p_to_tile;

  /* Pointer to current tile. */
  tile_t *p_current_tile;

  /* Pointer to default tile. */
  tile_t *p_default_tile;

  /* Pointer to a modal box. */
  modal_t *p_modal;

  /* Eco mode timer. */
  bool b_eco_mode_enabled;
  screen_mode_t screen_mode;
  timer_config_t eco_timer;
  int eco_max_inactivity;
  int eco_max_inactivity_to_deepsleep;
  bool b_inactivity_detected;
  bool b_usb_plugged;

} ui_t;



/**
 * Exports
 **/

/* Main UI */
void ui_init(void);
void ui_set_default_tile(tile_t *p_tile);
void ui_select_tile(tile_t *p_tile);
void ui_default_tile();
void ui_process_events(void);
void ui_set_modal(modal_t *p_modal);
void ui_unset_modal(void);

/* Swipe management. */
void ui_swipe_right(void);
void ui_swipe_left(void);
void ui_swipe_up(void);
void ui_swipe_down(void);

void ui_go_right(void);
void ui_go_left(void);
void ui_go_up(void);
void ui_go_down(void);

/* Eco mode management. */
bool is_ecomode_set(void);
void enable_ecomode(void);
void disable_ecomode(void);
void ui_wakeup(void);

/* Tiles */
void tile_init(tile_t *p_tile, void *p_user_data);
int tile_draw(tile_t *p_tile);
void tile_set_drawfunc(tile_t *p_tile, FDrawTile pfn_drawfunc);
void *tile_get_user_data(tile_t *p_tile);
void tile_draw_widgets(tile_t *p_tile);

/* Drawing primitives for tiles. */
void tile_set_pixel(tile_t *p_tile, int x, int y, uint16_t pixel);
void tile_fill_region(tile_t *p_tile, int x, int y, int width, int height, uint16_t color);
void tile_draw_line(tile_t *p_tile, int x0, int y0, int x1, int y1, uint16_t color);
void tile_draw_circle(tile_t *p_tile, int x, int y, int r, uint16_t color);
void tile_draw_disc(tile_t *p_tile, int x, int y, int r, uint16_t color);
void tile_bitblt(tile_t *p_tile, image_t *source, int source_x, int source_y, int width, int height, int dest_x, int dest_y);
void tile_draw_char(tile_t *p_tile, int x, int y, char c, uint16_t color);
void tile_draw_text(tile_t *p_tile, int x, int y, char *psz_text, uint16_t color);
void tile_draw_char_x2(tile_t *p_tile, int x, int y, char c, uint16_t color);
void tile_draw_text_x2(tile_t *p_tile, int x, int y, char *psz_text, uint16_t color);


/* Tile linkage */
void tile_link_right(tile_t *p_tile, tile_t *p_right_tile);
void tile_link_left(tile_t *p_tile, tile_t *p_left_tile);
void tile_link_top(tile_t *p_tile, tile_t *p_top_tile);
void tile_link_bottom(tile_t *p_tile, tile_t *p_bottom_tile);

/* Events */
FTileEventHandler tile_set_event_handler(tile_t *p_tile, FTileEventHandler pfn_event_handler);
int tile_send_event(tile_t *p_tile, tile_event_t tile_event, int x, int y, int velocity);


#endif /* __INC_TWATCH_UI_H */
