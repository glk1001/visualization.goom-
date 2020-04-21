#ifndef _GOOM_FX_H
#define _GOOM_FX_H

#include "goom_plugin_info.h"
#include "goom_visual_fx.h"

VisualFX convolve_create();
VisualFX flying_star_create(void);

void zoom_filter_c(int sizeX, int sizeY, Pixel* src, Pixel* dest, int* brutS, int* brutD,
                   int buffratio, int precalCoef[16][16]);

#endif
