#version 410 core

// This shader computes diffuse lighting based on a flat per-vertex shading model

struct Material {
  vec3 diffuse;
};

uniform Material mtl = Material(vec3(0.75, 0.75, 0.75));

flat out vec4 f_Color;

void main()
{
  f_Color = vec4(mtl.diffuse, 1.0f);
}

