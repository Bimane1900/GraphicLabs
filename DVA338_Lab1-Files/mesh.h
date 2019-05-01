#ifndef _MESH_H_
#define _MESH_H_

#include "algebra.h"

typedef struct _Triangle {
	int vInds[3]; //vertex indices
} Triangle;

typedef struct _Mesh { 
	int nv;				
	Vector *vertices;
	Vector *vnorms;
	int nt;				
	Triangle *triangles;
	Vector rotation;
	Vector scale;
	Vector translation;
	struct _Mesh *next; 
	char* name;
	unsigned int vbo, ibo, vao; // OpenGL handles for rendering
} Mesh;

typedef struct _Camera {
	Vector position;
	Vector rotation;
	double fov; 
	double nearPlane; 
	double farPlane; 
} Camera;

void insertModel(Mesh ** objlist, int nv, float * vArr, int nt, int * tArr, float scale = 1.0, char* name = (char*)"Name", Vector t = { 0,0,0 }, Vector r = { 0,0,0 }, Vector s = { 1,1,1 });

#endif
