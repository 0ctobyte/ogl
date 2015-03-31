#version 410 core

// This shader computes diffuse lighting based on a flat per-vertex shading model

flat in vec4 f_Color;

out vec4 o_Color;

void main()
{
  o_Color = f_Color;
}

