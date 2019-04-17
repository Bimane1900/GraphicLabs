#include <stdlib.h>
#include <stdio.h>
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
	Vector v1, v2, temp1, temp2;
	int x, y, z;
	for (int i = 0; i < nv; i++)
	{
		v1 = {0,0,0}, v2  = {0,0,0};
		for (int j = 0; j < nt; j++)
		{
			if (mesh->triangles[j].vInds[0] == i ||
					mesh->triangles[j].vInds[1] == i || 
					mesh->triangles[j].vInds[2] == i ) {
				x = mesh->triangles[j].vInds[0];
				y = mesh->triangles[j].vInds[1];
				z = mesh->triangles[j].vInds[2];
				temp1 = Subtract(mesh->vertices[y], mesh->vertices[x]);
				temp2 = Subtract(mesh->vertices[z], mesh->vertices[x]);
				v1 = CrossProduct(temp1,temp2);
				v2 = Add(v2, v1);
			}
		}
		v2 = Normalize(v2);
		mesh->vnorms[i] = v2;
	}

	mesh->next = *list;
	*list = mesh;	
}