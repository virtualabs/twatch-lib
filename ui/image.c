#include "ui/image.h"

#define WIDGET(x) (widget_t *)(&x->widget)

void widget_image_drawfunc(widget_t *p_widget)
{
  /* Retrieve the image widget structure. */
  widget_image_t *p_widget_img = (widget_image_t *)p_widget->p_user_data;

  /* Render image. */
  widget_bitblt(
    p_widget,
    p_widget_img->p_image,
    0,
    0,
    p_widget_img->widget.box.width,
    p_widget_img->widget.box.height,
    //p_widget_img->p_image->width,
    //p_widget_img->p_image->height,
    0,
    0
  );
}

void widget_image_init(widget_image_t *p_widget_img, tile_t *p_tile, int x, int y, int width, int height, image_t *p_image)
{
  /* Initialize the underlying widget. */
  widget_init(&p_widget_img->widget, p_tile, x, y, width, height);

  /* Set properties. */
  p_widget_img->p_image = p_image;

  /* Set user data. */
  widget_set_userdata(&p_widget_img->widget, (void *)p_widget_img);

  /* Set drawing function. */
  widget_set_drawfunc(&p_widget_img->widget, widget_image_drawfunc);
}
