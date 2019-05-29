using namespace std;
#include "Sphere.h"
#include<iostream>


bool Sphere::hit(const Ray & r, HitRec & rec) const {	
	
	Vec3f v = c - r.o;
	float s = v.dot(r.d);
	float vLenSq = v.dot(v);
	float radSq = this->r * this->r;
	//case if looking away from sphere center but we are inside sphere
	if (s < 0 && vLenSq > radSq) {
		return false;
	}
	float mSq = vLenSq - s * s;
	//case if the ray missed the sphere, mSq = distance from center to ray squared
	if (mSq > radSq) {
		return false;
	}

	/*
		Formula to test intersection
	*/
	Vec3f CenToOrigin = r.o - this->c;
	float A = r.d.dot(r.d);
	float B = 2 * (CenToOrigin.dot(r.d));
	float C = CenToOrigin.dot(CenToOrigin) - radSq;
	float D = B * B - 4 * A*C;

	if (D < 0) {
		return false;
	}

	float t1 = (-B + sqrtf(D)) / (2 * A);
	float t2 = (-B - sqrtf(D)) / (2 * A);

	const float t = (t1 < t2)  ? t1 : t2;

	if (!rec.anyHit || rec.tHit > t) {
	
		rec.tHit = t;
		rec.tHit2 = (t1 > t2) ? t1 : t2;
		rec.anyHit = true;
		return true;
	}
	return false;
}


void Sphere::computeSurfaceHitFields(const Ray & r, HitRec & rec) const {
	rec.p2 = r.o + r.d * rec.tHit2;
	rec.p = r.o + r.d * rec.tHit;
	rec.n = (rec.p - c).normalize();
	rec.n2 = (rec.p2 - c).normalize();
}
