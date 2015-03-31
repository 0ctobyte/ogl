#version 410 core

// This shader computes diffuse lighting based on a flat per-vertex shading model

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
};

struct Camera {
  vec3 position;
};

uniform mat4 modelviewprojection;
uniform mat4 modelview;
uniform mat4 normalmodelview;
uniform LightSource light = LightSource(vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), 0.005, 0.04);
uniform Material mtl = Material(vec3(0.75, 0.75, 0.75), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), 80.0, 1.0);
uniform Camera cam = Camera(vec3(0.0, 0.0, 0.0));

in vec3 in_Position;
in vec3 in_Normal; 

// No interpolation over the pixel. The per-vertex color is the same over all the fragments
flat out vec4 f_Color;

void main()
{
  // transform normal in world coordinates
  vec3 normal = normalize(mat3(normalmodelview)*in_Normal);

  // Calculate position of this vertex in world space
  vec3 vert_pos = vec3(modelview * vec4(in_Position, 1));

  // Calculate vector from this vertex to light source
  vec3 vert_to_light = light.position - vert_pos;
  
  // Calculate the angle of incidence brightness
  float brightness = max(0.0, dot(normal, normalize(vert_to_light)));

  // Calculate the diffuse component
  vec3 diffuse = brightness * mtl.diffuse * light.intensities;

  // Calculate the angle of reflectance.
  // The surf_to_light needs to go in the opposite direction in order to represent the angle of incidence
  vec3 incidence = normalize(-vert_to_light);
  vec3 reflection = reflect(incidence, normal);
  vec3 vert_to_cam = normalize(cam.position - vert_pos);
  float specular_brightness = max(0.0, dot(vert_to_cam, reflection));
  float specular_coefficient = (brightness > 0.0) ? pow(specular_brightness, mtl.shininess) : 0.0;

  // Calculate the specular component
  vec3 specular = specular_coefficient * mtl.specular * light.intensities;

  // Calculate the ambient component
  vec3 ambient = light.ambient_coefficient * mtl.ambient * light.intensities;

  // Calculate the attenuation based on distance from light source
  float attenuation = 1.0 / (1.0 + light.attenuation * pow(length(vert_to_light), 2));

  // Final color based on 
  // 1. The diffuse component
  // 2. The ambient component
  // 3. The specular component
  // 4. The distance from light source (attenuation)
  // 5. Gamma correction (if needed)
  vec3 linear_color = max(ambient, attenuation * (diffuse + specular));
  f_Color = vec4(pow(linear_color, light.gamma), mtl.transparency);

  gl_Position = modelviewprojection*vec4(in_Position, 1.0);
}

