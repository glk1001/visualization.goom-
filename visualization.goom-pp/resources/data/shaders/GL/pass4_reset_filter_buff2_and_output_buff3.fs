#version 430

#include "pass4_reset_filter_buff2_and_output_buff3_consts.h"

#include "tone-maps.glsl"
#include "color-space-conversions.glsl"

layout(location = 0) out vec4 fragColor;

// @formatter:off
layout(binding=FILTER_BUFF1_IMAGE_UNIT, rgba16f) uniform readonly image2D img_filterBuff1;
layout(binding=FILTER_BUFF2_IMAGE_UNIT, rgba16f) uniform          image2D img_filterBuff2;
layout(binding=FILTER_BUFF3_IMAGE_UNIT, rgba16f) uniform readonly image2D img_filterBuff3;
layout(binding=LUM_AVG_IMAGE_UNIT,         r16f) uniform readonly image2D img_lumAvg;
// @formatter:on

uniform float u_brightness;
uniform float u_brightnessAdjust;
uniform float u_hueShift;
uniform float u_chromaFactor;

vec3 GetHueShift(vec3 color);
vec3 GetChromaticIncrease(vec3 color);

void main()
{
  ivec2 xy = ivec2(gl_FragCoord.xy);

  // Get the hdr color to work with.
  vec4 filtBuff3Val = imageLoad(img_filterBuff3, xy);
  vec4 hdrColor = filtBuff3Val;

  // Copy filter buff1 to filter buff2 ready for the next frame.
  vec4 filtBuff1Val = imageLoad(img_filterBuff1, xy);
  imageStore(img_filterBuff2, xy, filtBuff1Val);

  // Apply the hue shift.
  hdrColor.rgb = GetHueShift(hdrColor.rgb);

  // Apply the chromatic increase.
  hdrColor.rgb = GetChromaticIncrease(hdrColor.rgb);
  
  // Finish with the tone mapping.
  float averageLuminance = imageLoad(img_lumAvg, ivec2(0, 0)).x;
  vec3 toneMappedColor = GetToneMappedColor(hdrColor.rgb,
                                            averageLuminance,
                                            u_brightnessAdjust * u_brightness);
  //  vec3 toneMappedColor = GetToneMappedColor(hdrColor, 1.0, u_brightness);
  //  vec3 toneMappedColor = hdrColor.rgb;

  fragColor = vec4(toneMappedColor, 1.0F);
}

vec3 GetChromaticIncrease(vec3 color)
{
  // 'Chromatic Increase' - https://github.com/gurki/vivid
  vec3 lch = rgb_to_lch(color);
  lch.y = min(u_chromaFactor * lch.y, 140.0);
  return lch_to_rgb(lch);
}

vec3 GetHueShift(vec3 color)
{
  const vec3 k = vec3(0.57735, 0.57735, 0.57735);
  float cosAngle = cos(u_hueShift);
  float sinAngle = sin(u_hueShift);
  return vec3((color * cosAngle) +
              (sinAngle * cross(k, color)) +
              ((1.0 - cosAngle) * dot(k, color) * k)
             );
}
