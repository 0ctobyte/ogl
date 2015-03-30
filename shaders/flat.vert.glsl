#version 410 core

uniform mat4 modelviewprojection;

in vec4 in_Position;

void main()
{
  gl_Position = modelviewprojection*in_Position;
}

