//#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <glew.h>
#include <freeglut.h>
#include <iostream>
#include <fstream>

#include "algebra.h"
#include "shaders.h"
#include "mesh.h"
void ModMenu(int k);

#define NoOfShaders 3
#define ShaderSize 9000
#define NumLights 1

int screen_width = 1024;
int screen_height = 768;


Mesh *meshList = NULL; // Global pointer to linked list of triangle meshes
Mesh* object = NULL;
Mesh* Cube = NULL;
Camera cam = { {0,0,20}, {0,0,0}, 60, 1, 10000 }; // Setup the global camera parameters


GLuint shprg[NoOfShaders]; // Shader program id

int selectedIndex = -1;


//int Mutplelights = NumLights;

//Models
Vector ambient = { 0.0, 0.5, 0.0 };
Vector diffuse = { 0.0, 0.7, 0.0 };
Vector specular = { 1.0, 1.0, 1.0 };
int shininess = 16;




//Light's 
Vector light = { 10.0, 0.0, 10.0 };//position
Vector Iambient = { 0.2, 0.2, 0.2 };
Vector Idiffuse = { 0.5, 0.5, 0.5 };
Vector Ispecular = {1.0, 1.0, 1.0 };

Vector light2 = { -9.8, 5.4, 2.2 };//position
Vector Iambient2 = { 0.2, 0.2, 0.2 };
Vector Idiffuse2 = { 0.5, 0.5, 0.5 };
Vector Ispecular2 = { 1.0, 1.0, 1.0 };
// Global transform matrices
// V is the view transform
// P is the projection transform
// PV = P * V is the combined view-projection transform
Matrix V, P, PV;


void prepareShaderProgram(char vs_src[ShaderSize], char fs_src[ShaderSize])/*(const char ** vs_src, const char ** fs_src)*/ {
	//printf("inprep vs_src: \n%s\n", vs_src);
	//printf("inprer fs_src: \n%s\n", fs_src);
	selectedIndex++;
	GLint success = GL_FALSE;
	char info[512];
	shprg[selectedIndex] = glCreateProgram();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_src, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	if (!success) {
		printf("Error in vertex shader!\n");
		glGetShaderInfoLog(vs, 512, NULL, info);
		printf("%s\n", info);
	}
	else printf("Vertex shader compiled successfully!\n");

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_src, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if (!success) {
		printf("Error in fragment shader!\n");
		glGetShaderInfoLog(fs, 512, NULL, info);
		printf("%s\n", info);
	}
	else printf("Fragment shader compiled successfully!\n");


	glAttachShader(shprg[selectedIndex], vs);
	glAttachShader(shprg[selectedIndex], fs);
	glLinkProgram(shprg[selectedIndex]);
	GLint isLinked = GL_FALSE;
	glGetProgramiv(shprg[selectedIndex], GL_LINK_STATUS, &isLinked);
	if (!isLinked) {
		printf("Link error in shader program!\n");
		glGetProgramInfoLog(shprg[selectedIndex], 512, NULL, info);
		printf("%s\n", info);
	}
	else printf("Shader program linked successfully!\n");
}

void prepareMesh(Mesh *mesh) {
	int sizeVerts = mesh->nv * 3 * sizeof(float);
	int sizeCols = mesh->nv * 3 * sizeof(float);
	int sizeTris = mesh->nt * 3 * sizeof(int);

	// For storage of state and other buffer objects needed for vertex specification
	glGenVertexArrays(1, &mesh->vao);
	glBindVertexArray(mesh->vao);

	// Allocate VBO and load mesh data (vertices and normals)
	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeVerts + sizeCols, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVerts, (void *)mesh->vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeVerts, sizeCols, (void *)mesh->vnorms);

	// Allocate index buffer and load mesh indices
	glGenBuffers(1, &mesh->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeTris, (void *)mesh->triangles, GL_STATIC_DRAW);

	//Define the format of the vertex data
	GLint vPos = glGetAttribLocation(shprg[selectedIndex], "vPos");
	glEnableVertexAttribArray(vPos);
	glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Define the format of the vertex data 
	GLint vNorm = glGetAttribLocation(shprg[selectedIndex], "vNorm");
	glEnableVertexAttribArray(vNorm);
	glVertexAttribPointer(vNorm, 3, GL_FLOAT, GL_FALSE, 0, (void *)(mesh->nv * 3 * sizeof(float)));

	glBindVertexArray(0);

}

