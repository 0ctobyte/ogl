#version 410 core

// This shader computes diffuse lighting based on the Phong per-pixel lighting model

struct LightSource {
  vec3 position;
  vec3 intensities;
  float attenuation;
  float ambient_coefficient;
};

struct Material {
  vec3 diffuse;
  vec3 ambient;
};

uniform mat4 modelview;
uniform mat4 normalmodelview;
uniform LightSource light = LightSource(vec3(0.0, 0.0, 0.0),
                                        vec3(1.0, 1.0, 1.0),
                                        0.005, 0.04);
uniform Material mtl = Material(vec3(0.75, 0.75, 0.75),
                                vec3(1.0, 1.0, 1.0));

// The normals and positions are interpolated for each pixel
smooth in vec3 o_Position;
smooth in vec3 o_Normal; 

out vec4 f_Color;

void main()
{
  // Calculate vector from this pixel's surface to light source
  vec3 surf_to_light = light.position - o_Position;

  // Calculate the cosine of the angle of incidence (brightness)
  // (no need to divide the dot product by the product of the lengths of the vectors since they have been normalized)
  // Brightness must be clamped between 0 and 1 (anything less than 0 means 0 brightness)
  float brightness = max(0.0, dot(o_Normal, normalize(surf_to_light)));

  // Calculate the diffuse component
  vec3 diffuse = brightness * mtl.diffuse * light.intensities;

  // Calculate the ambient component
  vec3 ambient = light.ambient_coefficient * mtl.ambient * light.intensities;

  // Calculate the attenuation based on distance from light source
  float attenuation = 1.0 / (1.0 + light.attenuation * pow(length(surf_to_light), 2));

  // Calculate the final color based on
  // 1. The diffuse component
  // 2. The ambient component
  // 3. The distance from the light source (attenuation)
  f_Color = vec4(max(ambient, attenuation * diffuse), 1.0f);
}

