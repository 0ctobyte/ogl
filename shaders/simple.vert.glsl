#version 410 core

// This shader colors the model uniformly, no lighting

uniform mat4 modelviewprojection;

layout(location = 0) in vec3 in_Position;

void main()
{
  gl_Position = modelviewprojection*vec4(in_Position, 1);
}

