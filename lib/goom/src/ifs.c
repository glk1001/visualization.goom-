/*
 * ifs.c --- modified iterated functions system for goom.
 */

/*-
 * Copyright (c) 1997 by Massimino Pascal <Pascal.Massimon@ens.fr>
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * If this mode is weird and you have an old MetroX server, it is buggy.
 * There is a free SuSE-enhanced MetroX X server that is fine.
 *
 * When shown ifs, Diana Rose (4 years old) said, "It looks like dancing."
 *
 * Revision History:
 * 13-Dec-2003: Added some goom specific stuffs (to make ifs a VisualFX).
 * 11-Apr-2002: jeko@ios-software.com: Make ifs.c system-indendant. (ifs.h added)
 * 01-Nov-2000: Allocation checks
 * 10-May-1997: jwz@jwz.org: turned into a standalone program.
 *              Made it render into an offscreen bitmap and then copy
 *              that onto the screen, to reduce flicker.
 */

/* #ifdef STANDALONE */

#include "ifs.h"
#include "goom.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_tools.h"
#include "goom_testing.h"
#include "mathtools.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct _ifsPoint {
  gint32 x, y;
} IFSPoint;

#define MODE_ifs

#define PROGCLASS "IFS"

#define HACK_INIT init_ifs
#define HACK_DRAW draw_ifs

#define ifs_opts xlockmore_opts

#define DEFAULTS "*delay: 20000 \n" \
"*ncolors: 100 \n"

#define SMOOTH_COLORS

#define LRAND()  ((long) (goom_random(goomInfo->gRandom) & 0x7fffffff))
#define NRAND(n) ((int) (LRAND() % (n)))

#if RAND_MAX < 0x10000
#  define MAXRAND (((float)(RAND_MAX<16)+((float)RAND_MAX)+1.0f)/127.0f)
#else
#  define MAXRAND (2147483648.0/127.0)           /* unsigned 1<<31 / 127.0 (cf goom_tools) as a float */
#endif

/*****************************************************/

typedef float DBL;
typedef int F_PT;
/* typedef float               F_PT; */

/*****************************************************/

#define FIX 12
#define UNIT   ( 1<<FIX )
#define MAX_SIMI  6

#define MAX_DEPTH_2  10
#define MAX_DEPTH_3  6
#define MAX_DEPTH_4  4
#define MAX_DEPTH_5  2

/* PREVIOUS VALUE 
#define MAX_SIMI  6

 * settings for a PC 120Mhz... *
#define MAX_DEPTH_2  10
#define MAX_DEPTH_3  6
#define MAX_DEPTH_4  4
#define MAX_DEPTH_5  3
*/

#define DBL_To_F_PT(x)  (F_PT)( (DBL)(UNIT)*(x) )

typedef struct Similitude_Struct SIMI;
typedef struct Fractal_Struct FRACTAL;

struct Similitude_Struct {
  DBL c_x, c_y;
  DBL r, r2, A, A2;
  F_PT Ct, St, Ct2, St2;
  F_PT Cx, Cy;
  F_PT R, R2;
};

struct Fractal_Struct {
  int Nb_Simi;
  SIMI Components[5 * MAX_SIMI];
  int Depth, Col;
  int Count, Speed;
  int Width, Height, Lx, Ly;
  DBL r_mean, dr_mean, dr2_mean;
  int Cur_Pt, Max_Pt;

  IFSPoint *Buffer1, *Buffer2;
};

typedef struct _IFS_DATA {
  PluginParam enabled_bp;
  PluginParameters params;

  FRACTAL *Root;
  FRACTAL *Cur_F;

  /* Used by the Trace recursive method */
  IFSPoint *Buf;
  int Cur_Pt;
  int initalized;
} IfsData;

/*****************************************************/

static DBL Gauss_Rand(PluginInfo *goomInfo, DBL c, DBL S, DBL A_mult_1_minus_exp_neg_S)
{
  DBL y = (DBL) LRAND() / MAXRAND;
  y = A_mult_1_minus_exp_neg_S * (1.0 - exp(-y * y * S));
  if (NRAND(2)) {
    return (c + y);
  }
  return (c - y);
}

static DBL Half_Gauss_Rand(PluginInfo *goomInfo, DBL c, DBL S, DBL A_mult_1_minus_exp_neg_S)
{
  DBL y = (DBL) LRAND() / MAXRAND;
  y = A_mult_1_minus_exp_neg_S * (1.0 - exp(-y * y * S));
  return (c + y);
}

static DBL get_1_minus_exp_neg_S(DBL S)
{
  return 1.0 - exp(-S);
}

