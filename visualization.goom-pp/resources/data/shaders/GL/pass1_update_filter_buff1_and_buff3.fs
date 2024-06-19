#version 430

#include "pass1_update_filter_buff1_and_buff3_consts.h"

uniform sampler2D tex_filterBuff2;    // Low colors from last frame
uniform sampler2D tex_mainColorImage; // Main colors for this frame
uniform sampler2D tex_lowColorImage;  // Low colors for this frame

// At end of this pass, 'filterBuff1' contains the newly mapped colors plus low colors.
layout(binding = FILTER_BUFF1_IMAGE_UNIT, rgba16f) uniform image2D img_filterBuff1;
// At end of this pass, 'filterBuff3' contains the newly mapped colors plus main colors.
layout(binding = FILTER_BUFF3_IMAGE_UNIT, rgba16f) uniform image2D img_filterBuff3;

// All the buffers used for position mapping.
layout(binding = FILTER_SRCE_POS_IMAGE_UNIT1, rg32f) uniform image2D  img_filterSrcePosBuff1;
layout(binding = FILTER_SRCE_POS_IMAGE_UNIT2, rg32f) uniform image2D  img_filterSrcePosBuff2;
layout(binding = FILTER_DEST_POS_IMAGE_UNIT1, rg32f) uniform readonly image2D img_filterDestPosBuff1;
layout(binding = FILTER_DEST_POS_IMAGE_UNIT2, rg32f) uniform readonly image2D img_filterDestPosBuff2;

in vec3 position;
in vec2 texCoord;

uniform float u_lerpFactor;  // For lerping between srce and dest buffers.
uniform float u_minBuff2Buff3Mix = 0.1F;
uniform float u_maxBuff2Buff3Mix = 0.9F;
uniform bool u_resetSrceFilterPosBuffers;
uniform float u_pos1Pos2MixFreq;
uniform uint u_time;

// For base multiplier, too close to 1, gives washed
// out look, too far away and things get too dark.
uniform float u_baseColorMultiplier;       // Used to factor this frames' buff2 color.
uniform float u_mainColorMultiplier = 1.0; // Used to factor this frames' main color.
uniform float u_lowColorMultiplier  = 0.7; // Used to factor this frames' low color.

vec4 GetPosMappedFilterBuff2ColorValue(vec2 uv, ivec2 deviceXY);
float GetBaseColorMultiplier(vec3 color);

