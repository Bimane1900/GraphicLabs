#define _USE_MATH_DEFINES // To get M_PI defined
#include <math.h>
#include <stdio.h>
#include "algebra.h"

#define PI 3.14159265

Vector CrossProduct(Vector a, Vector b) {
	Vector v = { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
	return v;
}

float DotProduct(Vector a, Vector b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vector Subtract(Vector a, Vector b) {
	Vector v = { a.x-b.x, a.y-b.y, a.z-b.z };
	return v;
}    

Vector Add(Vector a, Vector b) {
	Vector v = { a.x+b.x, a.y+b.y, a.z+b.z };
	return v;
}    

float Length(Vector a) {
	return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

Vector Normalize(Vector a) {
	float len = Length(a);
	Vector v = { a.x/len, a.y/len, a.z/len };
	return v;
}

Vector ScalarVecMul(float t, Vector a) {
	Vector b = { t*a.x, t*a.y, t*a.z };
	return b;
}

HomVector MatVecMul(Matrix a, Vector b) {
	HomVector h;
	h.x = b.x*a.e[0] + b.y*a.e[4] + b.z*a.e[8] + a.e[12];
	h.y = b.x*a.e[1] + b.y*a.e[5] + b.z*a.e[9] + a.e[13];
	h.z = b.x*a.e[2] + b.y*a.e[6] + b.z*a.e[10] + a.e[14];
	h.w = b.x*a.e[3] + b.y*a.e[7] + b.z*a.e[11] + a.e[15];
	return h;
}

Vector Homogenize(HomVector h) {
	Vector a;
	if (h.w == 0.0) {
		fprintf(stderr, "Homogenize: w = 0\n");
		a.x = a.y = a.z = 9999999;
		return a;
	}
	a.x = h.x / h.w;
	a.y = h.y / h.w;
	a.z = h.z / h.w;
	return a;
}

Matrix MatMatMul(Matrix a, Matrix b) {
	Matrix c;
	int i, j, k;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			c.e[j*4+i] = 0.0;
			for (k = 0; k < 4; k++)
				c.e[j*4+i] += a.e[k*4+i] * b.e[j*4+k];
		}
	}
	return c;
}

void PrintVector(char *name, Vector a) {
	printf("%s: %6.5lf %6.5lf %6.5lf\n", name, a.x, a.y, a.z);
}

void PrintHomVector(char *name, HomVector a) {
	printf("%s: %6.5lf %6.5lf %6.5lf %6.5lf\n", name, a.x, a.y, a.z, a.w);
}

Matrix CreateEmptyMatrix()
{
	Matrix c;
	for (int i = 0; i < 16; i++)
	{
		c.e[i] = 0;
	}
	c.e[0] = 1;
	c.e[5] = 1; c.e[10] = 1; c.e[15] = 1;
	return c;
}

Matrix CreateTranslation(Vector v)
{
	Matrix m = CreateEmptyMatrix();
	m.e[12] = v.x;
	m.e[13] = v.y;
	m.e[14] = v.z;
	return m;
}

Matrix CreateScaling(float x, float y, float z)
{
	Matrix c = CreateEmptyMatrix();
	c.e[0] = x;
	c.e[5] = y;
	c.e[10] = z;
	return c;
}

Matrix CreateRotation(float theta, char a)
{
	Matrix c = CreateEmptyMatrix();
	switch (a){
		case 'x': 
			
			c.e[5] = cos(theta*PI / 180);
			c.e[6] = sin(theta*PI / 180);
			c.e[9] = -sin(theta*PI / 180);
			c.e[10] = cos(theta*PI / 180);
			break;
		case 'y':
			c.e[0] = cos(theta*PI / 180);
			c.e[2] = -sin(theta*PI / 180);
			c.e[8] = sin(theta*PI / 180);
			c.e[10] = cos(theta*PI / 180);
			break;
		case 'z':
			c.e[0] = cos(theta*PI / 180);
			c.e[1] = sin(theta*PI / 180);
			c.e[4] = -sin(theta*PI / 180);
			c.e[5] = cos(theta*PI / 180);

			break;
	}
	

	return c;
}

Matrix OrthogonalProj(Vector close, Vector Far)
{
	Matrix m = CreateEmptyMatrix();
	m.e[0] = 2 / (Far.x - close.x);
	m.e[5] = 2 / (Far.y - close.y);
	m.e[10] = 2 / (close.z - Far.z);
	m.e[12] = -((Far.x + close.x) / (Far.x - close.x));
	m.e[13] = -((Far.y + close.y) / (Far.y - close.y));
	m.e[14] = -((Far.z + close.z) / (Far.z - close.z));
	return m;
}

Matrix PerspectiveProj2(float fovy, int aspect, float Near, float Far)
{
	Matrix m = CreateEmptyMatrix();
	m.e[0] = ((1 / tan(fovy / 2 * PI / 90))) / aspect;
	m.e[5] = (1 / tan(fovy / 2 * PI / 90));
	m.e[10] = (Far + Near) / (Near - Far);
	m.e[11] = -1;
	m.e[14] = (2 * Far*Near) / (Near - Far);
	m.e[15] = 0;
	return m;
}


void PrintMatrix(char *name, Matrix a) { 
	int i,j;

	printf("%s:\n", name);
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			printf("%6.5lf ", a.e[j*4+i]);
		}
		printf("\n");
	}
}


