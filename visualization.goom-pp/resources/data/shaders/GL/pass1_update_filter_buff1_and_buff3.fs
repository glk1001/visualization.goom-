#version 430

#include "pass1_update_filter_buff1_and_buff3_consts.h"

uniform sampler2D tex_filterBuff2;
uniform sampler2D tex_mainImage;
uniform sampler2D tex_lowImage;

layout(binding = FILTER_BUFF1_IMAGE_UNIT, rgba16f) uniform image2D img_filterBuff1;
layout(binding = FILTER_BUFF3_IMAGE_UNIT, rgba16f) uniform image2D img_filterBuff3;

layout(binding = FILTER_SRCE_POS_IMAGE_UNIT1, rg32f) uniform image2D  img_filterSrcePosBuff1;
layout(binding = FILTER_SRCE_POS_IMAGE_UNIT2, rg32f) uniform image2D  img_filterSrcePosBuff2;
layout(binding = FILTER_DEST_POS_IMAGE_UNIT1, rg32f) uniform readonly image2D img_filterDestPosBuff1;
layout(binding = FILTER_DEST_POS_IMAGE_UNIT2, rg32f) uniform readonly image2D img_filterDestPosBuff2;

in vec3 position;
in vec2 texCoord;

uniform float u_lerpFactor;
uniform float u_buff2Buff3Mix = 0.1;
uniform bool u_resetSrceFilterPosBuffers;
uniform float u_pos1Pos2MixFreq;
uniform uint u_time;

// For base multiplier, too close to 1, gives washed
// out look, too far away and things get too dark.
uniform float u_baseColorMultiplier;
uniform float u_mainColorMultiplier = 1.0;
uniform float u_lowColorMultiplier  = 0.7;

/**
vec3 blend(vec3 base, vec3 blend, float opacity)
{
//  return blendAdd(base, blend, opacity);
  return blendMode(1, base, blend, opacity);
//  return blendLighten(base, blend, opacity);
}
**/

vec4 GetPosMappedFilterBuff2Value(vec2 uv, ivec2 deviceXY);
float GetBaseColorMultiplier(vec3 color);

void main()
{
  ivec2 deviceXY = ivec2(gl_FragCoord.xy);

  vec4 filterBuff2Val = GetPosMappedFilterBuff2Value(texCoord, deviceXY);
  vec4 filterBuff3Val = imageLoad(img_filterBuff3, deviceXY);

  vec4 colorMain = texture(tex_mainImage, texCoord);
  vec4 colorLow  = texture(tex_lowImage, texCoord);

  //  vec4 filtBuff2ColorMain = vec4(blend(filtBuff2Val.rgb, 90*colorMain.rgb, 0.5*colorMain.a), 1.0);
  //  vec4 filtBuff2ColorMain = vec4((1-colorMain.a)*filtBuff2Val.rgb + colorMain.a*colorMain.rgb, 1.0);
  //  float alpha = 1 - 0.5 * colorMain.a;
  //  vec4 filtBuff2ColorMain = (1-alpha)*filtBuff2Val + alpha*50*colorMain;
  //  vec4 filtBuff2ColorMain = vec4(blend(100*colorMain.rgb, 0.5*filtBuff2Val.rgb, colorMain.a), 1.0);
  //  vec4 filtBuff2ColorMain = vec4(filtBuff2Val.rgb, 1.0);

  filterBuff2Val.rgb = mix(filterBuff2Val.rgb, filterBuff3Val.rgb, u_buff2Buff3Mix);

  filterBuff2Val.rgb *= GetBaseColorMultiplier(filterBuff2Val.rgb);

  vec3 filterBuff2ColorMain = filterBuff2Val.rgb + (u_mainColorMultiplier * colorMain.rgb);
  vec3 filterBuff2ColorLow  = filterBuff2Val.rgb + (u_lowColorMultiplier * colorLow.rgb);

  imageStore(img_filterBuff1, deviceXY, vec4(filterBuff2ColorLow, colorLow.a));
  imageStore(img_filterBuff3, deviceXY, vec4(filterBuff2ColorMain, colorLow.a));

  discard;
}


