#include "surf3d.h"
#include "goom_plugin_info.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

grid3d* grid3d_new(const int x_width, const int num_x, const int z_depth, const int num_z, const v3d center)
{
  grid3d* g = (grid3d*)malloc(sizeof(grid3d));
  surf3d* s = &(g->surf);
  s->nbvertex = num_x * num_z;
  s->center = center;
  s->vertex = (v3d*)malloc((size_t)(s->nbvertex) * sizeof(v3d));
  s->svertex = (v3d*)malloc((size_t)(s->nbvertex) * sizeof(v3d));

  g->defx = num_x;
  g->sizex = x_width;
  g->defz = num_z;
  g->sizez = z_depth;
  g->mode = 0;

  const float x_step = x_width / (float)(num_x-1);
  const float z_step = z_depth / (float)(num_z-1);
  const float x_start = x_width / 2.0;
  const float z_start = z_depth / 2.0;

  float z = z_start;
  for (int nz=num_z-1; nz >= 0; nz--) {
    const int nx_start = num_x * nz;
    float x = x_start;
    for (int nx=num_x-1; nx >= 0; nx--) {
      const int nv = nx + nx_start;
      s->vertex[nv].x = x;
      s->vertex[nv].y = 0.0;
      s->vertex[nv].z = z;
      x -= x_step;
    }
    z -= z_step;
  }
  return g;
}

void grid3d_draw (PluginInfo *plug, grid3d *g, int color, int colorlow,
	int dist, Pixel *buf, Pixel *back, int W,int H) {

	int x;
	v2d v2,v2x;

	v2d *v2_array = malloc(g->surf.nbvertex * sizeof(v2d));
	v3d_to_v2d(g->surf.svertex, g->surf.nbvertex, W, H, dist, v2_array);
	
	for (x=0;x<g->defx;x++) {
		int z;
		v2x = v2_array[x];

		for (z=1;z<g->defz;z++) {
			v2 = v2_array[z*g->defx + x];
			if (((v2.x != -666) || (v2.y!=-666))
					&& ((v2x.x != -666) || (v2x.y!=-666))) {
				plug->methods.draw_line (buf,v2x.x,v2x.y,v2.x,v2.y, colorlow, W, H);
				plug->methods.draw_line (back,v2x.x,v2x.y,v2.x,v2.y, color, W, H);
			}
			v2x = v2;
		}
	}

	free(v2_array);
}

void surf3d_rotate (surf3d *s, float angle) {
	int i;
	float cosa;
	float sina;
	SINCOS(angle,sina,cosa);
	for (i=0;i<s->nbvertex;i++) {
		Y_ROTATE_V3D(s->vertex[i],s->svertex[i],cosa,sina);
	}
}

void surf3d_translate (surf3d *s) {
	int i;
	for (i=0;i<s->nbvertex;i++) {
		TRANSLATE_V3D(s->center,s->svertex[i]);
	}
}

void grid3d_update (grid3d *g, float angle, float *vals, float dist) {
	int i;
	float cosa;
	float sina;
	surf3d *s = &(g->surf);
	v3d cam = s->center;
	cam.z += dist;

	SINCOS((angle/4.3f),sina,cosa);
	cam.y += sina*2.0f;
	SINCOS(angle,sina,cosa);

	if (g->mode==0) {
		if (vals)
			for (i=0;i<g->defx;i++)
				s->vertex[i].y = s->vertex[i].y*0.2 + vals[i]*0.8;

		for (i=g->defx;i<s->nbvertex;i++) {
			s->vertex[i].y *= 0.255f;
			s->vertex[i].y += (s->vertex[i-g->defx].y * 0.777f);
		}
	}

	for (i=0;i<s->nbvertex;i++) {
		Y_ROTATE_V3D(s->vertex[i],s->svertex[i],cosa,sina);
		TRANSLATE_V3D(cam,s->svertex[i]);
	}
}
