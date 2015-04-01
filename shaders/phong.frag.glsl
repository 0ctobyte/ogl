#version 410 core

// This shader computes diffuse lighting based on the Phong per-pixel lighting model

struct LightSource {
  vec3 position;
  vec3 intensities;
  vec3 gamma;
  float attenuation;
  float ambient_coefficient;
};

struct Material {
  vec3 diffuse;
  vec3 ambient;
  vec3 specular;
  float shininess;
  float transparency;
  bool use_texture;
};

struct Camera {
  vec3 position;
};

uniform mat4 modelview;
uniform mat4 normalmodelview;
uniform LightSource light = LightSource(vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), 0.005, 0.04);
uniform Material mtl = Material(vec3(0.75, 0.75, 0.75), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), 80.0, 1.0, false);
uniform Camera cam = Camera(vec3(0.0, 0.0, 0.0));
uniform sampler2D tex;

// The normals and positions are interpolated for each pixel
smooth in vec3 o_Position;
smooth in vec3 o_TexCoord;
smooth in vec3 o_Normal; 

out vec4 f_Color;

void main()
{
  // Calculate vector from this pixel's surface to light source
  vec3 surf_to_light = light.position - o_Position;

  // Determine whether use texture or diffuse material color
  vec4 surface_color = (mtl.use_texture) ? texture(tex, o_TexCoord) : vec4(mtl.diffuse, 1.0);

  // Calculate the cosine of the angle of incidence (brightness)
  // (no need to divide the dot product by the product of the lengths of the vectors since they have been normalized)
  // Brightness must be clamped between 0 and 1 (anything less than 0 means 0 brightness)
  float brightness = max(0.0, dot(o_Normal, normalize(surf_to_light)));

  // Calculate the diffuse component
  vec3 diffuse = brightness * surface_color.rgb * light.intensities;

  // Calculate the angle of reflectance.
  // The surf_to_light needs to go in the opposite direction in order to represent the angle of incidence
  vec3 incidence = normalize(-surf_to_light);
  vec3 reflection = reflect(incidence, o_Normal);
  vec3 surf_to_cam = normalize(cam.position - o_Position);
  float specular_brightness = max(0.0, dot(surf_to_cam, reflection));
  float specular_coefficient = (brightness > 0.0) ? pow(specular_brightness, mtl.shininess) : 0.0;

  // Calculate the specular component
  vec3 specular = specular_coefficient * mtl.specular * light.intensities;

  // Calculate the ambient component
  vec3 ambient = light.ambient_coefficient * mtl.ambient * light.intensities;

  // Calculate the attenuation based on distance from light source
  float attenuation = 1.0 / (1.0 + light.attenuation * pow(length(surf_to_light), 2));
 
  // Calculate the final color based on
  // 1. The diffuse component
  // 2. The ambient component
  // 3. The specular component
  // 4. The distance from the light source (attenuation)
  // 5. Gamma correction (if needed)
  vec3 linear_color = max(ambient, attenuation * (diffuse + specular));
  f_Color = vec4(pow(linear_color, light.gamma), mtl.transparency);
}