const float BLACK_CUTOFF = 0.03;

bool NotCloseToBlack(vec3 color)
{
  return (color.r > BLACK_CUTOFF) || (color.r != color.g) || (color.r != color.b);
}

// Try to get purer blacks by using a lower baseColorMultiplier for small grey values.
float GetBaseColorMultiplier(vec3 color)
{
  const float LOW_BASE_COLOR_MULTIPLIER = 0.25;

  return NotCloseToBlack(color) ? u_baseColorMultiplier
                                : mix(LOW_BASE_COLOR_MULTIPLIER,
                                      u_baseColorMultiplier,
                                      pow(color.r / BLACK_CUTOFF, 3.0));
}


struct LerpedNormalizedPositions
{
  vec2 pos1;
  vec2 pos2;
};
struct TexelPositions
{
  vec2 uv1;
  vec2 uv2;
};
LerpedNormalizedPositions GetLerpedNormalizedPositions(ivec2 deviceXY);
TexelPositions GetTexelPositions(LerpedNormalizedPositions lerpedNormalizedPositions);
void ResetImageSrceFilterBuffPositions(ivec2 deviceXY,
                                       LerpedNormalizedPositions lerpedNormalizedPositions);
float GetColorBlendTMix(vec2 uv, TexelPositions texelPositions);
vec4 GetColorFromBlendedColors(TexelPositions filterBuff2TexelPositions, float tMix);

vec4 GetPosMappedFilterBuff2Value(vec2 uv, ivec2 deviceXY)
{
  deviceXY = ivec2(deviceXY.x, HEIGHT - 1 - deviceXY.y);

  LerpedNormalizedPositions lerpedNormalizedPositions = GetLerpedNormalizedPositions(deviceXY);

  if (u_resetSrceFilterPosBuffers)
  {
    ResetImageSrceFilterBuffPositions(deviceXY, lerpedNormalizedPositions);
  }

  const float deltaAmp = 0.01;
  const float deltaFreq = 0.05;
  const vec2 delta = vec2(cos(deltaFreq * u_time), sin(deltaFreq * u_time));
  lerpedNormalizedPositions.pos1 += deltaAmp * delta;
  lerpedNormalizedPositions.pos2 -= deltaAmp * delta;

  TexelPositions filterBuff2TexelPositions = GetTexelPositions(lerpedNormalizedPositions);

  return GetColorFromBlendedColors(filterBuff2TexelPositions, 
                                   GetColorBlendTMix(uv, filterBuff2TexelPositions));
}

struct SrceAndDestNormalizedPositions
{
  vec2 srcePos1;
  vec2 destPos1;
  vec2 srcePos2;
  vec2 destPos2;
};
struct FilterBuffColors
{
  vec4 color1;
  vec4 color2;
};

SrceAndDestNormalizedPositions GetSrceAndDestNormalizedPositions(ivec2 deviceXY)
{
  return SrceAndDestNormalizedPositions(
             imageLoad(img_filterSrcePosBuff1, deviceXY).xy,
             imageLoad(img_filterDestPosBuff1, deviceXY).xy,
             imageLoad(img_filterSrcePosBuff2, deviceXY).xy,
             imageLoad(img_filterDestPosBuff2, deviceXY).xy
  );
}

LerpedNormalizedPositions GetLerpedNormalizedPositions(ivec2 deviceXY)
{
  SrceAndDestNormalizedPositions normalizedPositions = GetSrceAndDestNormalizedPositions(deviceXY);

  return LerpedNormalizedPositions(
             mix(normalizedPositions.srcePos1, normalizedPositions.destPos1, u_lerpFactor),
             mix(normalizedPositions.srcePos2, normalizedPositions.destPos2, u_lerpFactor)
  );
}

