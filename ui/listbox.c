#include "ui/listbox.h"

void widget_listbox_drawfunc(widget_t *p_widget)
{
  int text_width, dx, dy;
  widget_listbox_t *p_listbox = (widget_listbox_t *)p_widget->p_user_data;

  if (p_listbox != NULL)
  {
    /* Position container (position not updated while animating). */
    p_listbox->container.widget.box.x = widget_get_abs_x(p_widget) + 1;
    p_listbox->container.widget.box.y = widget_get_abs_y(p_widget) + 1;

    /* Draw container. */
    widget_draw(&p_listbox->container);

    /* Draw listbox borders. */
   /* Draw the button. */
    widget_draw_line(
      p_widget,
      1,
      0,
      p_widget->box.width - 2,
      0,
      LISTBOX_STYLE_BORDER
    );
    widget_draw_line(
      p_widget,
      1,
      p_widget->box.height - 1,
      p_widget->box.width - 2,
      p_widget->box.height - 1,
      LISTBOX_STYLE_BORDER
    );
    widget_draw_line(
      p_widget,
      0,
      1,
      0,
      p_widget->box.height - 2,
      LISTBOX_STYLE_BORDER
    );
    widget_draw_line(
      p_widget,
      p_widget->box.width - 1,
      1,
      p_widget->box.width - 1,
      p_widget->box.height - 2,
      LISTBOX_STYLE_BORDER
    );
  }
}

void widget_listbox_init(widget_listbox_t *p_widget_listbox, tile_t *p_tile, int x, int y, int width, int height)
{
  /* Initialize our widget. */
  widget_init(&p_widget_listbox->widget, p_tile, x, y, width, height);

  /* Initialize our container (width = listbox width - scrollbar width). */
  widget_container_init(
    &p_widget_listbox->container,
    NULL,
    x + 1,
    y + 1,
    width - 2,
    height - 2
  );

  /* No items. */
  p_widget_listbox->n_items = 0;
  p_widget_listbox->selected_index = -1;

  /* Set user data. */
  widget_set_userdata(&p_widget_listbox->widget, (void *)p_widget_listbox);

  /* Set drawing function. */
  widget_set_drawfunc(&p_widget_listbox->widget, widget_listbox_drawfunc);
}

void widget_listbox_add(widget_listbox_t *p_widget_listbox, widget_t *p_widget)
{
  widget_container_item_t *p_item = NULL;

  /* Does our container contains at least one item ? */
  if (p_widget_listbox->container.p_children != NULL)
  {
    /* Yes, append our widget after the last item. */
    p_item = p_widget_listbox->container.p_children;
    while (p_item->p_next != NULL)
      p_item = p_item->p_next;
  }
  
  /* Update our new widget position and width based on previous widget. */
  p_widget->box.x = 2;
  if (p_item != NULL)
  {
    p_widget->box.y = p_item->rel_box.y + p_item->rel_box.height;
  }
  else
  {
    p_widget->box.y = 2;
  }

  p_widget->box.width = p_widget_listbox->container.widget.box.width - 4;

  /* Add widget to container. */
  widget_container_add(&p_widget_listbox->container, p_widget);

}

void widget_listbox_remove(widget_listbox_t *p_widget_listbox, widget_t *p_widget)
{

}