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

typedef struct {
  vec3_t position;
  vec3_t intensities;
  float attenuation;
  float ambient_coefficient;
} lightsource_t;

static uint32_t s_id;
static mesh_t mesh;
static vec3_t camera, model_rot, model_pos = {0.0f, 0.0f, -10.0f};
static mat4_t projection = MAT4_IDENTITY, modelviewprojection = MAT4_IDENTITY, modelview = MAT4_IDENTITY, normalmodelview = MAT4_IDENTITY; 
static lightsource_t light = {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, 0.005f, 0.04f};

void init() {
  // Set up a perspective projection matrix
  mat4_perspective(&projection, 60.0f, (float)w/(float)h, 1.0f, 10000.0f);
  
  // Initial update
  update();
  
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
 float step_size = 0.2f, rot_mult = 10.0f;
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
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  case SDLK_g:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  case SDLK_p:
    break;
  }
}

void update() {
  // Update the modelview matrix, improves performance since shaders don't have to compute this for every vertex
  // Translate the camera
  mat4_identity(&modelview);
  mat4_translate(&modelview, &camera);
  mat4_inverse(&modelview);

  // Apply the rotation and translation to the model
  mat4_translate(&modelview, &model_pos);
  mat4_rotatef(&modelview, model_rot.x, 1.0f, 0.0f, 0.0f);
  mat4_rotatef(&modelview, model_rot.y, 0.0f, 1.0f, 0.0f);
  mat4_rotatef(&modelview, model_rot.z, 0.0f, 0.0f, 1.0f);

  // update the modelviewprojection matrix
  modelviewprojection = projection;
  mat4_mult(&modelviewprojection, &modelview);

  // Update the modelview matrix for the normals
  // The normals can be rotated but translation and scaling should not be applied to the normals
  normalmodelview = modelview;
  mat4_untranslate(&normalmodelview);
  mat4_inverse(&normalmodelview);
  mat4_transpose(&normalmodelview);

  // The point light will track the camera
  light.position = camera;
}

void draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  mesh_bind(&mesh);
  shader_bind(s_id);

  // Set the per vertex attributes for the shader
  shader_set_attrib(s_id, "in_Position", 3*sizeof(vec3_t), 0);
  shader_set_attrib(s_id, "in_Normal", 3*sizeof(vec3_t), 2*sizeof(vec3_t));

  // Set the uniform variables
  shader_set_uniform(s_id, "modelviewprojection", SHADER_UNIFORM_MAT4, modelviewprojection.m);
  shader_set_uniform(s_id, "modelview", SHADER_UNIFORM_MAT4, modelview.m);
  shader_set_uniform(s_id, "normalmodelview", SHADER_UNIFORM_MAT4, normalmodelview.m);
  shader_set_uniform(s_id, "light.position", SHADER_UNIFORM_VEC3, &light.position); 
  shader_set_uniform(s_id, "light.intensities", SHADER_UNIFORM_VEC3, &light.intensities); 
  shader_set_uniform(s_id, "light.attenuation", SHADER_UNIFORM_FLOAT, &light.attenuation); 
  shader_set_uniform(s_id, "light.ambient_coefficient", SHADER_UNIFORM_FLOAT, &light.ambient_coefficient); 
   
  size_t size = array_size(mesh.faces);
  for(uint32_t i = 0; i < size; i++) {
    face_group_t *face = (face_group_t*)array_at(mesh.faces, i);

    shader_set_uniform(s_id, "mtl.diffuse", SHADER_UNIFORM_VEC3, &face->mtl.diffuse);
    shader_set_uniform(s_id, "mtl.ambient", SHADER_UNIFORM_VEC3, &face->mtl.ambient);
    shader_set_uniform(s_id, "mtl.specular", SHADER_UNIFORM_VEC3, &face->mtl.specular);
    shader_set_uniform(s_id, "mtl.shininess", SHADER_UNIFORM_FLOAT, &face->mtl.shininess);
    shader_set_uniform(s_id, "mtl.transparency", SHADER_UNIFORM_VEC3, &face->mtl.transparency);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face->ibo);
    glDrawElements(GL_TRIANGLES, (GLsizei)array_size(face->indices), GL_UNSIGNED_INT, 0);
  }

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