void renderMesh(Mesh *mesh) {

	// Assignment 1: Apply the transforms from local mesh coordinates to world coordinates here
	// Combine it with the viewing transform that is passed to the shader below
	Matrix S = CreateScaling(mesh->scale.x, mesh->scale.y, mesh->scale.z);
	Matrix Rx = CreateRotation(mesh->rotation.x, 'x');
	Matrix Ry = CreateRotation(mesh->rotation.y, 'y');
	Matrix Rz = CreateRotation(mesh->rotation.z, 'z');
	Matrix T = CreateTranslation(mesh->translation);
	Matrix W = MatMatMul(T, MatMatMul(Rx, MatMatMul(Ry, MatMatMul(Rz, S))));
	Matrix M = MatMatMul(PV, W);

	// Pass the viewing transform to the shader
	GLint loc_PV = glGetUniformLocation(shprg[selectedIndex], "PV");
	glUniformMatrix4fv(loc_PV, 1, GL_FALSE, M.e);
	Matrix VM = MatMatMul(V, W);
	GLint loc_W = glGetUniformLocation(shprg[selectedIndex], "VM");
	glUniformMatrix4fv(loc_W, 1, GL_FALSE, W.e);

	//GLint light_LOC = glGetUniformLocation(shprg[selectedIndex], "light");
	//glUniform3f(light_LOC, light.x, light.y, light.z);

	//GLint loc_V = glGetUniformLocation(shprg[selectedIndex], "view");
	//glUniform3f(loc_V, cam.position.x, cam.position.y, cam.position.z);

	// Select current resources 
	glBindVertexArray(mesh->vao);

	// To accomplish wireframe rendering (can be removed to get filled triangles)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Draw all triangles
	glDrawElements(GL_TRIANGLES, mesh->nt * 3, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);

	//viewPos

	GLint loc_viewPos = glGetUniformLocation(shprg[selectedIndex], "viewPos");
	glUniform3f(loc_viewPos, cam.position.x, cam.position.y, cam.position.z);
	

	//light
	GLint loc_lightPostion = glGetUniformLocation(shprg[selectedIndex], "lightP");
	glUniform3f(loc_lightPostion, light.x, light.y, light.z);

	GLint loc_lightAmbient = glGetUniformLocation(shprg[selectedIndex], "Iambient");
	glUniform3f(loc_lightAmbient, Iambient.x, Iambient.y, Iambient.z);

	GLint loc_lightDiffuse = glGetUniformLocation(shprg[selectedIndex], "Idiffuse");
	glUniform3f(loc_lightDiffuse, Idiffuse.x, Idiffuse.y, Idiffuse.z);

	GLint loc_lightSpecular = glGetUniformLocation(shprg[selectedIndex], "Ispecular");
	glUniform3f(loc_lightSpecular, Ispecular.x, Ispecular.y, Ispecular.z);


	//light2
	GLint loc_lightPostion2 = glGetUniformLocation(shprg[selectedIndex], "lightP2");
	glUniform3f(loc_lightPostion2, light2.x, light2.y, light2.z);

	GLint loc_lightAmbient2 = glGetUniformLocation(shprg[selectedIndex], "Iambient2");
	glUniform3f(loc_lightAmbient2, Iambient2.x, Iambient2.y, Iambient2.z);
	
	GLint loc_lightDiffuse2 = glGetUniformLocation(shprg[selectedIndex], "Idiffuse2");
	glUniform3f(loc_lightDiffuse2, Idiffuse2.x, Idiffuse2.y, Idiffuse2.z);

	GLint loc_lightSpecular2 = glGetUniformLocation(shprg[selectedIndex], "Ispecular2");
	glUniform3f(loc_lightSpecular2, Ispecular2.x, Ispecular2.y, Ispecular2.z);


	// material
	GLint loc_meshAmbient = glGetUniformLocation(shprg[selectedIndex], "Matambient");
	glUniform3f(loc_meshAmbient, ambient.x, ambient.y, ambient.z);
	
	GLint loc_meshDiffuse = glGetUniformLocation(shprg[selectedIndex], "Matdiffuse");
	glUniform3f(loc_meshDiffuse, diffuse.x, diffuse.y, diffuse.z);

	GLint loc_meshSpecular = glGetUniformLocation(shprg[selectedIndex], "Matspecular");
	glUniform3f(loc_meshSpecular, specular.x, specular.y, specular.z);
	
	GLint loc_meshShininess = glGetUniformLocation(shprg[selectedIndex], "shininess");
	glUniform1i(loc_meshShininess, shininess);


	GLint loc_multiLights = glGetUniformLocation(shprg[selectedIndex], "Mutplelights");
	glUniform1i(loc_multiLights, NumLights);



}



