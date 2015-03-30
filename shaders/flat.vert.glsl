#version 410 core

in vec4 in_Position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
  gl_Position = projection*inverse(view)*model*in_Position;
}

