#version 430

layout(location = 0) out vec4 fragColor;

// TODO - #include these
#define FILTER_BUFF1_IMAGE_UNIT    0
#define FILTER_BUFF2_IMAGE_UNIT    1
#define FILTER_BUFF3_IMAGE_UNIT    2
#define LUM_AVG_IMAGE_UNIT         3

#define LUM_HISTOGRAM_BUFFER_INDEX 3

//#include "glsl-blend/all.glsl"
#include "tone-maps.glsl"

uniform sampler2D tex_filterBuff2;
uniform sampler2D tex_filterSrcePositions;
uniform sampler2D tex_filterDestPositions;
uniform sampler2D tex_mainImage;
uniform sampler2D tex_lowImage;

layout(binding=FILTER_BUFF1_IMAGE_UNIT, rgba16) uniform image2D img_filterBuff1;
layout(binding=FILTER_BUFF2_IMAGE_UNIT, rgba16) uniform image2D img_filterBuff2;
layout(binding=FILTER_BUFF3_IMAGE_UNIT, rgba16) uniform image2D img_filterBuff3;

in vec3 position;
in vec2 texCoord;

uniform float u_averageLuminance;
uniform float u_brightness;

uniform float u_lerpFactor;
uniform float u_filterPosMinCoord   = -2.0;
uniform float u_filterPosCoordWidth = +4.0;

subroutine vec4 RenderPassType();
subroutine uniform RenderPassType RenderPass;

void main()
{
  // This will call either pass 1 or pass 2.
  fragColor = RenderPass();
}


/**
vec3 blend(vec3 base, vec3 blend, float opacity)
{
//  return blendAdd(base, blend, opacity);
  return blendMode(1, base, blend, opacity);
//  return blendLighten(base, blend, opacity);
}
**/

vec4 GetPosMappedFilterBuff2Value(vec2 uv);

subroutine (RenderPassType) vec4 Pass1UpdateFilterBuffers()
{
  ivec2 xy = ivec2(gl_FragCoord.xy);

  vec4 filtBuff2Val = GetPosMappedFilterBuff2Value(texCoord);

  vec4 colorMain = texture(tex_mainImage, texCoord);
  vec4 colorLow  = texture(tex_lowImage, texCoord);

//  vec4 filtBuff2ColorMain = vec4(blend(filtBuff2Val.rgb, 90*colorMain.rgb, 0.5*colorMain.a), 1.0);
//  vec4 filtBuff2ColorMain = vec4((1-colorMain.a)*filtBuff2Val.rgb + colorMain.a*colorMain.rgb, 1.0);
//  float alpha = 1 - 0.5 * colorMain.a;
//  vec4 filtBuff2ColorMain = (1-alpha)*filtBuff2Val + alpha*50*colorMain;
//  vec4 filtBuff2ColorMain = vec4(blend(100*colorMain.rgb, 0.5*filtBuff2Val.rgb, colorMain.a), 1.0);
//  vec4 filtBuff2ColorLow  = vec4(blend(filtBuff2Val.rgb, colorLow.rgb, 1.0), 1.0);
  vec4 filtBuff2ColorMain = vec4(filtBuff2Val.rgb + colorMain.rgb, 1.0);
  vec4 filtBuff2ColorLow  = vec4(filtBuff2Val.rgb + colorLow.rgb, 1.0);

  imageStore(img_filterBuff1, xy, filtBuff2ColorLow);
  imageStore(img_filterBuff3, xy, filtBuff2ColorMain);

  discard;

  return vec4(1.0);
}


subroutine (RenderPassType) vec4 Pass2OutputToneMappedImage()
{
  ivec2 xy = ivec2(gl_FragCoord.xy);

  vec4 filtBuff3Val = imageLoad(img_filterBuff3, xy);

  // Copy filter buff1 to filter buff2 ready for next frame.
  vec4 filtBuff1Val = imageLoad(img_filterBuff1, xy);
  imageStore(img_filterBuff2, xy, filtBuff1Val);

  vec3 hdrColor = filtBuff3Val.rgb;

  vec3 toneMappedColor = GetToneMappedColor(hdrColor, u_averageLuminance, 20.0*u_brightness);
//  vec3 toneMappedColor = GetToneMappedColor(hdrColor, 1.0, u_brightness);
//  vec3 toneMappedColor = hdrColor;

  return vec4(toneMappedColor, 1.0F);
}


vec4 GetPosMappedFilterBuff2Value(vec2 uv)
{
  vec2 srcePos = (texture(tex_filterSrcePositions, uv).xy - u_filterPosMinCoord) / u_filterPosCoordWidth;
  vec2 destPos = (texture(tex_filterDestPositions, uv).xy - u_filterPosMinCoord) / u_filterPosCoordWidth;

  vec2 filtBuff2Pos = mix(srcePos, destPos, u_lerpFactor);

  return texture(tex_filterBuff2, filtBuff2Pos);
}