static void Random_Simis(PluginInfo *goomInfo, FRACTAL *F, SIMI *Cur, int i)
{
  static DBL c_AS_factor;
  static DBL r_1_minus_exp_neg_S;
  static DBL r2_1_minus_exp_neg_S;
  static DBL A_AS_factor;
  static DBL A2_AS_factor;

  static int doneInit = 0;
  if (!doneInit) {
    c_AS_factor = 0.8 * get_1_minus_exp_neg_S(4.0);
    r_1_minus_exp_neg_S = get_1_minus_exp_neg_S(3.0);
    r2_1_minus_exp_neg_S = get_1_minus_exp_neg_S(2.0);
    A_AS_factor = 360.0 * get_1_minus_exp_neg_S(4.0);
    A2_AS_factor = A_AS_factor;
    doneInit = 1;
  }
  const DBL r_AS_factor = F->dr_mean * r_1_minus_exp_neg_S;
  const DBL r2_AS_factor = F->dr2_mean * r2_1_minus_exp_neg_S;

  while (i--) {
    Cur->c_x = Gauss_Rand(goomInfo, 0.0, 4.0, c_AS_factor);
    Cur->c_y = Gauss_Rand(goomInfo, 0.0, 4.0, c_AS_factor);
    Cur->r = Gauss_Rand(goomInfo, F->r_mean, 3.0, r_AS_factor);
    Cur->r2 = Half_Gauss_Rand(goomInfo, 0.0, 2.0, r2_AS_factor);
    Cur->A = Gauss_Rand(goomInfo, 0.0, 4.0, A_AS_factor) * (M_PI / 180.0);
    Cur->A2 = Gauss_Rand(goomInfo, 0.0, 4.0, A2_AS_factor) * (M_PI / 180.0);
    Cur->Ct = 0;
    Cur->St = 0;
    Cur->Ct2 = 0;
    Cur->St2 = 0;
    Cur->Cx = 0;
    Cur->Cy = 0;
    Cur->R = 0;
    Cur->R2 = 0;
    Cur++;
  }
}

static void free_ifs_buffers(FRACTAL *Fractal)
{
  if (Fractal->Buffer1 != NULL) {
    (void) free((void*) Fractal->Buffer1);
    Fractal->Buffer1 = (IFSPoint*) NULL;
  }
  if (Fractal->Buffer2 != NULL) {
    (void) free((void*) Fractal->Buffer2);
    Fractal->Buffer2 = (IFSPoint*) NULL;
  }
}

static void free_ifs(FRACTAL *Fractal)
{
  free_ifs_buffers(Fractal);
}

/***************************************************************/

static void init_ifs(PluginInfo *goomInfo, IfsData *data)
{
  int width = goomInfo->screen.width;
  int height = goomInfo->screen.height;

  data->enabled_bp = secure_b_param("Enabled", 1);

  if (data->Root == NULL) {
    data->Root = (FRACTAL*) malloc(sizeof(FRACTAL));
    if (data->Root == NULL)
      return;
    data->Root->Buffer1 = (IFSPoint*) NULL;
    data->Root->Buffer2 = (IFSPoint*) NULL;
  }
  FRACTAL *Fractal = data->Root;

  free_ifs_buffers(Fractal);

  int i = (NRAND(4)) + 2;/* Number of centers */
  switch (i) {
  case 3:
    Fractal->Depth = MAX_DEPTH_3;
    Fractal->r_mean = .6;
    Fractal->dr_mean = .4;
    Fractal->dr2_mean = .3;
    break;

  case 4:
    Fractal->Depth = MAX_DEPTH_4;
    Fractal->r_mean = .5;
    Fractal->dr_mean = .4;
    Fractal->dr2_mean = .3;
    break;

  case 5:
    Fractal->Depth = MAX_DEPTH_5;
    Fractal->r_mean = .5;
    Fractal->dr_mean = .4;
    Fractal->dr2_mean = .3;
    break;

  default:
  case 2:
    Fractal->Depth = MAX_DEPTH_2;
    Fractal->r_mean = .7;
    Fractal->dr_mean = .3;
    Fractal->dr2_mean = .4;
    break;
  }

  Fractal->Nb_Simi = i;
  Fractal->Max_Pt = Fractal->Nb_Simi - 1;
  for (i = 0; i <= Fractal->Depth + 2; ++i) {
    Fractal->Max_Pt *= Fractal->Nb_Simi;
  }

  if ((Fractal->Buffer1 = (IFSPoint*) calloc(Fractal->Max_Pt, sizeof(IFSPoint))) == NULL) {
    free_ifs(Fractal);
    return;
  }
  if ((Fractal->Buffer2 = (IFSPoint*) calloc(Fractal->Max_Pt, sizeof(IFSPoint))) == NULL) {
    free_ifs(Fractal);
    return;
  }

  Fractal->Speed = 6;
  Fractal->Width = width;/* modif by JeKo */
  Fractal->Height = height;/* modif by JeKo */
  Fractal->Cur_Pt = 0;
  Fractal->Count = 0;
  Fractal->Lx = (Fractal->Width - 1) / 2;
  Fractal->Ly = (Fractal->Height - 1) / 2;
  Fractal->Col = pcg32_rand() % (width * height);/* modif by JeKo */

  Random_Simis(goomInfo, Fractal, Fractal->Components, 5 * MAX_SIMI);

  for (int i = 0; i < 5 * MAX_SIMI; i++) {
      SIMI cur = Fractal->Components[i];
      GOOM_LOG_DEBUG("simi[%d]: c_x = %f, c_y = %f, r = %f, r2 = %f, A = %f, A2 = %f.",
          i, cur.c_x, cur.c_y, cur.r, cur.r2, cur.A, cur.A2);
  }
}

/***************************************************************/

static inline void Transform(SIMI *Simi, F_PT xo, F_PT yo, F_PT *x, F_PT *y)
{
  F_PT xx, yy;

  xo = xo - Simi->Cx;
  xo = (xo * Simi->R) >> FIX;/* / UNIT; */
  yo = yo - Simi->Cy;
  yo = (yo * Simi->R) >> FIX;/* / UNIT; */

  xx = xo - Simi->Cx;
  xx = (xx * Simi->R2) >> FIX;/* / UNIT; */
  yy = -yo - Simi->Cy;
  yy = (yy * Simi->R2) >> FIX;/* / UNIT; */

  *x = ((xo * Simi->Ct - yo * Simi->St + xx * Simi->Ct2 - yy * Simi->St2) >> FIX/* / UNIT */) + Simi->Cx;
  *y = ((xo * Simi->St + yo * Simi->Ct + xx * Simi->St2 + yy * Simi->Ct2) >> FIX/* / UNIT */) + Simi->Cy;
}

