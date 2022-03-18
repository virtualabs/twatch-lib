#include "ui/spinner.h"

#define  ANIM_COUNTER_THRESHOLD   10
#define  DISC_DIAM                15
#define  DISC_GAP                 15
#define  DISC_COUNT               4

/**
 * @brief: Rendering function for spinners.
 * @param p_widget: pointer to a  `widget_spinner_t` structure
 **/

int widget_spinner_drawfunc(widget_t *p_widget)
{
  int i, j, off_x, off_y;
  widget_spinner_t *p_spinner = (widget_spinner_t *)p_widget->p_user_data;

  if (p_spinner != NULL)
  {
    /* Increment our counter and update animation_step. */
    p_spinner->counter++;
    if (p_spinner->counter >= ANIM_COUNTER_THRESHOLD)
    {
      p_spinner->animation_step = (p_spinner->animation_step + 1)%4;
      p_spinner->counter = 0;
    }

    /* Draw background. */
    widget_fill_region(
      p_widget,
      1,
      1,
      p_widget->box.width-2,
      p_widget->box.height-2,
      p_widget->style.background
    );

    /* Pick previous disc. */
    if (p_spinner->animation_step==0)
      j=3;
    else
      j=p_spinner->animation_step-1;

    /* Draw discs. */
    off_x = (p_widget->box.width - (DISC_COUNT*DISC_DIAM) - ((DISC_COUNT-1)*DISC_GAP))/2;
    off_y = (p_widget->box.height - DISC_DIAM)/2;
    for (i=0; i<DISC_COUNT; i++)
    {
      if (i == p_spinner->animation_step)
      {
        widget_draw_disc(
          p_widget,
          off_x + (DISC_DIAM/2) + i*(DISC_DIAM+DISC_GAP),
          off_y + (DISC_DIAM/2),
          DISC_DIAM/2 + p_spinner->counter/2,
          p_widget->style.front
        );
      }
      else if (i == j)
      {
        widget_draw_disc(
          p_widget,
          off_x + (DISC_DIAM/2) + i*(DISC_DIAM+DISC_GAP),
          off_y + (DISC_DIAM/2),
          (DISC_DIAM/2) + (9-p_spinner->counter)/2,
          p_widget->style.front
        );
      }
      else
      {
        widget_draw_disc(
          p_widget,
          off_x + (DISC_DIAM/2) + i*(DISC_DIAM+DISC_GAP),
          off_y + (DISC_DIAM/2),
          DISC_DIAM/2,
          p_widget->style.front
        );
      }
    }
  }

  /* Success. */
  return TE_PROCESSED;
}


/**
 * @brief: Create a spinner.
 * @param p_widget_spinner: Pointer to a `widget_spinner_t` structure
 * @param p_tile: Tile where this spinner will be placed
 * @param x: X coordinate of the spinner widget
 * @param y: Y coordinate of thespinnerl widget
 * @param width: widget width
 * @param height: widget height
 **/

void widget_spinner_init(widget_spinner_t *p_widget_spinner, tile_t *p_tile, int x, int y, int width, int height)
{
  /* Initialize widget. */
  widget_init(&p_widget_spinner->widget, p_tile, x, y, width, height);

  /* Set properties. */
  p_widget_spinner->counter = 0;
  p_widget_spinner->animation_step = 0;

  /* Set user data. */
  widget_set_userdata(&p_widget_spinner->widget, (void *)p_widget_spinner);

  /* Set drawing function. */
  widget_set_drawfunc(&p_widget_spinner->widget, widget_spinner_drawfunc);
}