void display(void) {
	Mesh *mesh;

	glClear(GL_DEPTH_BUFFER_BIT + GL_COLOR_BUFFER_BIT);

	// Assignment 1: Calculate the transform to view coordinates yourself 	
	// The matrix V should be calculated from camera parameters
	// Therefore, you need to replace this hard-coded transform. 
	V = CreateEmptyMatrix();
	Matrix T = CreateTranslation({ -cam.position.x, -cam.position.y, -cam.position.z });
	Matrix Rx = CreateRotation(-cam.rotation.x, 'x');
	Matrix Ry = CreateRotation(-cam.rotation.y, 'y');
	Matrix Rz = CreateRotation(-cam.rotation.z, 'z');
	V = MatMatMul(Rz, MatMatMul(Ry, MatMatMul(Rx, T)));


	// Assignment 1: Calculate the projection transform yourself 	
	// The matrix P should be calculated from camera parameters
	// Therefore, you need to replace this hard-coded transform. 

	//P = OrthogonalProj({ -20,-10,1 }, { 20,10,1000 });
	P = PerspectiveProj2(cam.fov, (float)screen_width / (float)screen_height, cam.nearPlane, cam.farPlane);


	// This finds the combined view-projection matrix
	PV = MatMatMul(P, V);

	// Select the shader program to be used during rendering 
	glUseProgram(shprg[selectedIndex]);

	// Render all meshes in the scene
	mesh = meshList;

	while (mesh != NULL) {
		renderMesh(mesh);
		mesh = mesh->next;
	}
	glFlush();
}

