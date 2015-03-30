#version 410 core

uniform mat4 modelview;
uniform mat4 normalmodelview;
uniform vec3 surface_col;
uniform struct Light {
  vec3 position;
  vec3 diffuse;
  vec3 ambient;
  float ambient_coefficient;
} light;

in vec3 o_Position;
in vec3 o_Normal; 

out vec4 f_Color;

void main()
{
  // transform normal in world coordinates
  vec3 normal = normalize(mat3(normalmodelview)*o_Normal);

  // Calculate the position of this fragment in world coordinates
  vec3 frag_pos = vec3(modelview * vec4(o_Position, 1));

  // Calculate vector from this pixel's surface to light source
  vec3 surf_to_light = normalize(light.position - frag_pos);

  // Calculate the cosine of the angle of incidence (brightness)
  // (no need to divide the dot product by the product of the lengths of the vectors since they have been normalized)
  // Brightness must be clamped between 0 and 1 (anything less than 0 means 0 brightness)
  float diffuse_coefficient = max(0.0, dot(normal, surf_to_light));

  // Calculate the diffuse component
  vec3 diffuse = diffuse_coefficient * surface_col * light.diffuse;

  // Calculate the ambient component
  vec3 ambient = light.ambient_coefficient * surface_col * light.ambient;

  // Calculate the final color based on
  // 1. The diffuse component
  // 2. The ambient component
  f_Color = vec4(diffuse + ambient, 1.0f);
}

