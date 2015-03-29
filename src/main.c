#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <OpenGL/gl3.h>

#include "mat.h"
#include "shader.h"
#include "mesh.h"
#include "array.h"

void check_gl_errors();
void init();
void update();
void draw();
void key_down(SDL_Event *event);
uint32_t timer(uint32_t interval, void *param);

int main(int argc, char **argv) { 
  bool running = true;
  SDL_Window *window;
  SDL_GLContext *context;

  // Initialize SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  atexit(SDL_Quit);

  // Set the SDL OpenGL attributes
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  
  // Create the SDL window
  window = SDL_CreateWindow("OGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
  if (!window) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create SDL window: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // Set the SDL OpenGL context on the SDL window
  context = SDL_GL_CreateContext(window);
  if(!context) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create OpenGL context: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // Enable VSYNC
  if(SDL_GL_SetSwapInterval(1) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "VSYNC not enabled!: %s\n", SDL_GetError());
  }

  // Enable timer
  if(SDL_AddTimer(1000/60, timer, NULL) == 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't set up timer: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL version: %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "OpenGL vendor: %s\n", glGetString(GL_VENDOR));
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "OpenGL renderer: %s\n", glGetString(GL_RENDERER));
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "OpenGL version: %s\n", glGetString(GL_VERSION));
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

  init();

  // Main loop
  while(running) {
    update();
    SDL_Event event;
    SDL_PollEvent(&event);
    switch(event.type) {
    case SDL_QUIT:
      SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Program quit after %i ticks\n", event.quit.timestamp);
      running = false;
      break;
    case SDL_USEREVENT:
      draw();
      SDL_GL_SwapWindow(window);
      break;
    case SDL_KEYDOWN:
      key_down(&event);
      break;
    }
    check_gl_errors();
  }

  return 0;
}

static uint32_t s_id;
static vec3_t camera, model_rot, model_pos = {0.0f, 0.0f, -10.0f};
static mesh_t mesh;
static mat4_t projection = MAT4_IDENTITY; 
static mat4_t view = MAT4_IDENTITY;
static mat4_t model = MAT4_IDENTITY;

void key_down(SDL_Event *event) {
  switch(event->key.keysym.sym) {
  case SDLK_UP:
    if(event->key.keysym.mod == KMOD_LSHIFT) {
      camera.z -= 0.02f;
    } else {
      camera.y += 0.02f;
    }
    break;
  case SDLK_DOWN:
    if(event->key.keysym.mod == KMOD_LSHIFT) {
      camera.z += 0.02f;
    } else {
      camera.y -= 0.02f;
    }
    break;
  case SDLK_RIGHT:
    camera.x += 0.02f;
    break;
  case SDLK_LEFT:
    camera.x -= 0.02f;
    break;
  case SDLK_w:
    model_rot.x += 0.2f;
    break;
  case SDLK_s:
    model_rot.x -= 0.2f;
    break;
  case SDLK_d:
    model_rot.y += 0.2f;
    break;
  case SDLK_a:
    model_rot.y -= 0.2f;
    break;
  case SDLK_e:
    model_rot.z += 0.2f;
    break;
  case SDLK_q:
    model_rot.z -= 0.2f;
    break;
  }
}

void init() {
  mat4_perspective(&projection, 60.0f, 640.0f/480.0f, 1.0f, 10000.0f);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  s_id = shader_load("shaders/simple.vert.glsl", "shaders/simple.frag.glsl");  

  if(s_id == 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load shaders\n");
    exit(EXIT_FAILURE);
  }

  if(mesh_load(&mesh, "resources/monkey.obj") == false) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load mesh\n");
    exit(EXIT_FAILURE);
  }
}

void update() {
  mat4_identity(&view);
  mat4_translate(&view, &camera);

  mat4_identity(&model);
  mat4_translate(&model, &model_pos);
  mat4_rotatef(&model, model_rot.x, 1.0f, 0.0f, 0.0f);
  mat4_rotatef(&model, model_rot.y, 0.0f, 1.0f, 0.0f);
  mat4_rotatef(&model, model_rot.z, 0.0f, 0.0f, 1.0f);
}

void draw() {
  glClear(GL_COLOR_BUFFER_BIT);

  mesh_bind(&mesh);
  shader_bind(s_id);

  shader_set_attribs(s_id, 3*sizeof(vec3_t), 0, sizeof(vec3_t), 2*sizeof(vec3_t));
  shader_set_uniforms(s_id, projection.m, view.m, model.m);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.buf_ids[MESH_IBO]);
  glDrawElements(GL_TRIANGLES, (GLsizei)array_size(mesh.indices), GL_UNSIGNED_INT, 0);

  shader_unbind();
  mesh_unbind();
}

uint32_t timer(uint32_t interval, void *param) {
  SDL_Event event;
  SDL_UserEvent userevent;

  userevent.type = SDL_USEREVENT;
  userevent.code = 0;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type= SDL_USEREVENT;
  event.user = userevent;

  SDL_PushEvent(&event);
  return interval;
}


void check_gl_errors() {
  GLenum err;
  while((err = glGetError()) != GL_NO_ERROR) {
    switch(err) {
      case GL_INVALID_ENUM:
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "OpenGL error: %s\n", "GL_INVALID_ENUM");
        break;
      case GL_INVALID_VALUE:
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "OpenGL error: %s\n", "GL_INVALID_VALUE");
        break;
      case GL_INVALID_OPERATION:
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "OpenGL error: %s\n", "GL_INVALID_OPERATION");
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "OpenGL error: %s\n", "GL_INVALID_FRAMEBUFFER_OPERATION");
        break;
      case GL_OUT_OF_MEMORY:
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "OpenGL error: %s\n", "GL_OUT_OF_MEMORY");
        break;
    }
  }
}

