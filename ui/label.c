#include "ui/label.h"


/**
 * @brief: Rendering function for labels.
 * @param p_widget: pointer to a  `widget_label_t` structure
 **/

void widget_label_drawfunc(widget_t *p_widget)
{
  int text_width, dx, dy;
  widget_label_t *p_label = (widget_label_t *)p_widget->p_user_data;

  if (p_label->psz_label != NULL)
    {
      /* Draw text. */
      widget_draw_text_x2(
        p_widget,
        3,
        3,
        p_label->psz_label,
        p_widget->style.front
      );
    }
}


/**
 * @brief: Create a label.
 * @param p_widget_label: Pointer to a `widget_label_t` structure
 * @param p_tile: Tile where this label will be placed
 * @param x: X coordinate of the label widget
 * @param y: Y coordinate of the label widget
 * @param width: widget width
 * @param height: widget height
 * @param psz_label: label text, NULL if none
 **/

void widget_label_init(widget_label_t *p_widget_label, tile_t *p_tile, int x, int y, int width, int height, char *psz_label)
{
  /* Initialize widget. */
  widget_init(&p_widget_label->widget, p_tile, x, y, width, height);

  /* Set properties. */
  p_widget_label->psz_label = psz_label;

  /* Set user data. */
  widget_set_userdata(&p_widget_label->widget, (void *)p_widget_label);

  /* Set drawing function. */
  widget_set_drawfunc(&p_widget_label->widget, widget_label_drawfunc);
}


/**
 * @brief: Set label text
 * @param p_widget_label: pointer to a `widget_label_t` structure
 * @param psz_label: pointer to a string containing the label text
 **/

void widget_label_set_text(widget_label_t *p_widget_label, char *psz_label)
{
  p_widget_label->psz_label = psz_label;
}