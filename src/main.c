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
void key_down(SDL_Event *event);
void update();
void draw();
uint32_t timer(uint32_t interval, void *param);

static char obj_model[256], vertex_shader[256], fragment_shader[256];
static int32_t w, h;

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
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
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
  //if(SDL_GL_SetSwapInterval(1) != 0) {
  //  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "VSYNC not enabled!: %s\n", SDL_GetError());
  //}

  // Enable timer
  if(SDL_AddTimer(1000/60, timer, NULL) == 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't set up timer: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // Get size of SDL drawable portion of the window
  SDL_GL_GetDrawableSize(window, &w, &h);

  // Get the OpenGL context settings
  int32_t minor_version, major_version, profile_mask;
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major_version);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor_version);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile_mask);

  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL version: %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL OpenGL context: %d.%d %s\n", major_version, minor_version, (profile_mask == SDL_GL_CONTEXT_PROFILE_CORE) ? "core profile" : ((profile_mask == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY) ? "compatibility profile" : "ES profile"));
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "OpenGL vendor: %s\n", glGetString(GL_VENDOR));
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "OpenGL renderer: %s\n", glGetString(GL_RENDERER));
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "OpenGL version: %s\n", glGetString(GL_VERSION));
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

  sprintf(obj_model, "resources/teapot.obj");
  sprintf(vertex_shader, "shaders/flat.vert.glsl");
  sprintf(fragment_shader, "shaders/flat.frag.glsl");

  if(argc > 1) {
    snprintf(obj_model, 256, "resources/%s.obj", argv[1]);
  }

  if(argc > 2) {
    snprintf(vertex_shader, 256, "shaders/%s.vert.glsl", argv[2]);
    snprintf(fragment_shader, 256, "shaders/%s.frag.glsl", argv[2]);
  }

  init();

  // Main loop
  while(running) {
    SDL_Event event;
    SDL_WaitEvent(&event);
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
      update();
      break;
    }
    //check_gl_errors();
  }

  return 0;
}

static uint32_t s_id;
static float step_size = 0.2f, rot_mult = 10.0f;
static vec3_t camera, light_position, model_rot, model_pos = {0.0f, 0.0f, -10.0f};
static vec3_t surface_color, light_color;
static mesh_t mesh;
static mat4_t projection = MAT4_IDENTITY; 
static mat4_t view = MAT4_IDENTITY;
static mat4_t model = MAT4_IDENTITY;

void init() {
  mat4_perspective(&projection, 60.0f, (float)w/(float)h, 1.0f, 10000.0f);
  mat4_translate(&model, &model_pos);

  // The surface light is a soft grey whereas the point light is pure white light
  surface_color.x = 0.75f; surface_color.y = 0.75f; surface_color.z = 0.75f;
  light_color.x = 1.0f; light_color.y = 1.0f; light_color.z = 1.0f;
  //light_position.x = -100.0f; light_position.y = 100.0f; light_position.z = 100.0f;

  // Set the viewport to the current window dimensions
  glViewport(0, 0, w, h);

  // Enable scissor test and set scissor box to the size of the window
  glEnable(GL_SCISSOR_TEST);
  glScissor(0, 0, w, h);

  // Set the color and dpeth buffer clear values
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepth(1.0f);

  // Enable back face culling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_CLAMP);
  glDepthMask(GL_TRUE);
  glDepthRange(0.0f, 1.0f);

  s_id = shader_load(vertex_shader, fragment_shader);  

  if(s_id == 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load shaders\n");
    exit(EXIT_FAILURE);
  }

  if(mesh_load(&mesh, obj_model) == false) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load mesh\n");
    exit(EXIT_FAILURE);
  }
}

void key_down(SDL_Event *event) {
  switch(event->key.keysym.sym) {
  case SDLK_UP:
    if(event->key.keysym.mod == KMOD_LSHIFT) {
      camera.z -= step_size;
    } else {
      camera.y += step_size;
    }
    break;
  case SDLK_DOWN:
    if(event->key.keysym.mod == KMOD_LSHIFT) {
      camera.z += step_size;
    } else {
      camera.y -= step_size;
    }
    break;
  case SDLK_RIGHT:
    camera.x += step_size;
    break;
  case SDLK_LEFT:
    camera.x -= step_size;
    break;
  case SDLK_w:
    model_rot.x += rot_mult*step_size;
    break;
  case SDLK_s:
    model_rot.x -= rot_mult*step_size;
    break;
  case SDLK_d:
    model_rot.y += rot_mult*step_size;
    break;
  case SDLK_a:
    model_rot.y -= rot_mult*step_size;
    break;
  case SDLK_e:
    model_rot.z += rot_mult*step_size;
    break;
  case SDLK_q:
    model_rot.z -= rot_mult*step_size;
    break;
  case SDLK_f:
    glPolygonMode(GL_FRONT, GL_LINE);
    break;
  case SDLK_g:
    glPolygonMode(GL_FRONT, GL_FILL);
    break;
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

  // The point light will track the camera
  light_position = camera;
}

void draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mesh_bind(&mesh);
  shader_bind(s_id);

  // Set the per vertex attributes for the shader
  shader_set_attrib(s_id, "in_Position", 3*sizeof(vec3_t), 0);
  shader_set_attrib(s_id, "in_Normal", 3*sizeof(vec3_t), 2*sizeof(vec3_t));

  // Set the uniform variables
  shader_set_uniform(s_id, "projection", SHADER_UNIFORM_MAT4, projection.m);
  shader_set_uniform(s_id, "view", SHADER_UNIFORM_MAT4, view.m);
  shader_set_uniform(s_id, "model", SHADER_UNIFORM_MAT4, model.m);
  shader_set_uniform(s_id, "surface_col", SHADER_UNIFORM_VEC3, &surface_color);
  shader_set_uniform(s_id, "light.position", SHADER_UNIFORM_VEC3, &light_position); 
  shader_set_uniform(s_id, "light.color", SHADER_UNIFORM_VEC3, &light_color); 

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

