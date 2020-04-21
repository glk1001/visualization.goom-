#ifndef _DRAWMETHODS_H
#define _DRAWMETHODS_H

#include "goom_config.h"
#include "goom_graphic.h"

#include <stdint.h>

void draw_line(Pixel* data, int x1, int y1, int x2, int y2, uint32_t col, int screenx, int screeny);

#endif /* _DRAWMETHODS_H */
