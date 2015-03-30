#version 410 core

in vec4 in_Position;

uniform mat4 projection;
uniform mat4 modelview;

void main()
{
  gl_Position = projection*modelview*in_Position;
}