/***************************************************************/

static void Trace(FRACTAL *F, F_PT xo, F_PT yo, IfsData *data)
{
  F_PT x, y, i;
  SIMI *Cur;

  Cur = data->Cur_F->Components;
//	GOOM_LOG_DEBUG("data->Cur_F->Nb_Simi = %d, xo = %d, yo = %d", data->Cur_F->Nb_Simi, xo, yo);
  for (i = data->Cur_F->Nb_Simi; i; --i, Cur++) {
    Transform(Cur, xo, yo, &x, &y);

    data->Buf->x = F->Lx + ((x * F->Lx) >> (FIX + 1)/* /(UNIT*2) */);
    data->Buf->y = F->Ly - ((y * F->Ly) >> (FIX + 1)/* /(UNIT*2) */);
    data->Buf++;

    data->Cur_Pt++;

    if (F->Depth && ((x - xo) >> 4) && ((y - yo) >> 4)) {
      F->Depth--;
      Trace(F, x, y, data);
      F->Depth++;
    }
  }
}

static void Draw_Fractal(IfsData *data)
{
  FRACTAL *F = data->Root;
  int i, j;
  F_PT x, y, xo, yo;
  SIMI *Cur, *Simi;

  for (Cur = F->Components, i = F->Nb_Simi; i; --i, Cur++) {
    Cur->Cx = DBL_To_F_PT(Cur->c_x);
    Cur->Cy = DBL_To_F_PT(Cur->c_y);

    Cur->Ct = DBL_To_F_PT(cos(Cur->A));
    Cur->St = DBL_To_F_PT(sin(Cur->A));
    Cur->Ct2 = DBL_To_F_PT(cos(Cur->A2));
    Cur->St2 = DBL_To_F_PT(sin(Cur->A2));

    Cur->R = DBL_To_F_PT(Cur->r);
    Cur->R2 = DBL_To_F_PT(Cur->r2);
  }

  data->Cur_Pt = 0;
  data->Cur_F = F;
  data->Buf = F->Buffer2;
  for (Cur = F->Components, i = F->Nb_Simi; i; --i, Cur++) {
    xo = Cur->Cx;
    yo = Cur->Cy;
    GOOM_LOG_DEBUG("F->Nb_Simi = %d, xo = %d, yo = %d", F->Nb_Simi, xo, yo);
    for (Simi = F->Components, j = F->Nb_Simi; j; --j, Simi++) {
      if (Simi == Cur) {
        continue;
      }
      Transform(Simi, xo, yo, &x, &y);
      Trace(F, x, y, data);
    }
  }

  /* Erase previous */
  F->Cur_Pt = data->Cur_Pt;
  data->Buf = F->Buffer1;
  F->Buffer1 = F->Buffer2;
  F->Buffer2 = data->Buf;
}

static IFSPoint* draw_ifs(PluginInfo *goomInfo, int *nbpt, IfsData *data)
{
  int i;
  DBL u, uu, v, vv, u0, u1, u2, u3;
  SIMI *S, *S1, *S2, *S3, *S4;
  FRACTAL *F;

  if (data->Root == NULL) {
    return NULL;
  }
  F = data->Root;
  if (F->Buffer1 == NULL) {
    return NULL;
  }

  u = (DBL) (F->Count) * (DBL) (F->Speed) / 1000.0;
  uu = u * u;
  v = 1.0 - u;
  vv = v * v;
  u0 = vv * v;
  u1 = 3.0 * vv * u;
  u2 = 3.0 * v * uu;
  u3 = u * uu;

  S = F->Components;
  S1 = S + F->Nb_Simi;
  S2 = S1 + F->Nb_Simi;
  S3 = S2 + F->Nb_Simi;
  S4 = S3 + F->Nb_Simi;

  for (i = F->Nb_Simi; i; --i, S++, S1++, S2++, S3++, S4++) {
    S->c_x = u0 * S1->c_x + u1 * S2->c_x + u2 * S3->c_x + u3 * S4->c_x;
    S->c_y = u0 * S1->c_y + u1 * S2->c_y + u2 * S3->c_y + u3 * S4->c_y;
    S->r = u0 * S1->r + u1 * S2->r + u2 * S3->r + u3 * S4->r;
    S->r2 = u0 * S1->r2 + u1 * S2->r2 + u2 * S3->r2 + u3 * S4->r2;
    S->A = u0 * S1->A + u1 * S2->A + u2 * S3->A + u3 * S4->A;
    S->A2 = u0 * S1->A2 + u1 * S2->A2 + u2 * S3->A2 + u3 * S4->A2;
  }

  Draw_Fractal(data);

  if (F->Count >= 1000 / F->Speed) {
    S = F->Components;
    S1 = S + F->Nb_Simi;
    S2 = S1 + F->Nb_Simi;
    S3 = S2 + F->Nb_Simi;
    S4 = S3 + F->Nb_Simi;

    for (i = F->Nb_Simi; i; --i, S++, S1++, S2++, S3++, S4++) {
      S2->c_x = 2.0 * S4->c_x - S3->c_x;
      S2->c_y = 2.0 * S4->c_y - S3->c_y;
      S2->r = 2.0 * S4->r - S3->r;
      S2->r2 = 2.0 * S4->r2 - S3->r2;
      S2->A = 2.0 * S4->A - S3->A;
      S2->A2 = 2.0 * S4->A2 - S3->A2;

      *S1 = *S4;
    }

    Random_Simis(goomInfo, F, F->Components + 3 * F->Nb_Simi, F->Nb_Simi);
    Random_Simis(goomInfo, F, F->Components + 4 * F->Nb_Simi, F->Nb_Simi);

    F->Count = 0;
  } else {
    F->Count++;
  }

  F->Col++;

  (*nbpt) = data->Cur_Pt;
  return F->Buffer2;
}