void changeSize(int w, int h) {
	screen_width = w;
	screen_height = h;
	glViewport(0, 0, screen_width, screen_height);

}
void keypress(unsigned char key, int x, int y) {
	//int k, input = 0;
	switch (key) {
	case 'l':
		if (selectedIndex < 2)
		{
			selectedIndex++;
		}
		break;
	case 'L':
		if (selectedIndex > 0)
		{
			selectedIndex--;
		}
		break;
	case 'e':
		light.x += 0.6f;
		//PrintVector((char*)"light", light);
		object->translation.x += 0.6f;
		break;
	case 'E':
		light.x -= 0.6f;
		//PrintVector((char*)"light", light);
		object->translation.x -= 0.6f;
		break;
	case 'w':
		light.y += 0.6f;
		//PrintVector((char*)"light", light);
		object->translation.y += 0.6f;
		break;
	case 'W':
		light.y -= 0.6f;
		//PrintVector((char*)"light", light);
		object->translation.y -= 0.6f;
		break;
	case 'r':
		light.z += 0.6f;
		//PrintVector((char*)"light", light);
		object->translation.z += 0.6f;
		break;
	case 'R':
		light.z -= 0.6f;
		//PrintVector((char*)"light", light);
		object->translation.z -= 0.6f;
		break;
	case 'z':
		cam.position.z -= 0.6f;
		break;
	case 'Z':
		cam.position.z += 0.6f;
		break;
	case 'y':
		cam.position.y += 0.6f;
		break;
	case 'Y':
		cam.position.y -= 0.6f;
		break;
	case 'x':
		cam.position.x += 0.6f;
		break;
	case 'X':
		cam.position.x -= 0.6f;
		break;
	case 'k':
		cam.rotation.y += 0.6f;
		break;
	case 'K':
		cam.rotation.y -= 0.6f;
		break;
	case 'j':
		cam.rotation.z += 0.6f;
		break;
	case 'J':
		cam.rotation.z -= 0.6f;
		break;
	case 'h':
		cam.rotation.x += 0.6f;
		break;
	case 'H':
		cam.rotation.x -= 0.6f;
		break;
	case 'a':
		object->rotation.x -= 1.0;
		break;
	case 'A':
		object->rotation.x += 1.0;
		break;
	case 's':
		object->rotation.y -= 1.0;
		break;
	case 'S':
		object->rotation.y += 1.0;
		break;
	case 'd':
		object->rotation.z -= 1.0;
		break;
	case 'D':
		object->rotation.z += 1.0;
		break;
	case 'o':
		if (object->next == NULL)
		{
			object = meshList;
		}
		else
		{
			object = object->next;
		}
		break;
	case 'O':
		/*fflush(stdin);
		float x, y, z;
		printf("Enter a Vector");
		scanf("%f %f %f", &x,&y,&z);
		object->translation = { x, y, z };
		PrintVector((char*)"OO: ", { x, y, z });*/
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		//k = (int)key - 48;
		ModMenu((int)key - 48);
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}

	glutPostRedisplay();
}

char * readShader(char * filename) {
	FILE * f;
	//const char *shdcode = (char*)calloc(1000, sizeof 'a');
	char code[ShaderSize] = "\0";
	f = fopen(filename, "r");
	if (f == NULL) {
		perror("Error opening file");
		//exit(EXIT_FAILURE);
	}
	char file[199];
	while (fgets(file, 199, f) != NULL)
	{
		strcat(code, file);
	};
	fclose(f);
	return code;

}

void init(void) {
	// Compile and link the given shader program (vertex shader and fragment shader)
	char vsShad[ShaderSize];
	char fsShad[ShaderSize];
	/*strcpy_s(vsShad, readShader((char*)"./GouradVertex.txt"));
	strcpy_s(fsShad, readShader((char*)"./GouradFrag.txt"));
	prepareShaderProgram(vsShad, fsShad);
	strcpy_s(vsShad, readShader((char*)"./PhongVertex.txt"));
	strcpy_s(fsShad, readShader((char*)"./PhongFrag.txt"));
	prepareShaderProgram(vsShad, fsShad);
	strcpy_s(vsShad, readShader((char*)"./CartoonVertex.txt"));
	strcpy_s(fsShad, readShader((char*)"./CartoonFrag.txt"));
	prepareShaderProgram(vsShad, fsShad);*/
	const char* Vert[NoOfShaders] = { "./GouradVertex.txt", "./PhongVertex.txt","./CartoonVertex.txt" };
	const char* Frag[NoOfShaders] = { "./GouradFrag.txt", "./PhongFrag.txt","./CartoonFrag.txt" };
	for (int i = 0; i < NoOfShaders; i++)
	{
		strcpy_s(vsShad, readShader((char*)Vert[i]));
		strcpy_s(fsShad, readShader((char*)Frag[i]));
		prepareShaderProgram(vsShad, fsShad);
	}
	glEnable(GL_DEPTH_TEST);

	// Setup OpenGL buffers for rendering of the meshes
	Mesh * mesh = meshList;
	while (mesh != NULL) {
		prepareMesh(mesh);
		mesh = mesh->next;
	}
}

void cleanUp(void) {
	printf("Running cleanUp function... ");
	// Free openGL resources
	// ...

	// Free meshes
	// ...
	printf("Done!\n\n");
}

