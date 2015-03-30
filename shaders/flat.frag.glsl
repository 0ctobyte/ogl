#version 410 core

uniform vec3 surface_col;

out vec4 fColor;

void main()
{
  fColor = vec4(surface_col, 1.0f);
}

