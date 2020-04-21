#include "goom_config.h"
#include "goom_fx.h"
#include "goom_plugin_info.h"
#include "mathtools.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NB_THETA 512

#define MAX 2.0f

typedef struct _CONV_DATA {
  PluginParam light;
  PluginParam factor_adj_p;
  PluginParam factor_p;
  PluginParameters params;

  /* rotozoom */
  int theta;
  float ftheta;
  int h_sin[NB_THETA];
  int h_cos[NB_THETA];
  int h_height;
  float visibility;
} ConvData;

/* init rotozoom tables */
static void compute_tables(VisualFX* _this, PluginInfo* info)
{
  ConvData* data = (ConvData*)_this->fx_data;

  if (data->h_height == info->screen.height) {
    return;
  }

  const double screen_coef = 2.0 * 300.0 / (double)info->screen.height;
  data->h_height = info->screen.height;

  for (int i = 0; i < NB_THETA; i++) {
    const double radian = 2 * i * M_PI / NB_THETA;
    const double h = (0.2 + cos(radian) / 15.0 * sin(radian * 2.0 + 12.123)) * screen_coef;
    data->h_cos[i] = 0x10000 * (-h * cos(radian) * cos(radian));
    data->h_sin[i] = 0x10000 * (h * sin(radian + 1.57) * sin(radian));
  }
}

static void convolve_init(VisualFX* _this, PluginInfo* info)
{
  ConvData* data;
  data = (ConvData*)malloc(sizeof(ConvData));
  _this->fx_data = (void*)data;

  data->light = secure_f_param("Screen Brightness");
  data->light.param.fval.max = 300.0f;
  data->light.param.fval.step = 1.0f;
  data->light.param.fval.value = 100.0f;

  data->factor_adj_p = secure_f_param("Flash Intensity");
  data->factor_adj_p.param.fval.max = 200.0f;
  data->factor_adj_p.param.fval.step = 1.0f;
  data->factor_adj_p.param.fval.value = 70.0f;

  data->factor_p = secure_f_feedback("Factor");

  data->params = plugin_parameters("Bright Flash", 5);
  data->params.params[0] = &data->light;
  data->params.params[1] = &data->factor_adj_p;
  data->params.params[2] = 0;
  data->params.params[3] = &data->factor_p;
  data->params.params[4] = 0;

  /* init rotozoom tables */
  compute_tables(_this, info);
  data->theta = 0;
  data->ftheta = 0.0;
  data->visibility = 1.0;

  _this->params = &data->params;
}

static void convolve_free(VisualFX* _this)
{
  ConvData* data = _this->fx_data;
  free(data->params.params);
  free(data);
}

static void create_output_with_brightness(VisualFX* _this, Pixel* src, Pixel* dest,
                                          PluginInfo* info, unsigned int iff)
{
  int i = 0; //info->screen.height * info->screen.width - 1;
  for (int y = 0; y < info->screen.height; y++) {
    for (int x = 0; x < info->screen.width; x++) {
      const unsigned int f0 = (src[i].cop[0] * iff) >> 8;
      const unsigned int f1 = (src[i].cop[1] * iff) >> 8;
      const unsigned int f2 = (src[i].cop[2] * iff) >> 8;
      const unsigned int f3 = (src[i].cop[3] * iff) >> 8;

      dest[i].cop[0] = (f0 & 0xffffff00) ? 0xff : (unsigned char)f0;
      dest[i].cop[1] = (f1 & 0xffffff00) ? 0xff : (unsigned char)f1;
      dest[i].cop[2] = (f2 & 0xffffff00) ? 0xff : (unsigned char)f2;
      dest[i].cop[3] = (f3 & 0xffffff00) ? 0xff : (unsigned char)f3;

      i++;
    }
  }

  compute_tables(_this, info);
}

/*#include <stdint.h>

 static uint64_t GetTick()
 {
 uint64_t x;
 asm volatile ("RDTSC" : "=A" (x));
 return x;
 }*/

static void convolve_apply(VisualFX* _this, Pixel* src, Pixel* dest, PluginInfo* info)
{
  ConvData* data = (ConvData*)_this->fx_data;
  const float ff = (FVAL(data->factor_p) * FVAL(data->factor_adj_p) + FVAL(data->light)) / 100.0f;
  const unsigned int iff = (unsigned int)(ff * 256);
  {
    const double fcycle = (double)info->cycle;
    const float INCREASE_RATE = 1.5;
    const float DECAY_RATE = 0.955;
    if (FVAL(info->sound.last_goom_p) > 0.8) {
      FVAL(data->factor_p) += FVAL(info->sound.goom_power_p) * INCREASE_RATE;
    }
    FVAL(data->factor_p) *= DECAY_RATE;

    float rotate_param = FVAL(info->sound.last_goom_p);
    if (rotate_param < 0.0) {
      rotate_param = 0.0;
    }
    rotate_param += FVAL(info->sound.goom_power_p);

    const float rotate_coef = 4.0 + FVAL(info->sound.goom_power_p) * 6.0;
    data->ftheta = (data->ftheta + rotate_coef * sin(rotate_param * 6.3));
    data->theta = ((unsigned int)data->ftheta) % NB_THETA;
    data->visibility = (cos(fcycle * 0.001 + 1.5) * sin(fcycle * 0.008) +
                        cos(fcycle * 0.011 + 5.0) - 0.8 + info->sound.speedvar) *
                       1.5;
    if (data->visibility < 0.0) {
      data->visibility = 0.0;
    }
    data->factor_p.change_listener(&data->factor_p);
  }

  if ((ff > 0.98f) && (ff < 1.02f)) {
    memcpy(dest, src, (size_t)info->screen.size * sizeof(Pixel));
  } else {
    create_output_with_brightness(_this, src, dest, info, iff);
  }
  /*
   //   Benching suite...
   {
   uint64_t before, after;
   double   timed;
   static double stimed = 10000.0;
   before = GetTick();
   data->visibility = 1.0;
   create_output_with_brightness(_this,src,dest,info,iff);
   after  = GetTick();
   timed = (double)((after-before) / info->screen.size);
   if (timed < stimed) {
   stimed = timed;
   printf ("CLK = %3.0f CPP\n", stimed);
   }
   }
   */
}

VisualFX convolve_create(void)
{
  VisualFX vfx;
  vfx.init = convolve_init;
  vfx.free = convolve_free;
  vfx.apply = convolve_apply;
  vfx.save = NULL;
  vfx.restore = NULL;
  vfx.fx_data = 0;
  return vfx;
}