/***************************************************************/

static void release_ifs(IfsData *data)
{
  if (data->Root != NULL) {
    free_ifs(data->Root);
    (void) free((void*) data->Root);
    data->Root = (FRACTAL*) NULL;
  }
}

#define RAND() goom_random(goomInfo->gRandom)
#define MOD_MER 0
#define MOD_FEU 1
#define MOD_MERVER 2

static struct {
  int justChanged;
  int couleur;
  int v[4];
  int col[4];
  int mode;
  int cycle;
} ifs_update_data = {
    0,
    0xc0c0c0c0,
    { 2, 4, 3, 2 },
    { 2, 4, 3, 2 },
    MOD_MERVER,
    0
};

static void ifs_update(PluginInfo *goomInfo, Pixel *data, Pixel *back, int increment, IfsData *fx_data)
{
  GOOM_LOG_DEBUG("increment = %d", increment);

  int couleursl = ifs_update_data.couleur;
  int width = goomInfo->screen.width;
  int height = goomInfo->screen.height;

  ifs_update_data.cycle++;
  if (ifs_update_data.cycle >= 80) {
    ifs_update_data.cycle = 0;
  }

  int cycle10;
  if (ifs_update_data.cycle < 40) {
    cycle10 = ifs_update_data.cycle / 10;
  } else {
    cycle10 = 7 - ifs_update_data.cycle / 10;
  }

  {
    unsigned char *tmp = (unsigned char*) &couleursl;

    for (int i = 0; i < 4; i++) {
      *tmp = (*tmp) >> cycle10;
      tmp++;
    }
  }

  int nbpt;
  IFSPoint *points = draw_ifs(goomInfo, &nbpt, fx_data);
  nbpt--;
  GOOM_LOG_DEBUG("nbpt = %d", nbpt);

  for (int i = 0; i < nbpt; i += increment) {
    int x = (int) points[i].x & 0x7fffffff;
    int y = (int) points[i].y & 0x7fffffff;

    if ((x < width) && (y < height)) {
      int pos = x + (int) (y * width);
      int tra = 0, i = 0;
      unsigned char *bra = (unsigned char*) &back[pos];
      unsigned char *dra = (unsigned char*) &data[pos];
      unsigned char *cra = (unsigned char*) &couleursl;

      for (; i < 4; i++) {
        tra = *cra;
        tra += *bra;
        if (tra > 255)
          tra = 255;
        *dra = tra;
        ++dra;
        ++cra;
        ++bra;
      }
    }
  }

  ifs_update_data.justChanged--;
  GOOM_LOG_DEBUG("ifs_update_data.justChanged = %d", ifs_update_data.justChanged);

  ifs_update_data.col[ALPHA] = ifs_update_data.couleur >> (ALPHA * 8) & 0xff;
  ifs_update_data.col[BLEU] = ifs_update_data.couleur >> (BLEU * 8) & 0xff;
  ifs_update_data.col[VERT] = ifs_update_data.couleur >> (VERT * 8) & 0xff;
  ifs_update_data.col[ROUGE] = ifs_update_data.couleur >> (ROUGE * 8) & 0xff;

  GOOM_LOG_DEBUG("ifs_update_data.col[ALPHA] = %d", ifs_update_data.col[ALPHA]);
  GOOM_LOG_DEBUG("ifs_update_data.col[BLEU] = %d", ifs_update_data.col[BLEU]);
  GOOM_LOG_DEBUG("ifs_update_data.col[VERT] = %d", ifs_update_data.col[VERT]);
  GOOM_LOG_DEBUG("ifs_update_data.col[ROUGE] = %d", ifs_update_data.col[ROUGE]);

  GOOM_LOG_DEBUG("ifs_update_data.v[ALPHA] = %d", ifs_update_data.v[ALPHA]);
  GOOM_LOG_DEBUG("ifs_update_data.v[BLEU] = %d", ifs_update_data.v[BLEU]);
  GOOM_LOG_DEBUG("ifs_update_data.v[VERT] = %d", ifs_update_data.v[VERT]);
  GOOM_LOG_DEBUG("ifs_update_data.v[ROUGE] = %d", ifs_update_data.v[ROUGE]);

  GOOM_LOG_DEBUG("ifs_update_data.mode = %d", ifs_update_data.mode);

  if (ifs_update_data.mode == MOD_MER) {
    ifs_update_data.col[BLEU] += ifs_update_data.v[BLEU];
    if (ifs_update_data.col[BLEU] > 255) {
      ifs_update_data.col[BLEU] = 255;
      ifs_update_data.v[BLEU] = -(RAND() % 4) - 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[BLEU] = %d", ifs_update_data.v[BLEU]);
    }
    if (ifs_update_data.col[BLEU] < 32) {
      ifs_update_data.col[BLEU] = 32;
      ifs_update_data.v[BLEU] = (RAND() % 4) + 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[BLEU] = %d", ifs_update_data.v[BLEU]);
    }

    ifs_update_data.col[VERT] += ifs_update_data.v[VERT];
    if (ifs_update_data.col[VERT] > 200) {
      ifs_update_data.col[VERT] = 200;
      ifs_update_data.v[VERT] = -(RAND() % 3) - 2;
      GOOM_LOG_DEBUG("ifs_update_data.v[VERT] = %d", ifs_update_data.v[VERT]);
    }
    if (ifs_update_data.col[VERT] > ifs_update_data.col[BLEU]) {
      ifs_update_data.col[VERT] = ifs_update_data.col[BLEU];
      ifs_update_data.v[VERT] = ifs_update_data.v[BLEU];
    }
    if (ifs_update_data.col[VERT] < 32) {
      ifs_update_data.col[VERT] = 32;
      ifs_update_data.v[VERT] = (RAND() % 3) + 2;
      GOOM_LOG_DEBUG("ifs_update_data.v[VERT] = %d", ifs_update_data.v[VERT]);
    }

    ifs_update_data.col[ROUGE] += ifs_update_data.v[ROUGE];
    if (ifs_update_data.col[ROUGE] > 64) {
      ifs_update_data.col[ROUGE] = 64;
      ifs_update_data.v[ROUGE] = -(RAND () % 4) - 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ROUGE] = %d", ifs_update_data.v[ROUGE]);
    }
    if (ifs_update_data.col[ROUGE] < 0) {
      ifs_update_data.col[ROUGE] = 0;
      ifs_update_data.v[ROUGE] = (RAND () % 4) + 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ROUGE] = %d", ifs_update_data.v[ROUGE]);
    }

    ifs_update_data.col[ALPHA] += ifs_update_data.v[ALPHA];
    if (ifs_update_data.col[ALPHA] > 0) {
      ifs_update_data.col[ALPHA] = 0;
      ifs_update_data.v[ALPHA] = -(RAND () % 4) - 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ALPHA] = %d", ifs_update_data.v[ALPHA]);
    }
    if (ifs_update_data.col[ALPHA] < 0) {
      ifs_update_data.col[ALPHA] = 0;
      ifs_update_data.v[ALPHA] = (RAND () % 4) + 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ALPHA] = %d", ifs_update_data.v[ALPHA]);
    }

    if (((ifs_update_data.col[VERT] > 32) && (ifs_update_data.col[ROUGE] < ifs_update_data.col[VERT] + 40)
        && (ifs_update_data.col[VERT] < ifs_update_data.col[ROUGE] + 20) && (ifs_update_data.col[BLEU] < 64)
        && (RAND () % 20 == 0)) && (ifs_update_data.justChanged < 0)) {
      ifs_update_data.mode = RAND () % 3 ? MOD_FEU : MOD_MERVER;
      GOOM_LOG_DEBUG("ifs_update_data.mode = %d", ifs_update_data.mode);
      ifs_update_data.justChanged = 250;
    }
  } else if (ifs_update_data.mode == MOD_MERVER) {
    ifs_update_data.col[BLEU] += ifs_update_data.v[BLEU];
    if (ifs_update_data.col[BLEU] > 128) {
      ifs_update_data.col[BLEU] = 128;
      ifs_update_data.v[BLEU] = -(RAND () % 4) - 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[BLEU] = %d", ifs_update_data.v[BLEU]);
    }
    if (ifs_update_data.col[BLEU] < 16) {
      ifs_update_data.col[BLEU] = 16;
      ifs_update_data.v[BLEU] = (RAND () % 4) + 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[BLEU] = %d", ifs_update_data.v[BLEU]);
    }

    ifs_update_data.col[VERT] += ifs_update_data.v[VERT];
    if (ifs_update_data.col[VERT] > 200) {
      ifs_update_data.col[VERT] = 200;
      ifs_update_data.v[VERT] = -(RAND () % 3) - 2;
      GOOM_LOG_DEBUG("ifs_update_data.v[VERT] = %d", ifs_update_data.v[VERT]);
    }
    if (ifs_update_data.col[VERT] > ifs_update_data.col[ALPHA]) {
      ifs_update_data.col[VERT] = ifs_update_data.col[ALPHA];
      ifs_update_data.v[VERT] = ifs_update_data.v[ALPHA];
    }
    if (ifs_update_data.col[VERT] < 32) {
      ifs_update_data.col[VERT] = 32;
      ifs_update_data.v[VERT] = (RAND () % 3) + 2;
      GOOM_LOG_DEBUG("ifs_update_data.v[VERT] = %d", ifs_update_data.v[VERT]);
    }

    ifs_update_data.col[ROUGE] += ifs_update_data.v[ROUGE];
    if (ifs_update_data.col[ROUGE] > 128) {
      ifs_update_data.col[ROUGE] = 128;
      ifs_update_data.v[ROUGE] = -(RAND () % 4) - 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ROUGE] = %d", ifs_update_data.v[ROUGE]);
    }
    if (ifs_update_data.col[ROUGE] < 0) {
      ifs_update_data.col[ROUGE] = 0;
      ifs_update_data.v[ROUGE] = (RAND () % 4) + 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ROUGE] = %d", ifs_update_data.v[ROUGE]);
    }

    ifs_update_data.col[ALPHA] += ifs_update_data.v[ALPHA];
    if (ifs_update_data.col[ALPHA] > 255) {
      ifs_update_data.col[ALPHA] = 255;
      ifs_update_data.v[ALPHA] = -(RAND () % 4) - 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ALPHA] = %d", ifs_update_data.v[ALPHA]);
    }
    if (ifs_update_data.col[ALPHA] < 0) {
      ifs_update_data.col[ALPHA] = 0;
      ifs_update_data.v[ALPHA] = (RAND () % 4) + 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ALPHA] = %d", ifs_update_data.v[ALPHA]);
    }

    if (((ifs_update_data.col[VERT] > 32) && (ifs_update_data.col[ROUGE] < ifs_update_data.col[VERT] + 40)
        && (ifs_update_data.col[VERT] < ifs_update_data.col[ROUGE] + 20) && (ifs_update_data.col[BLEU] < 64)
        && (RAND () % 20 == 0)) && (ifs_update_data.justChanged < 0)) {
      ifs_update_data.mode = RAND () % 3 ? MOD_FEU : MOD_MER;
      GOOM_LOG_DEBUG("ifs_update_data.mode = %d", ifs_update_data.mode);
      ifs_update_data.justChanged = 250;
    }
  } else if (ifs_update_data.mode == MOD_FEU) {
    ifs_update_data.col[BLEU] += ifs_update_data.v[BLEU];
    if (ifs_update_data.col[BLEU] > 64) {
      ifs_update_data.col[BLEU] = 64;
      ifs_update_data.v[BLEU] = -(RAND () % 4) - 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[BLEU] = %d", ifs_update_data.v[BLEU]);
    }
    if (ifs_update_data.col[BLEU] < 0) {
      ifs_update_data.col[BLEU] = 0;
      ifs_update_data.v[BLEU] = (RAND () % 4) + 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[BLEU] = %d", ifs_update_data.v[BLEU]);
    }

    ifs_update_data.col[VERT] += ifs_update_data.v[VERT];
    if (ifs_update_data.col[VERT] > 200) {
      ifs_update_data.col[VERT] = 200;
      ifs_update_data.v[VERT] = -(RAND () % 3) - 2;
      GOOM_LOG_DEBUG("ifs_update_data.v[VERT] = %d", ifs_update_data.v[VERT]);
    }
    if (ifs_update_data.col[VERT] > ifs_update_data.col[ROUGE] + 20) {
      ifs_update_data.col[VERT] = ifs_update_data.col[ROUGE] + 20;
      ifs_update_data.v[VERT] = -(RAND () % 3) - 2;
      ifs_update_data.v[ROUGE] = (RAND () % 4) + 1;
      ifs_update_data.v[BLEU] = (RAND () % 4) + 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[VERT] = %d", ifs_update_data.v[VERT]);
    }
    if (ifs_update_data.col[VERT] < 0) {
      ifs_update_data.col[VERT] = 0;
      ifs_update_data.v[VERT] = (RAND () % 3) + 2;
      GOOM_LOG_DEBUG("ifs_update_data.v[VERT] = %d", ifs_update_data.v[VERT]);
    }

    ifs_update_data.col[ROUGE] += ifs_update_data.v[ROUGE];
    if (ifs_update_data.col[ROUGE] > 255) {
      ifs_update_data.col[ROUGE] = 255;
      ifs_update_data.v[ROUGE] = -(RAND () % 4) - 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ROUGE] = %d", ifs_update_data.v[ROUGE]);
    }
    if (ifs_update_data.col[ROUGE] > ifs_update_data.col[VERT] + 40) {
      ifs_update_data.col[ROUGE] = ifs_update_data.col[VERT] + 40;
      ifs_update_data.v[ROUGE] = -(RAND () % 4) - 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ROUGE] = %d", ifs_update_data.v[ROUGE]);
    }
    if (ifs_update_data.col[ROUGE] < 0) {
      ifs_update_data.col[ROUGE] = 0;
      ifs_update_data.v[ROUGE] = (RAND () % 4) + 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ROUGE] = %d", ifs_update_data.v[ROUGE]);
    }

    ifs_update_data.col[ALPHA] += ifs_update_data.v[ALPHA];
    if (ifs_update_data.col[ALPHA] > 0) {
      ifs_update_data.col[ALPHA] = 0;
      ifs_update_data.v[ALPHA] = -(RAND () % 4) - 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ALPHA] = %d", ifs_update_data.v[ALPHA]);
    }
    if (ifs_update_data.col[ALPHA] < 0) {
      ifs_update_data.col[ALPHA] = 0;
      ifs_update_data.v[ALPHA] = (RAND () % 4) + 1;
      GOOM_LOG_DEBUG("ifs_update_data.v[ALPHA] = %d", ifs_update_data.v[ALPHA]);
    }

    if (((ifs_update_data.col[ROUGE] < 64) && (ifs_update_data.col[VERT] > 32)
        && (ifs_update_data.col[VERT] < ifs_update_data.col[BLEU]) && (ifs_update_data.col[BLEU] > 32)
        && (RAND () % 20 == 0)) && (ifs_update_data.justChanged < 0)) {
      ifs_update_data.mode = RAND () % 2 ? MOD_MER : MOD_MERVER;
      GOOM_LOG_DEBUG("ifs_update_data.mode = %d", ifs_update_data.mode);
      ifs_update_data.justChanged = 250;
    }
  }

  ifs_update_data.couleur = (ifs_update_data.col[ALPHA] << (ALPHA * 8)) | (ifs_update_data.col[BLEU] << (BLEU * 8))
      | (ifs_update_data.col[VERT] << (VERT * 8)) | (ifs_update_data.col[ROUGE] << (ROUGE * 8));

  GOOM_LOG_DEBUG("ifs_update_data.col[ALPHA] = %d", ifs_update_data.col[ALPHA]);
  GOOM_LOG_DEBUG("ifs_update_data.col[BLEU] = %d", ifs_update_data.col[BLEU]);
  GOOM_LOG_DEBUG("ifs_update_data.col[VERT] = %d", ifs_update_data.col[VERT]);
  GOOM_LOG_DEBUG("ifs_update_data.col[ROUGE] = %d", ifs_update_data.col[ROUGE]);

  GOOM_LOG_DEBUG("ifs_update_data.v[ALPHA] = %d", ifs_update_data.v[ALPHA]);
  GOOM_LOG_DEBUG("ifs_update_data.v[BLEU] = %d", ifs_update_data.v[BLEU]);
  GOOM_LOG_DEBUG("ifs_update_data.v[VERT] = %d", ifs_update_data.v[VERT]);
  GOOM_LOG_DEBUG("ifs_update_data.v[ROUGE] = %d", ifs_update_data.v[ROUGE]);

  GOOM_LOG_DEBUG("ifs_update_data.mode = %d", ifs_update_data.mode);
}