void main()
{
  ivec2 deviceXY = ivec2(gl_FragCoord.xy);

  vec4 filterBuff2Val = GetPosMappedFilterBuff2ColorValue(texCoord, deviceXY);
  vec4 filterBuff3Val = imageLoad(img_filterBuff3, deviceXY);

  // Mix in some of the previous frames' color from the current deviceXY.
  const float extraBuff2Buff3MixFreq = 0.05F;
  const float extraBuff2Buff3Mix = mix(0.0F,
                                       u_maxBuff2Buff3Mix - u_minBuff2Buff3Mix,
                                       0.5F * (1.0F + cos(extraBuff2Buff3MixFreq * u_time)));
  filterBuff2Val.rgb = mix(filterBuff2Val.rgb,
                           filterBuff3Val.rgb,
                           u_minBuff2Buff3Mix + extraBuff2Buff3Mix);

  // Boost this frames' buff2 color by the base color multiplier.
  filterBuff2Val.rgb *= GetBaseColorMultiplier(filterBuff2Val.rgb);

  // Get and store the low color added to this frames' buff2 color.
  vec4 colorLow            = texture(tex_lowColorImage, texCoord);
  float alpha              = colorLow.a; // Use low color alpha for main also.
  vec3 filterBuff2ColorLow = filterBuff2Val.rgb + (u_lowColorMultiplier * colorLow.rgb);
  imageStore(img_filterBuff1, deviceXY, vec4(filterBuff2ColorLow, alpha));

  // Get and store the main color added to this frames' buff2 color.
  vec4 colorMain            = texture(tex_mainColorImage, texCoord);
  vec3 filterBuff2ColorMain = filterBuff2Val.rgb + (u_mainColorMultiplier * colorMain.rgb);
  imageStore(img_filterBuff3, deviceXY, vec4(filterBuff2ColorMain, alpha));

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


struct TexelPositions
{
  vec2 uv1;
  vec2 uv2;
};
struct FilterBuffColors
{
  vec4 color1;
  vec4 color2;
};

TexelPositions GetPosMappedFilterBuff2TexelPositions(ivec2 deviceXY);
FilterBuffColors GetFilterBuff2Colors(TexelPositions texelPositions);
vec4 GetColorFromMixOfColor1AndColor2(FilterBuffColors filterBuff2Colors, float tMix);
float GetColor1Color2TMix(vec2 fromUV, TexelPositions toTexelPositions);

vec4 GetPosMappedFilterBuff2ColorValue(vec2 uv, ivec2 deviceXY)
{
  TexelPositions filterBuff2TexelPositions = GetPosMappedFilterBuff2TexelPositions(deviceXY);
  FilterBuffColors filterBuff2Colors       = GetFilterBuff2Colors(filterBuff2TexelPositions);
  return GetColorFromMixOfColor1AndColor2(
      filterBuff2Colors, GetColor1Color2TMix(uv, filterBuff2TexelPositions));
}


struct LerpedNormalizedPositions
{
  vec2 pos1;
  vec2 pos2;
};
LerpedNormalizedPositions GetLerpedNormalizedPositions(ivec2 deviceXY);
TexelPositions GetTexelPositions(LerpedNormalizedPositions lerpedNormalizedPositions);
void ResetImageSrceFilterBuffPositions(ivec2 deviceXY,
                                       LerpedNormalizedPositions lerpedNormalizedPositions);

TexelPositions GetPosMappedFilterBuff2TexelPositions(ivec2 deviceXY)
{
  deviceXY = ivec2(deviceXY.x, HEIGHT - 1 - deviceXY.y);

  LerpedNormalizedPositions lerpedNormalizedPositions = GetLerpedNormalizedPositions(deviceXY);

  if (u_resetSrceFilterPosBuffers)
  {
    ResetImageSrceFilterBuffPositions(deviceXY, lerpedNormalizedPositions);
  }

  const float deltaAmp  = 0.01;
  const float deltaFreq = 0.05;
  const vec2 delta      = vec2(cos(deltaFreq * u_time), sin(deltaFreq * u_time));
  lerpedNormalizedPositions.pos1 += deltaAmp * delta;
  lerpedNormalizedPositions.pos2 -= deltaAmp * delta;

  return GetTexelPositions(lerpedNormalizedPositions);
}

struct SrceAndDestNormalizedPositions
{
  vec2 srcePos1;
  vec2 destPos1;
  vec2 srcePos2;
  vec2 destPos2;
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

void ResetImageSrceFilterBuffPositions(ivec2 deviceXY,
                                       LerpedNormalizedPositions lerpedNormalizedPositions)
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

float GetUVDistAdjustedTMix(vec2 fromUV, TexelPositions toTexelPositions, float tMix)
{
  const float MAX_UV = sqrt(2.0);
  //vec2 posUv = mix(toTexelPositions.uv1, toTexelPositions.uv2, vec2(tMix.x));
  //float distUv = min(distance(fromUV, posUv), MAX_UV) / MAX_UV;
  float uvDist = min(distance(fromUV, toTexelPositions.uv2), MAX_UV) / MAX_UV;

  return tMix * (1.0 - uvDist);
}

float GetColor1Color2TMix(vec2 fromUV, TexelPositions toTexelPositions)
{
  const float SIN_T_MIX = GetSinTMix();
  // return SIN_T_MIX;
  return GetUVDistAdjustedTMix(fromUV, toTexelPositions, SIN_T_MIX);
}

vec4 GetColorFromMixOfColor1AndColor2(FilterBuffColors filterBuff2Colors, float tMix)
{
  vec3 mixedColor = mix(filterBuff2Colors.color1.rgb, filterBuff2Colors.color2.rgb, vec3(tMix));
  float alpha     = filterBuff2Colors.color1.a;

  return vec4(mixedColor, alpha);
}
