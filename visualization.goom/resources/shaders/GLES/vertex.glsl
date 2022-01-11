#version 100

varying vec2 texCoords;

uniform mat4 u_projModelMat;

attribute vec2 in_position;
attribute vec2 in_texCoords;

void main()
{
  gl_Position = u_projModelMat * vec4(in_position, 0.0, 1.0);
  texCoords = in_texCoords;
}
