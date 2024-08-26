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
uniform float u_prevFrameTMix;
uniform bool u_resetSrceFilterPosBuffers;
uniform float u_pos1Pos2MixFreq;
uniform uint u_time;

// For base multiplier, too close to 1, gives washed
// out look, too far away and things get too dark.
uniform float u_baseColorMultiplier;        // Used to factor this frames' buff2 color.
uniform float u_mainColorMultiplier = 1.0F; // Used to factor this frames' main color.
uniform float u_lowColorMultiplier  = 0.7F; // Used to factor this frames' low color.

vec4 GetPosMappedFilterBuff2ColorValue(vec2 uv, ivec2 deviceXY);
float GetBaseColorMultiplier(vec3 color);
vec2 GetTexelPos(vec2 filterPos);

void main()
{
  ivec2 deviceXY = ivec2(gl_FragCoord.xy);

  vec4 filterBuff2Val = GetPosMappedFilterBuff2ColorValue(texCoord, deviceXY);
  vec4 filterBuff3Val = imageLoad(img_filterBuff3, deviceXY);

  // Mix in some of the previous frames' color from the current deviceXY.
  filterBuff2Val.rgb = mix(filterBuff2Val.rgb, filterBuff3Val.rgb, u_prevFrameTMix);

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


const float BLACK_CUTOFF = 0.03F;

bool NotCloseToBlack(vec3 color)
{
  return (color.r > BLACK_CUTOFF) || (color.r != color.g) || (color.r != color.b);
}

// Try to get purer blacks by using a lower baseColorMultiplier for small grey values.
float GetBaseColorMultiplier(vec3 color)
{
  const float LOW_BASE_COLOR_MULTIPLIER = 0.25F;

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
//   deviceXY = ivec2(deviceXY.x, HEIGHT - 1 - deviceXY.y);
//
//   const float xRatioDeviceToNormalizedCoord = FILTER_POS_COORD_WIDTH / float(WIDTH - 1);
//   const float yRatioDeviceToNormalizedCoord = FILTER_POS_COORD_WIDTH / float(WIDTH - 1);
//   const float X_MIN_COORD = FILTER_POS_MIN_COORD;
//   const float Y_MIN_COORD = FILTER_POS_MIN_COORD;
//
//   vec2 pos = vec2(X_MIN_COORD + (xRatioDeviceToNormalizedCoord * float(deviceXY.x)),
//                   Y_MIN_COORD + (yRatioDeviceToNormalizedCoord * float(deviceXY.y))
//              );
//
//   float x = (pos.x - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;
//   float y = (pos.y - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;
//   uv = vec2(x, 1 - (ASPECT_RATIO * y));
//
//   uv = GetTexelPos(pos);
//   //uv = vec2(deviceXY.x/float(WIDTH-1), deviceXY.y/float(HEIGHT-1));
//
// //   return texture(tex_filterBuff2, uv);
//   TexelPositions filterBuff2TexelPositions = TexelPositions(uv, uv);

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

vec2 GetVortexVelocity(vec2 position)
{
  vec2 p = position;

  float r = length(p);
  float theta = atan(p.y, p.x);
  vec2 v = vec2(p.y, -p.x) / r;
  float t = sqrt(10.0 * r) + theta + 0.02 * float(u_time);

  v *= sin(t);
  v *= 2,0 * length(v);
  v += 0.2 * p;

  return v;
}

vec2 GetReflectingPoolVelocity(vec2 position)
{
  vec2 p = position;

  vec2 v;

  v.x = sin(5.0*p.y + p.x);
  v.y = cos(5.0*p.x - p.y);

  return v;
}

vec2 GetAmulet(vec2 position)
{
  vec2 p = position;

  vec2 v;

  float sqDistFromZero = p.x * p.x + p.y * p.y;

  const float sinT = sin(0.01*u_time);
  const float cosT = cos(0.01*u_time);
  float x = p.x;
  p.x = p.x*cosT - p.y*sinT;
  p.y = p.y*cosT + x*sinT;

  const float Ax = 1.0;
  const float Ay = 1.0;
  const float baseX = 0.25;
  const float baseY = 0.25;
  v.x = baseX + (Ax * sqDistFromZero),
  v.y = baseY + (Ay * sqDistFromZero);

  return -p * v;
}

vec2 GetWave(vec2 position)
{
  vec2 p = position;

  vec2 v;

  float sqDistFromZero = p.x * p.x + p.y * p.y;
  const float reducerCoeff = 0.0;
  float reducer        = exp(-reducerCoeff * sqDistFromZero);

  const float freqFactor = 10.5;
  //float angle          = atan(p.y, p.x);
  float angle = pow(sqDistFromZero, 1.1);

  float sinAngle = sin(freqFactor * angle);
  float cosAngle = cos(freqFactor * angle);

  const float Ax = 0.2;
  const float Ay = 0.2;
  const float baseX = 0.1;
  const float baseY = 0.1;

  v.x = baseX + reducer * Ax * cosAngle,
  v.y = baseY + reducer * Ay * sinAngle;

  return -p * v;
}

vec2 GetBeautifulFieldVelocity(vec2 position)
{
  vec2 p = position;// + vec2(0.5, 1.0);

  float frame = 150.0 * sin(0.01 * float(u_time));

  const float PI = 3.14;
  const float dt = 0.01;

  float t = frame * dt;
  const float w = 2.0 * (PI / 5.0);
  const float A = 5.0;

  float d = sqrt((p.x * p.x) + (p.y * p.y));

  vec2 v = vec2(A * cos((w * t) / d), A * sin((w * t) / d));

  return v;
}

vec2 GetGPUFilteredPosition(ivec2 deviceXY)
{
  const float xRatioDeviceToNormalizedCoord = FILTER_POS_COORD_WIDTH / float(WIDTH - 1);
  const float yRatioDeviceToNormalizedCoord = FILTER_POS_COORD_WIDTH / float(WIDTH - 1);
  //const float yRatioDeviceToNormalizedCoord = FILTER_POS_COORD_WIDTH / float(HEIGHT - 1);
  const float X_MIN_COORD = FILTER_POS_MIN_COORD;
  //const float Y_MIN_COORD = (1.0F / ASPECT_RATIO) * FILTER_POS_MIN_COORD;
  const float Y_MIN_COORD = FILTER_POS_MIN_COORD;

  vec2 pos = vec2(X_MIN_COORD + (xRatioDeviceToNormalizedCoord * float(deviceXY.x)),
                  Y_MIN_COORD + (yRatioDeviceToNormalizedCoord * float(deviceXY.y))
             );

  //pos.y = (pos.y - Y_MIN_COORD)*ASPECT_RATIO + Y_MIN_COORD;

  //vec2 v = GetVortexVelocity(pos);
  vec2 v = GetWave(pos);
  //vec2 v = GetAmulet(pos);
  //vec2 v = GetReflectingPoolVelocity(pos);
  //vec2 v = GetBeautifulFieldVelocity(pos);

  pos = pos + v;
  //pos.y -= 0.5;
  //pos.y = (pos.y - Y_MIN_COORD)/ASPECT_RATIO + Y_MIN_COORD;
  return pos;
}

TexelPositions GetPosMappedFilterBuff2TexelPositions(ivec2 deviceXY)
{
  deviceXY = ivec2(deviceXY.x, HEIGHT - 1 - deviceXY.y);

  LerpedNormalizedPositions lerpedNormalizedPositions = GetLerpedNormalizedPositions(deviceXY);

  if (u_resetSrceFilterPosBuffers)
  {
    ResetImageSrceFilterBuffPositions(deviceXY, lerpedNormalizedPositions);
  }

  const float deltaAmp  = 0.01F;
  const float deltaFreq = 0.05F;
  const vec2 delta      = vec2(cos(deltaFreq * u_time), sin(deltaFreq * u_time));
  lerpedNormalizedPositions.pos1 += deltaAmp * delta;
  lerpedNormalizedPositions.pos2 -= deltaAmp * delta;

  const float tGPU = 1.0F;
  vec2 GPUPos = GetGPUFilteredPosition(deviceXY);

  lerpedNormalizedPositions.pos1 = mix(lerpedNormalizedPositions.pos1, GPUPos, tGPU);
  lerpedNormalizedPositions.pos2 = mix(lerpedNormalizedPositions.pos2, GPUPos, tGPU);
//   lerpedNormalizedPositions.pos1 = GPUPos;
//   lerpedNormalizedPositions.pos2 = GPUPos;

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
  // const float r = 0.25F;
  // const vec2 circleCentre1 = 0.5F + vec2(r*cos(u_pos1Pos2MixFreq * u_time), r*sin(u_pos1Pos2MixFreq * u_time));
  // const vec2 circleCentre2 = 0.4F + vec2(r*cos(1+u_pos1Pos2MixFreq * u_time), r*sin(1+u_pos1Pos2MixFreq * u_time));
  // float dist1 = distance(uv, circleCentre1);
  // float dist2 = distance(uv, circleCentre2) - 0.2F;
  // vec3 t = vec3(step(0.0F, sdf_smin(dist1, dist2)));

  // vec3 t = vec3(step(0.5F, uv.x));
  // float posDist = distance(filtBuff2Pos1, filtBuff2Pos2)/5.5;
  // float posDist = distance(vec2(0), filtBuff2Pos2)/5.5F;

  // return 0.5F;
  // return step(100, u_time % 200);
  return 0.5F * (1.0F + sin(u_pos1Pos2MixFreq * u_time));
}

float GetUVDistAdjustedTMix(vec2 fromUV, TexelPositions toTexelPositions, float tMix)
{
  const float MAX_UV = sqrt(2.0F);
  //vec2 posUv = mix(toTexelPositions.uv1, toTexelPositions.uv2, vec2(tMix.x));
  //float distUv = min(distance(fromUV, posUv), MAX_UV) / MAX_UV;
  float uvDist = min(distance(fromUV, toTexelPositions.uv2), MAX_UV) / MAX_UV;

  return tMix * (1.0F - uvDist);
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
