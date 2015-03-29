#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <OpenGL/gl3.h>

#include "shader.h"

uint32_t _shader_compile(const char *shadercode, uint32_t shader_type) {
  // Create the shader object
  uint32_t shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &shadercode, NULL);
  glCompileShader(shader);

  // Check if compilation went okay
  int32_t status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if(status == GL_FALSE) {
    int32_t info_log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);

    char *info_log = (char*)malloc((size_t)(info_log_length+1));
    glGetShaderInfoLog(shader, info_log_length, NULL, info_log);

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile %s shader:\n%s\n", (shader_type = GL_VERTEX_SHADER) ? "vertex" : "fragment", info_log);

    glDeleteShader(shader);
    free(info_log);
    return 0;
  }

  return shader;
}

uint32_t _shader_link(uint32_t vshader, uint32_t fshader) {
  // Link the shader objects to the program
  uint32_t program = glCreateProgram();
  glAttachShader(program, vshader);
  glAttachShader(program, fshader);
  glLinkProgram(program);

  int32_t status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if(status == GL_FALSE) {
    int32_t info_log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);

    char *info_log = (char*)malloc((size_t)(info_log_length+1));
    glGetProgramInfoLog(program, info_log_length, NULL, info_log);

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Linker failure: %s\n", info_log);

    free(info_log);
    glDetachShader(program, vshader);
    glDeleteShader(vshader);
    glDetachShader(program, fshader);
    glDeleteShader(fshader);
    glDeleteProgram(program);
    return 0;
  }

  // The shader objects aren't needed anymore after linking, so remove them
  glDetachShader(program, vshader);
  glDeleteShader(vshader);
  glDetachShader(program, fshader);
  glDeleteShader(fshader);

  return program;
}
 
uint32_t shader_load(const char *vertfile, const char *fragfile) {
  // Get the number of bytes in the file
  FILE *f = fopen(vertfile, "rb");
  
  if(f == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not open file: %s\n", vertfile);
    return 0;
  }

  fseek(f, 0, SEEK_END);
  size_t fsize = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);

  // Read the vertex shader file into a buffer
  char *vertcode = (char*)malloc(fsize+1);
  fread(vertcode, fsize, 1, f);
  fclose(f);

  // NULL terminated buffer
  vertcode[fsize] = 0;

  // Compile the vertex shader
  uint32_t vshader = _shader_compile(vertcode, GL_VERTEX_SHADER);
  if(vshader == 0) return 0;
  
  free(vertcode);

  // Get the number of bytes in the file
  f = fopen(fragfile, "rb");
  
  if(f == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not open file: %s\n", vertfile);
    return 0;
  }

  fseek(f, 0, SEEK_END);
  fsize = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);

  // Read the fragment shader file into a buffer
  char *fragcode = (char*)malloc(fsize+1);
  fread(fragcode, fsize, 1, f);
  fclose(f);

  // NULL terminated buffer
  fragcode[fsize] = 0;

  // Compile the fragment shader
  uint32_t fshader = _shader_compile(fragcode, GL_FRAGMENT_SHADER);
  if(fshader == 0) return 0;
    
  free(fragcode);

  // Link the compiled shader objects
  uint32_t program = _shader_link(vshader, fshader);
  if(program == 0) return 0;

  // Unbind the shader program
  glUseProgram(0);

  return program;
}

