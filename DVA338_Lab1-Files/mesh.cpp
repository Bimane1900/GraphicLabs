#include <stdlib.h>
#include "mesh.h"

float rnd() {
	return 2.0f * float(rand()) / float(RAND_MAX) - 1.0f;
}

void insertModel(Mesh **list, int nv, float * vArr, int nt, int * tArr, float scale) {
	Mesh * mesh = (Mesh *) malloc(sizeof(Mesh));
	mesh->nv = nv;
	mesh->nt = nt;	
	mesh->vertices = (Vector *) malloc(nv * sizeof(Vector));
	mesh->vnorms = (Vector *)malloc(nv * sizeof(Vector));
	mesh->triangles = (Triangle *) malloc(nt * sizeof(Triangle));
	
	// set mesh vertices
	for (int i = 0; i < nv; i++) {
		mesh->vertices[i].x = vArr[i*3] * scale;
		mesh->vertices[i].y = vArr[i*3+1] * scale;
		mesh->vertices[i].z = vArr[i*3+2] * scale;
	}

	// set mesh triangles
	for (int i = 0; i < nt; i++) {
		mesh->triangles[i].vInds[0] = tArr[i*3];
		mesh->triangles[i].vInds[1] = tArr[i*3+1];
		mesh->triangles[i].vInds[2] = tArr[i*3+2];
	}

	// Assignment 1: 
	// Calculate and store suitable vertex normals for the mesh here.
	// Replace the code below that simply sets some arbitrary normal values	
	for (int i = 0; i < nv; i++) {
		mesh->vnorms[i].x = rnd();
		mesh->vnorms[i].y = rnd();
		mesh->vnorms[i].z = rnd();
	}

	mesh->next = *list;
	*list = mesh;	
}
