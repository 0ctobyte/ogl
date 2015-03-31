#version 410 core

// This shader computes diffuse lighting based on the Goraud per-vertex lighting model

// The per-vertex color is interpolated over the pixels
smooth in vec4 f_Color;

out vec4 o_Color;

void main()
{
  o_Color = f_Color;
}

