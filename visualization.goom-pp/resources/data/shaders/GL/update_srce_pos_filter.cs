#version 430

#include "update_srce_pos_filter_consts.h"

layout(binding=FILTER_SRCE_POS_IMAGE_UNIT, rg32f) uniform          image2D img_filterSrcePos;
layout(binding=FILTER_DEST_POS_IMAGE_UNIT, rg32f) uniform readonly image2D img_filterDestPos;

uniform float u_lerpFactor;

layout (local_size_x = 1, local_size_y = 1) in;

void main()
{
  ivec2 xy = ivec2(gl_GlobalInvocationID.xy);

  vec2 filterSrcePos = imageLoad(img_filterSrcePos, xy).rg;
  vec2 filterDestPos = imageLoad(img_filterDestPos, xy).rg;

  vec2 newFilterSrcePos = mix(filterSrcePos, filterDestPos, u_lerpFactor);

  imageStore(img_filterSrcePos, xy, vec4(newFilterSrcePos, 0, 0));
}