void ResetImageSrceFilterBuffPositions(ivec2 deviceXY, LerpedNormalizedPositions lerpedNormalizedPositions)
{
  // Reset the filter srce pos buffers to the current lerped state, ready for
  // a new filter dest pos buffer.
  imageStore(img_filterSrcePosBuff1, deviceXY, vec4(lerpedNormalizedPositions.pos1, 0, 0));
  imageStore(img_filterSrcePosBuff2, deviceXY, vec4(lerpedNormalizedPositions.pos2, 0, 0));
}

vec2 GetTexelPos(vec2 filterPos)
{
  float x = (filterPos.x - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;
  float y = (filterPos.y - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;

  return vec2(x, 1 - (ASPECT_RATIO * y));
}

TexelPositions GetTexelPositions(LerpedNormalizedPositions lerpedNormalizedPositions)
{
  return TexelPositions(
             GetTexelPos(lerpedNormalizedPositions.pos1),
             GetTexelPos(lerpedNormalizedPositions.pos2)
  );
}

FilterBuffColors GetFilterBuff2Colors(TexelPositions texelPositions)
{
  return FilterBuffColors(
             texture(tex_filterBuff2, texelPositions.uv1),
             texture(tex_filterBuff2, texelPositions.uv2)
  );
}

float GetSinTMix()
{
  // const float r = 0.25;
  // const vec2 circleCentre1 = 0.5 + vec2(r*cos(u_pos1Pos2MixFreq * u_time), r*sin(u_pos1Pos2MixFreq * u_time));
  // const vec2 circleCentre2 = 0.4 + vec2(r*cos(1+u_pos1Pos2MixFreq * u_time), r*sin(1+u_pos1Pos2MixFreq * u_time));
  // float dist1 = distance(uv, circleCentre1);
  // float dist2 = distance(uv, circleCentre2) - 0.2;
  // vec3 t = vec3(step(0.0, sdf_smin(dist1, dist2)));

  // vec3 t = vec3(step(0.5, uv.x));
  // float posDist = distance(filtBuff2Pos1, filtBuff2Pos2)/5.5;
  // float posDist = distance(vec2(0), filtBuff2Pos2)/5.5;

  // return 0.5;
  // return step(100, u_time % 200);
  return 0.5 * (1.0 + sin(u_pos1Pos2MixFreq * u_time));
}

float GetUVDistAdjustedTMix(vec2 uv, TexelPositions texelPositions, float tMix)
{
  const float MAX_UV = sqrt(2.0);
  //vec2 posUv = mix(texelPositions.uv1, texelPositions.uv2, vec2(tMix.x));
  //float distUv = min(distance(uv, posUv), MAX_UV) / MAX_UV;
  float uvDist = min(distance(uv, texelPositions.uv2), MAX_UV) / MAX_UV;

  return tMix * (1.0 - uvDist);
}

float GetColorBlendTMix(vec2 uv, TexelPositions texelPositions)
{
  const float SIN_T_MIX = GetSinTMix();
  // return SIN_T_MIX;
  return GetUVDistAdjustedTMix(uv, texelPositions, SIN_T_MIX);
}

vec3 GetMixedColor(vec3 tMix, FilterBuffColors filterBuffColors)
{
  // return mix(filterBuffColors.color1.rgb, filterBuffColors.color1.rgb, tMix);
  // return mix(filterBuffColors.color2.rgb, filterBuffColors.color2.rgb, tMix);

  return mix(filterBuffColors.color1.rgb, filterBuffColors.color2.rgb, tMix);
}

vec4 GetColorFromBlendedColors(TexelPositions filterBuff2TexelPositions, float tMix)
{
  FilterBuffColors filterBuff2Colors = GetFilterBuff2Colors(filterBuff2TexelPositions);
  float alpha = filterBuff2Colors.color1.a;

  return vec4(GetMixedColor(vec3(tMix), filterBuff2Colors), alpha);
}
