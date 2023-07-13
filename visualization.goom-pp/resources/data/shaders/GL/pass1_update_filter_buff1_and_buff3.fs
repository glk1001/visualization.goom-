#version 430

#include "pass1_update_filter_buff1_and_buff3_consts.h"

//#include "glsl-blend/all.glsl"

uniform sampler2D tex_filterBuff2;
uniform sampler2D tex_filterSrcePositions;
uniform sampler2D tex_filterDestPositions;
uniform sampler2D tex_mainImage;
uniform sampler2D tex_lowImage;

layout(binding=FILTER_BUFF1_IMAGE_UNIT, rgba16) uniform image2D img_filterBuff1;
layout(binding=FILTER_BUFF3_IMAGE_UNIT, rgba16) uniform image2D img_filterBuff3;

in vec3 position;
in vec2 texCoord;

uniform float u_lerpFactor;
uniform float u_buff2Buff3Mix = 0.1;

// For base multiplier, too close to 1, gives washed
// out look, too far away things get too dark.
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

vec4 GetPosMappedFilterBuff2Value(vec2 uv);

void main()
{
  vec4 filtBuff2Val = GetPosMappedFilterBuff2Value(texCoord);

  ivec2 xy = ivec2(gl_FragCoord.xy);
  vec4 filtBuff3Val = imageLoad(img_filterBuff3, xy);

  vec4 colorMain = texture(tex_mainImage, texCoord);
  vec4 colorLow  = texture(tex_lowImage, texCoord);

  //  vec4 filtBuff2ColorMain = vec4(blend(filtBuff2Val.rgb, 90*colorMain.rgb, 0.5*colorMain.a), 1.0);
  //  vec4 filtBuff2ColorMain = vec4((1-colorMain.a)*filtBuff2Val.rgb + colorMain.a*colorMain.rgb, 1.0);
  //  float alpha = 1 - 0.5 * colorMain.a;
  //  vec4 filtBuff2ColorMain = (1-alpha)*filtBuff2Val + alpha*50*colorMain;
  //  vec4 filtBuff2ColorMain = vec4(blend(100*colorMain.rgb, 0.5*filtBuff2Val.rgb, colorMain.a), 1.0);
  //  vec4 filtBuff2ColorMain = vec4(filtBuff2Val.rgb, 1.0);

  filtBuff2Val.rgb = mix(filtBuff2Val.rgb, filtBuff3Val.rgb, u_buff2Buff3Mix);

//  filtBuff2Val.rgb *= 1.00;
  filtBuff2Val.rgb *= u_baseColorMultiplier;

  vec3 filtBuff2ColorMain = filtBuff2Val.rgb + (u_mainColorMultiplier * colorMain.rgb);
  vec3 filtBuff2ColorLow  = filtBuff2Val.rgb + (u_lowColorMultiplier * colorLow.rgb);

  imageStore(img_filterBuff1, xy, vec4(filtBuff2ColorLow, colorLow.a));
  imageStore(img_filterBuff3, xy, vec4(filtBuff2ColorMain, colorLow.a));

  discard;
}

vec4 GetPosMappedFilterBuff2Value(vec2 uv)
{
  vec2 srceNormalizedPos = texture(tex_filterSrcePositions, uv).xy;
  vec2 destNormalizedPos = texture(tex_filterDestPositions, uv).xy;

  vec2 lerpNormalizedPos = mix(srceNormalizedPos, destNormalizedPos, u_lerpFactor);

  vec2 filtBuff2Pos = vec2((lerpNormalizedPos.x - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH,
                           (lerpNormalizedPos.y - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH);


  //  vec4 tex = texture(tex_lowImage, vec2(filtBuff2Pos.x, 1 - (ASPECT_RATIO * filtBuff2Pos.y)));
  //  return vec4(tex.x, tex.y, filtBuff2Pos.x, 1 - (ASPECT_RATIO * filtBuff2Pos.y));

  //  vec4 tex = texture(tex_filterBuff2, vec2(filtBuff2Pos.x, 1 - (ASPECT_RATIO * filtBuff2Pos.y)));
  //  return vec4(tex.x, tex.y, uv.x, uv.y);

  return texture(tex_filterBuff2, vec2(filtBuff2Pos.x, 1 - (ASPECT_RATIO * filtBuff2Pos.y)));
}