// Include data for some triangle meshes (hard coded in struct variables)
#include "./models/mesh_bunny.h"
#include "./models/mesh_cow.h"
#include "./models/mesh_cube.h"
#include "./models/mesh_frog.h"
#include "./models/mesh_knot.h"
#include "./models/mesh_sphere.h"
#include "./models/mesh_teapot.h"
#include "./models/mesh_triceratops.h"

void ModMenu(int k) {
	int input;
	object = meshList;
	float x, y, z;
	for (int i = 1; i < k && object->next != NULL; i++)
	{
		object = object->next;
	}

	printf("%s", object->name);
	printf("\n1. Translation\n2. Scaling\n3. Rotation\n4. Exit\n");
	fflush(stdin);
	scanf("%d", &input);
	switch (input) {
	case 1:
		printf("Enter translation x y z:");
		scanf("%f %f %f", &x, &y, &z);

		object->translation = { x,y,z };
		printf("Tranlation updated\n");
		break;
	case 2:
		printf("Enter scaling x y z:");
		scanf("%f %f %f", &x, &y, &z);
		object->scale = { x,y,z };
		printf("Scale updated\n");
		break;
	case 3:
		printf("Enter rotation x y z axis:");
		scanf("%f %f %f", &x, &y, &z);
		object->rotation = { x,y,z };
		printf("Rotation updated\n");
		break;
	default:
		break;
	}
}

void printMenu()
{
	Mesh* mesh = meshList;
	int n = 1;
	while (mesh != NULL)
	{
		printf("%d. %s \n", n, mesh->name);
		n++;
		mesh = mesh->next;
	}
}

int main(int argc, char **argv) {

	// Setup freeGLUT	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
	glutCreateWindow("DVA338 Programming Assignments");
	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keypress);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// Specify your preferred OpenGL version and profile
	glutInitContextVersion(4, 5);
	//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);	
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// Uses GLEW as OpenGL Loading Library to get access to modern core features as well as extensions
	GLenum err = glewInit();
	if (GLEW_OK != err) { fprintf(stdout, "Error: %s\n", glewGetErrorString(err)); return 1; }

	// Output OpenGL version info
	fprintf(stdout, "GLEW version: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, "OpenGL version: %s\n", (const char *)glGetString(GL_VERSION));
	fprintf(stdout, "OpenGL vendor: %s\n\n", glGetString(GL_VENDOR));

	// Insert the 3D models you want in your scene here in a linked list of meshes
	// Note that "meshList" is a pointer to the first mesh and new meshes are added to the front of the list	
	//insertModel(&meshList, cow.nov, cow.verts, cow.nof, cow.faces, 20.0, (char*)"Cow", { 0,0,0 }, { 0,-45,0 }, { 1,1,1 });
	//insertModel(&meshList, triceratops.nov, triceratops.verts, triceratops.nof, triceratops.faces, 3.0, (char*)"Triceratops", { -25,0,0 }, { 60,0,0 }, { 1,1,1 });
	//insertModel(&meshList, bunny.nov, bunny.verts, bunny.nof, bunny.faces, 60.0, (char*)"Bunny");
	
	//insertModel(&meshList, frog.nov, frog.verts, frog.nof, frog.faces, 2.5,(char*)"Frog");
	insertModel(&meshList, knot.nov, knot.verts, knot.nof, knot.faces, 1.0,(char*)"Knot");
	//insertModel(&meshList, sphere.nov, sphere.verts, sphere.nof, sphere.faces, 12.0,(char*)"Sphere");
	//insertModel(&meshList, teapot.nov, teapot.verts, teapot.nof, teapot.faces, 3.0,(char*)"Teapot");

	insertModel(&meshList, cube.nov, cube.verts, cube.nof, cube.faces, 5.0, (char*)"Cube", light, { 0,0,0 }, { 0.1,0.1,0.1 });
	if (NumLights == 2)
	{
		insertModel(&meshList, cube.nov, cube.verts, cube.nof, cube.faces, 5.0, (char*)"Cube", light2, { 0,0,0 }, { 0.1,0.1,0.1 });
	}
	object = meshList;
	init();
	printMenu();
	glutMainLoop();

	cleanUp();
	return 0;
}
