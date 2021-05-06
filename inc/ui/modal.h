#ifndef __INC_UI_MODAL_H
#define __INC_UI_MODAL_H

typedef struct {
  
  /* Underlying tile. */
  tile_t tile;

} modal_t;

void modal_init(modal_t *p_modal, int x, int y, int width, int height);

#endif /* __INC_UI_MODAL_H */