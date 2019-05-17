using namespace std;
#include "Sphere.h"
#include<iostream>


bool Sphere::hit(const Ray & r, HitRec & rec, int i) const {	
	
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

	Vec3f CenToOrigin = r.o - this->c;
	float A = r.d.dot(r.d);
	float B = 2 * (CenToOrigin.dot(r.o));
	float C = CenToOrigin.dot(CenToOrigin) - radSq;
	float D = B*B - 4 * A*C;

	if (D < 0) {
		return false;
	}

	float t1 = (-B + sqrtf(D)) / (2 * A);
	float t2 = (-B - sqrtf(D)) / (2 * A);

	if (t1 > t2) { t1 = t2; }

	if (rec.anyHit && rec.tHit > t1) { rec.tHit = t1; rec.primIndex = i; }
	else if (!rec.anyHit) { rec.tHit = t1; rec.primIndex = i; }
	
	rec.anyHit = true;
	return true;
}


void Sphere::computeSurfaceHitFields(const Ray & r, HitRec & rec) const {
	rec.p = r.o + r.d * rec.tHit;
	rec.n = (rec.p - c).normalize();
}