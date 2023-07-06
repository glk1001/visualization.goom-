#version 430

#include "pass4_reset_filter_buff2_and_output_buff3_consts.h"

#include "tone-maps.glsl"
#include "color-space-conversions.glsl"

layout(location = 0) out vec4 fragColor;

layout(binding=FILTER_BUFF1_IMAGE_UNIT, rgba16) uniform readonly image2D img_filterBuff1;
layout(binding=FILTER_BUFF2_IMAGE_UNIT, rgba16) uniform          image2D img_filterBuff2;
layout(binding=FILTER_BUFF3_IMAGE_UNIT, rgba16) uniform readonly image2D img_filterBuff3;
layout(binding=LUM_AVG_IMAGE_UNIT,        r16f) uniform readonly image2D img_lumAvg;

uniform float u_brightness;
uniform float u_chromaFactor;

vec3 GetChromaticIncrease(vec3 color);

void main()
{
  ivec2 xy = ivec2(gl_FragCoord.xy);

  // Get the hdr color to work with.
  vec4 filtBuff3Val = imageLoad(img_filterBuff3, xy);
  vec3 hdrColor = filtBuff3Val.rgb;

  // Copy filter buff1 to filter buff2 ready for the next frame.
  vec4 filtBuff1Val = imageLoad(img_filterBuff1, xy);
  imageStore(img_filterBuff2, xy, filtBuff1Val);

  // Apply the chromatic increase.
  hdrColor = GetChromaticIncrease(hdrColor);

  // Finish with the tone mapping.
  float averageLuminance = imageLoad(img_lumAvg, ivec2(0, 0)).x;
  vec3 toneMappedColor = GetToneMappedColor(hdrColor, averageLuminance, u_brightness);
  //  vec3 toneMappedColor = GetToneMappedColor(hdrColor, 1.0, u_brightness);
  //  vec3 toneMappedColor = hdrColor;

  fragColor = vec4(toneMappedColor, 1.0F);
}

vec3 GetChromaticIncrease(vec3 color)
{
  // 'Chromatic Increase' - https://github.com/gurki/vivid
  vec3 lch = rgb_to_lch(color);
  lch.y = min(u_chromaFactor * lch.y, 140.0);
  return lch_to_rgb(lch);
}
