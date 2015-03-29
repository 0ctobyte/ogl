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

  // Check if linking went okay
  int32_t status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if(status == GL_FALSE) {
    int32_t info_log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);

    char *info_log = (char*)malloc((size_t)(info_log_length+1));
    glGetProgramInfoLog(program, info_log_length, NULL, info_log);

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Linker failure: %s\n", info_log);

    // Cleanup
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
  FILE *f = fopen(vertfile, "rb");
 
  // Make sure the file was opened
  if(f == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not open file: %s\n", vertfile);
    return 0;
  }

  // Get the number of bytes in the file
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

  f = fopen(fragfile, "rb");
  
  if(f == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not open file: %s\n", vertfile);
    return 0;
  }

  // Get the number of bytes in the file
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

void shader_bind(uint32_t s_id) {
  glUseProgram(s_id);
}

void shader_set_attribs(uint32_t s_id, size_t stride, uintptr_t vert_offset, uintptr_t uv_offset, uintptr_t norm_offset) {
  glUseProgram(s_id);
  int32_t attrib_loc = glGetAttribLocation(s_id, "in_Position");
  glVertexAttribPointer((uint32_t)attrib_loc, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, (GLvoid*)vert_offset);
  glEnableVertexAttribArray((uint32_t)attrib_loc);
}

void shader_set_uniforms(uint32_t s_id, float *m_p, float *m_v, float *m_m) {
  glUseProgram(s_id);

  int32_t uniform_loc = glGetUniformLocation(s_id, "projection");
  glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, m_p);

  uniform_loc = glGetUniformLocation(s_id, "view");
  glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, m_v);

  uniform_loc = glGetUniformLocation(s_id, "model");
  glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, m_m);
}

void shader_unbind() {
  glUseProgram(0);
}

void shader_delete(uint32_t s_id) {
  glUseProgram(0);
  glDeleteProgram(s_id);
}

