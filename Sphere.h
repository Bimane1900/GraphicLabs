#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "Vec3.h"
#include "Ray.h"

class Sphere {
public:
	Vec3f c;
	float r;
	Vec3f diffuse = Vec3f(0.6f, 0.6f, 0.6f);
	Vec3f ambient = Vec3f(0.1f, 0.1f, 0.1f);
	Vec3f specular = Vec3f(.5f, .5f, .5f);
	float shiny = 64.0;
public:
	Sphere(const Vec3f & cen, float rad) : c(cen), r(rad) { }
	Sphere(const Vec3f & cen, float rad, Vec3f amb, Vec3f Diff) : c(cen), r(rad), ambient(amb), diffuse(Diff)  { }

	bool hit(const Ray & r, HitRec & rec) const;
	void computeSurfaceHitFields(const Ray & r, HitRec & rec) const;

};

#endif