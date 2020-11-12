#ifndef __INC_TWATCH_FONT_H
#define __INC_TWATCH_FONT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "drivers/st7789.h"

#define nr_chrs_f16 96
#define chr_hgt_f16 16
#define baseline_f16 13
#define data_size_f16 8
#define firstchr_f16 32

int font_get_text_width(char *psz_text);
int font_get_text_width_x2(char *psz_text);
int font_draw_char(int x, int y, char c, uint16_t color);
int font_draw_char_x2(int x, int y, char c, uint16_t color);
int font_draw_text(int x, int y, char *psz_text, uint16_t color);
int font_draw_text_x2(int x, int y, char *psz_text, uint16_t color);



#endif /* __INC_TWATCH_FONT_H */