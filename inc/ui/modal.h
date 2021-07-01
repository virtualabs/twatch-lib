#ifndef __INC_UI_MODAL_H
#define __INC_UI_MODAL_H

typedef struct {
  
  /* Underlying tile. */
  tile_t tile;

  /* Border color. */
  uint16_t border_color;

} modal_t;

void modal_init(modal_t *p_modal, int x, int y, int width, int height);
void modal_set_border_color(modal_t *p_modal, uint16_t color);

#endif /* __INC_UI_MODAL_H */