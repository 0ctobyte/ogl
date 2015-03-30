#version 410 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

in vec3 in_Position;
in vec3 in_Normal;

out vec3 o_Position;
out vec3 o_Normal;

void main()
{
  o_Position = in_Position;
  o_Normal = in_Normal;
  gl_Position = projection*inverse(view)*model*vec4(in_Position, 1);
}

