# OGL

An OBJ (a file format to describe 3D scenes/models) viewer using OpenGL 4.1 written in C. The program parses only a subset of the OBJ file format specification.

## Build

The Makefile is specific to Mac OS X 10.10 with Apple Clang 6.1.0
Just run `make`

## Run

`./ogl [shader] [obj_model]`

`shader` can be any of:
* simple
* flat
* goraud
* phong

`obj_model` can be any of the .obj files in the resources directory. Just specify the name of the OBJ file without the .obj extension.
For example, to view the resources/bunny.obj model using the phong lighting model:
`./ogl phong bunny