/** VISUAL_FX WRAPPER FOR IFS */

static void ifs_vfx_apply(VisualFX *_this, Pixel *src, Pixel *dest, PluginInfo *goomInfo)
{
  IfsData *data = (IfsData*) _this->fx_data;
  if (!data->initalized) {
    data->initalized = 1;
    init_ifs(goomInfo, data);
  }
  if (!BVAL(data->enabled_bp)) {
    return;
  }
  ifs_update(goomInfo, dest, src, goomInfo->update.ifs_incr, data);

  /*TODO: trouver meilleur soluce pour increment (mettre le code de gestion de l'ifs dans ce fichier: ifs_vfx_apply) */
}

static const char *vfxname = "Ifs";

static void ifs_vfx_save(VisualFX *_this, const PluginInfo *info, const char *file)
{
  FILE *f = fopen(file, "w");

  save_int_setting(f, vfxname, "ifs_update_data.justChanged", ifs_update_data.justChanged);
  save_int_setting(f, vfxname, "ifs_update_data.couleur", ifs_update_data.couleur);
  save_int_setting(f, vfxname, "ifs_update_data.v_0", ifs_update_data.v[0]);
  save_int_setting(f, vfxname, "ifs_update_data.v_1", ifs_update_data.v[1]);
  save_int_setting(f, vfxname, "ifs_update_data.v_2", ifs_update_data.v[2]);
  save_int_setting(f, vfxname, "ifs_update_data.v_3", ifs_update_data.v[3]);
  save_int_setting(f, vfxname, "ifs_update_data.col_0", ifs_update_data.col[0]);
  save_int_setting(f, vfxname, "ifs_update_data.col_1", ifs_update_data.col[1]);
  save_int_setting(f, vfxname, "ifs_update_data.col_2", ifs_update_data.col[2]);
  save_int_setting(f, vfxname, "ifs_update_data.col_3", ifs_update_data.col[3]);
  save_int_setting(f, vfxname, "ifs_update_data.mode", ifs_update_data.mode);
  save_int_setting(f, vfxname, "ifs_update_data.cycle", ifs_update_data.cycle);

  IfsData *data = (IfsData*) _this->fx_data;
  save_int_setting(f, vfxname, "data.Cur_Pt", data->Cur_Pt);
  save_int_setting(f, vfxname, "data.initalized", data->initalized);

  FRACTAL *Fractal = data->Root;
  save_int_setting(f, vfxname, "Fractal.Nb_Simi", Fractal->Nb_Simi);
  save_int_setting(f, vfxname, "Fractal.Depth", Fractal->Depth);
  save_int_setting(f, vfxname, "Fractal.Col", Fractal->Col);
  save_int_setting(f, vfxname, "Fractal.Count", Fractal->Count);
  save_int_setting(f, vfxname, "Fractal.Speed", Fractal->Speed);
  save_int_setting(f, vfxname, "Fractal.Width", Fractal->Width);
  save_int_setting(f, vfxname, "Fractal.Height", Fractal->Height);
  save_int_setting(f, vfxname, "Fractal.Lx", Fractal->Lx);
  save_int_setting(f, vfxname, "Fractal.Ly", Fractal->Ly);
  save_float_setting(f, vfxname, "Fractal.r_mean", Fractal->r_mean);
  save_float_setting(f, vfxname, "Fractal.dr_mean", Fractal->dr_mean);
  save_float_setting(f, vfxname, "Fractal.dr2_mean", Fractal->dr2_mean);
  save_int_setting(f, vfxname, "Fractal.Cur_Pt", Fractal->Cur_Pt);
  save_int_setting(f, vfxname, "Fractal.Max_Pt", Fractal->Max_Pt);

  for (int i = 0; i < Fractal->Nb_Simi; i++) {
    const SIMI *simi = &(Fractal->Components[i]);
    save_indexed_float_setting(f, vfxname, "simi.c_x", i, simi->c_x);
    save_indexed_float_setting(f, vfxname, "simi.c_y", i, simi->c_y);
    save_indexed_float_setting(f, vfxname, "simi.r", i, simi->r);
    save_indexed_float_setting(f, vfxname, "simi.r2", i, simi->r2);
    save_indexed_float_setting(f, vfxname, "simi.A", i, simi->A);
    save_indexed_float_setting(f, vfxname, "simi.A2", i, simi->A2);
    save_indexed_int_setting(f, vfxname, "simi.Ct", i, simi->Ct);
    save_indexed_int_setting(f, vfxname, "simi.St", i, simi->St);
    save_indexed_int_setting(f, vfxname, "simi.Ct2", i, simi->Ct2);
    save_indexed_int_setting(f, vfxname, "simi.St2", i, simi->St2);
    save_indexed_int_setting(f, vfxname, "simi.Cx", i, simi->Cx);
    save_indexed_int_setting(f, vfxname, "simi.Cy", i, simi->Cy);
    save_indexed_int_setting(f, vfxname, "simi.R", i, simi->R);
    save_indexed_int_setting(f, vfxname, "simi.R2", i, simi->R2);
  }

  fclose(f);
}

