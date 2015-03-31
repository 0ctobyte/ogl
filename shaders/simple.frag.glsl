#version 410 core

// This shader colors the model uniformly, no lighting

struct Material {
  vec3 diffuse;
};

uniform Material mtl = Material(vec3(0.75, 0.75, 0.75));

out vec4 o_Color;

void main()
{
  o_Color = vec4(mtl.diffuse, 1.0f);
}

