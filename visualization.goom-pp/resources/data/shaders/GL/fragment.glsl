#version 330 core

#include "tone-maps.glsl"
#include "color-space-conversions.glsl"

vec3 GetHueShift(vec3 color, float hue)
{
  const vec3 k = vec3(0.57735, 0.57735, 0.57735);
  float cosAngle = cos(hue);
  float sinAngle = sin(hue);
  return vec3((color * cosAngle) +
              (sinAngle * cross(k, color)) +
              ((1.0 - cosAngle) * dot(k, color) * k));
}

vec3 GetChromaticIncrease(vec3 color)
{
  // 'Chromatic Increase' - https://github.com/gurki/vivid
  vec3 lch = rgb_to_lch(color);
  lch.y = min(lch.y * 2.0F, 140.0);
  return lch_to_rgb(lch);
}

// ***********************
// Input/Output variables

out vec4 fragColor;

uniform sampler2D texBuffer;
uniform float u_texExposure;
uniform float u_texBrightness;
uniform float u_texContrast;
uniform float u_texContrastMinChan;
uniform float u_texHueShiftLerpT;
uniform float u_texSrceHueShift;
uniform float u_texDestHueShift;
uniform int u_time;
in vec2 texCoords;


void main()
{
  vec2 uvTex = texCoords;
  vec3 hdrColor = texture(texBuffer, uvTex).rgb;


/**
  if (hdrColor.r < 125.0/65535.0 && hdrColor.g < 125.0/65535.0 && hdrColor.b < 125.0/65535.0)
    hdrColor.rgb = vec3(0.5, 0.5, 0.5);
  else if (hdrColor.r < 255.0/65535.0 && hdrColor.g < 255.0/65535.0 && hdrColor.b < 255.0/65535.0)
    hdrColor.rgb = vec3(1.0, 0.0, 0.0);
  else if (hdrColor.r < 512.0/65535.0 && hdrColor.g < 512.0/65535.0 && hdrColor.b < 512.0/65535.0)
    hdrColor.rgb = vec3(0.0, 0.0, 1.0);
  else if (hdrColor.r < 1024.0/65535.0 && hdrColor.g < 1024.0/65535.0 && hdrColor.b < 1024.0/65535.0)
    hdrColor.rgb = vec3(0.0, 1.0, 0.0);
  else if (hdrColor.r < 1.0 && hdrColor.g < 1.0 && hdrColor.b < 1.0)
    hdrColor.rgb = vec3(0.0, 1.0, 1.0);
  else
    hdrColor.rgb = vec3(1.0, 1.0, 1.0);
**/
/**
  if (hdrColor.r > 20.0*1024.0/65535.0 &&
      hdrColor.g > 20.0*1024.0/65535.0 && hdrColor.b > 20.0*1024.0/65535.0)
    hdrColor.rgb = vec3(1.0, 0.0, 0.0);
  else
    hdrColor.rgb = vec3(0.5, 0.5, 0.5);
  fragColor = vec4(hdrColor, 1.0);
  return;
**/


  // Hue shift
  float hueShift = mix(u_texSrceHueShift, u_texDestHueShift, u_texHueShiftLerpT);
  hdrColor = GetHueShift(hdrColor, hueShift);

  // Chromatic increase
  hdrColor = GetChromaticIncrease(hdrColor);

  // Tone mapping
  vec3 toneMappedColor = GetToneMappedColor(hdrColor, u_texExposure, u_texBrightness);

  //const float contrast = 1.0;
  //toneMappedColor = max((contrast * (toneMappedColor - 0.5)) + 0.5, -0.0);
  toneMappedColor = max((u_texContrast * (toneMappedColor - 0.5)) + 0.5, u_texContrastMinChan);

  fragColor = vec4(toneMappedColor, 1.0);
}