static void ifs_vfx_restore(VisualFX *_this, PluginInfo *info, const char *file)
{
  FILE *f = fopen(file, "r");
  if (f == NULL) {
    exit(EXIT_FAILURE);
  }

  ifs_update_data.justChanged = get_int_setting(f, vfxname, "ifs_update_data.justChanged");
  ifs_update_data.couleur = get_int_setting(f, vfxname, "ifs_update_data.couleur");
  ifs_update_data.v[0] = get_int_setting(f, vfxname, "ifs_update_data.v_0");
  ifs_update_data.v[1] = get_int_setting(f, vfxname, "ifs_update_data.v_1");
  ifs_update_data.v[2] = get_int_setting(f, vfxname, "ifs_update_data.v_2");
  ifs_update_data.v[3] = get_int_setting(f, vfxname, "ifs_update_data.v_3");
  ifs_update_data.col[0] = get_int_setting(f, vfxname, "ifs_update_data.col_0");
  ifs_update_data.col[1] = get_int_setting(f, vfxname, "ifs_update_data.col_1");
  ifs_update_data.col[2] = get_int_setting(f, vfxname, "ifs_update_data.col_2");
  ifs_update_data.col[3] = get_int_setting(f, vfxname, "ifs_update_data.col_3");
  ifs_update_data.mode = get_int_setting(f, vfxname, "ifs_update_data.mode");
  ifs_update_data.cycle = get_int_setting(f, vfxname, "ifs_update_data.cycle");

  IfsData *data = (IfsData*) _this->fx_data;
  data->Cur_Pt = get_int_setting(f, vfxname, "data.Cur_Pt");
  data->initalized = get_int_setting(f, vfxname, "data.initalized");
  data->initalized = 1;

  FRACTAL *Fractal = data->Root;
  Fractal->Nb_Simi = get_int_setting(f, vfxname, "Fractal.Nb_Simi");
  Fractal->Depth = get_int_setting(f, vfxname, "Fractal.Depth");
  Fractal->Col = get_int_setting(f, vfxname, "Fractal.Col");
  Fractal->Count = get_int_setting(f, vfxname, "Fractal.Count");
  Fractal->Speed = get_int_setting(f, vfxname, "Fractal.Speed");
  Fractal->Width = get_int_setting(f, vfxname, "Fractal.Width");
  Fractal->Height = get_int_setting(f, vfxname, "Fractal.Height");
  Fractal->Lx = get_int_setting(f, vfxname, "Fractal.Lx");
  Fractal->Ly = get_int_setting(f, vfxname, "Fractal.Ly");
  Fractal->r_mean = get_float_setting(f, vfxname, "Fractal.r_mean");
  Fractal->dr_mean = get_float_setting(f, vfxname, "Fractal.dr_mean");
  Fractal->dr2_mean = get_float_setting(f, vfxname, "Fractal.dr2_mean");
  Fractal->Cur_Pt = get_int_setting(f, vfxname, "Fractal.Cur_Pt");
  Fractal->Max_Pt = get_int_setting(f, vfxname, "Fractal.Max_Pt");

  for (int i = 0; i < Fractal->Nb_Simi; i++) {
    SIMI *simi = &(Fractal->Components[i]);
    simi->c_x = get_indexed_float_setting(f, vfxname, "simi.c_x", i);
    simi->c_y = get_indexed_float_setting(f, vfxname, "simi.c_y", i);
    simi->r = get_indexed_float_setting(f, vfxname, "simi.r", i);
    simi->r2 = get_indexed_float_setting(f, vfxname, "simi.r2", i);
    simi->A = get_indexed_float_setting(f, vfxname, "simi.A", i);
    simi->A2 = get_indexed_float_setting(f, vfxname, "simi.A2", i);
    simi->Ct = get_indexed_int_setting(f, vfxname, "simi.Ct", i);
    simi->St = get_indexed_int_setting(f, vfxname, "simi.St", i);
    simi->Ct2 = get_indexed_int_setting(f, vfxname, "simi.Ct2", i);
    simi->St2 = get_indexed_int_setting(f, vfxname, "simi.St2", i);
    simi->Cx = get_indexed_int_setting(f, vfxname, "simi.Cx", i);
    simi->Cy = get_indexed_int_setting(f, vfxname, "simi.Cy", i);
    simi->R = get_indexed_int_setting(f, vfxname, "simi.R", i);
    simi->R2 = get_indexed_int_setting(f, vfxname, "simi.R2", i);
  }

  fclose(f);

//    ifs_vfx_save(_this, info, "/tmp/vfx_save_after_restore.txt");
}

static void ifs_vfx_init(VisualFX *_this, PluginInfo *info)
{
  IfsData *data = (IfsData*) malloc(sizeof(IfsData));

  data->enabled_bp = secure_b_param("Enabled", 1);
  data->params = plugin_parameters("Ifs", 1);
  data->params.params[0] = &data->enabled_bp;

  data->Root = (FRACTAL*) NULL;
  data->initalized = 0;

  _this->fx_data = data;
  _this->params = &data->params;

  init_ifs(info, data);
  data->initalized = 1;
}

static void ifs_vfx_free(VisualFX *_this)
{
  IfsData *data = (IfsData*) _this->fx_data;
  release_ifs(data);
  free(data);
}

VisualFX ifs_visualfx_create(void)
{
  VisualFX vfx;
  vfx.init = ifs_vfx_init;
  vfx.free = ifs_vfx_free;
  vfx.apply = ifs_vfx_apply;
  vfx.save = ifs_vfx_save;
  vfx.restore = ifs_vfx_restore;
  return vfx;
}
