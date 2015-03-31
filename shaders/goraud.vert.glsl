#version 410 core

// This shader computes diffuse lighting based on the Goraud per-vertex lighting model

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

uniform mat4 modelviewprojection;
uniform mat4 modelview;
uniform mat4 normalmodelview;
uniform LightSource light = LightSource(vec3(0.0, 0.0, 0.0),
                                        vec3(1.0, 1.0, 1.0),
                                        0.005, 0.04);
uniform Material mtl = Material(vec3(0.75, 0.75, 0.75),
                                vec3(1.0, 1.0, 1.0));
in vec3 in_Position;
in vec3 in_Normal; 

// The per-vertex color is interpolated over the pixels
smooth out vec4 f_Color;

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
  vec3 diffuse = brightness * mtl.ambient * light.intensities;

  // Calculate the ambient component
  vec3 ambient = light.ambient_coefficient * mtl.ambient * light.intensities;

  // Calculate the attenuation based on distance from light source
  float attenuation = 1.0 / (1.0 + light.attenuation * pow(length(vert_to_light), 2));

  // Final color based on 
  // 1. The diffuse component
  // 2. The ambient component
  // 3. The distance from light source (attenuation)
  f_Color = vec4(max(ambient, attenuation * diffuse), 1.0f);

  gl_Position = modelviewprojection*vec4(in_Position, 1);
}

