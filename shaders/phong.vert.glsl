#version 410 core

uniform mat4 modelviewprojection;
uniform mat4 modelview;
uniform mat4 normalmodelview;

in vec3 in_Position;
in vec3 in_Normal;

// The normals and positions are interpolated for each pixel
smooth out vec3 o_Position;
smooth out vec3 o_Normal;

void main()
{
  // Calculate position of vertex in world space
  o_Position = vec3(modelview * vec4(in_Position, 1));

  // Transform normal to world space
  o_Normal = normalize(mat3(normalmodelview)*in_Normal);

  gl_Position = modelviewprojection*vec4(in_Position, 1);
}

