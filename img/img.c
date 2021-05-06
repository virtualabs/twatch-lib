#include "img.h"
#include "drivers/st7789.h"

#define SCREEN_HEIGHT 240
#define IMG_DATA(x) ((uint8_t *)((uint8_t *)(x) + 4))

image_t *load_image(uint8_t *bitmap_data)
{
  return (image_t *)bitmap_data;
}

uint16_t _get_pixel_1bpp(image_t *source, int x, int y)
{
  int pos = (y*source->width + x)/8;
  uint8_t mask = (1<<((y*source->width + x)%8));
  return (((uint8_t *)source)[pos+6] & mask)?0xfff:0;
}

uint16_t _get_pixel_12bpp(image_t *source, int x, int y)
{
  uint16_t *p_img_raw = (uint16_t *)source;
  return (p_img_raw[y*source->width + x + 3]) & 0xfff;
}

void _screen_bitblt_1bpp(image_t *source, int source_x, int source_y, int width, int height, int dest_x, int dest_y)
{
  int x, y;

  for (x=0;x<width;x++)
  {
    for (y=0;y<height;y++)
      st7789_set_pixel(dest_x+x, dest_y+y, _get_pixel_1bpp(source, source_x+x, source_y+y));
  }
}

/* -- Old version of _screen_bitblt_12bpp -- 
void _screen_bitblt_12bpp(image_t *source, int source_x, int source_y, int width, int height, int dest_x, int dest_y)
{
  int x, y;

  for (x=0;x<width;x++)
  {
    for (y=0;y<height;y++)
      st7789_set_pixel(dest_x+x, dest_y+y, _get_pixel_12bpp(source, source_x+x, source_y+y));
  }
}*/

/* Optimized version of _screen_bitblt_12bpp */
void _screen_bitblt_12bpp(image_t *source, int source_x, int source_y, int width, int height, int dest_x, int dest_y)
{
  int x, y;
  uint16_t *p_pixels = (uint16_t *)((uint8_t *)source + sizeof(image_t));

  if (dest_y < 0)
  {
    source_y -= dest_y;
    height += dest_y;
    dest_y = 0;
  }

  if ((height + dest_y) >= SCREEN_HEIGHT)
  {
    height = SCREEN_HEIGHT - dest_y;
  }

  for (y=0;y<height;y++)
  {
    st7789_copy_line(
      dest_x,
      dest_y+y,
      &p_pixels[(source_y+y)*source->width + source_x],
      width
    );
  }
}

void screen_bitblt(image_t *source, int source_x, int source_y, int width, int height, int dest_x, int dest_y)
{
  switch(source->type)
  {
    /* Raw image. */
    case IMAGE_RAW:
      {
        switch (source->depth)
        {
          /* Black & white images. */
          case DEPTH_1BPP:
            _screen_bitblt_1bpp(source, source_x, source_y, width, height, dest_x, dest_y);
            break;


          /* Unsupported yet. */
          case DEPTH_12BPP:
            _screen_bitblt_12bpp(source, source_x, source_y, width, height, dest_x, dest_y);
            break;

          default:
            break;
        }
      }
      break;

    /* Unsupported. */
    default:
      break;
  }
}
