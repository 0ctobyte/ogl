#include <stdio.h>
#include <string.h>

#include <SDL2/SDL_log.h>

#include "mesh.h"

void mesh_load(mesh_t *mesh, const char *objfile) {
  FILE *f = fopen(objfile, "r");

  if(f == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not open Wavefront OBJ file: %s\n", objfile);
    return;
  }

  // Count the # of vertices
  char line[256];
  while(fgets(line, 256, f) != NULL) {
    if(line[0] == 'v') mesh->num_vertices++;
    else break;
  }

  if(feof(f) != 0 || ferror(f) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading file: %s\n", objfile);
    return;
  }

}

