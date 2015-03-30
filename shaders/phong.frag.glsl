#version 410 core

uniform mat4 view;
uniform mat4 model;

in vec3 o_Position;
in vec3 o_Normal; 

out vec4 f_Color;

void main()
{
  // Get camera position, this is also the light position
  vec4 cam_pos = view[3];
  vec3 light_col = vec3(1.0f, 1.0f, 1.0f);
  vec3 surface_col = vec3(0.0f, 0.0f, 1.0f);

  // transform normal in world coordinates
  mat3 normal_matrix = transpose(inverse(mat3(inverse(view)*model)));
  vec3 normal = normalize(normal_matrix*o_Normal);

  // Calculate the position of this fragment in world coordinates
  vec3 frag_pos = vec3(inverse(view)*model * vec4(o_Position, 1));

  // Calculate vector from this pixel's surface to light source
  vec3 surf_to_light = vec3(cam_pos) - frag_pos;

  // Calculate the cosine of the angle of incidence (brightness)
  // Brightness must be clamped between 0 and 1 (anything less than 0 means 0 brightness)
  float brightness = dot(normal, surf_to_light) / (length(surf_to_light) * length(normal));
  brightness = clamp(brightness, 0, 1);

  // Calculate the final color based on
  // 1. The angle of incidence: brightness
  // 2. The color/intensities of the light
  // 3. The surface color
  f_Color = vec4(brightness * light_col * surface_col, 1.0f);
}

