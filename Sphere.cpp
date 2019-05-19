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

	if (D < 0 || D != D) {
		return false;
	}

	float t1 = (-B + sqrtf(D)) / (2 * A);
	float t2 = (-B - sqrtf(D)) / (2 * A);
	float t;

	if (t1 < 0 && t2 < 0) {
		return false;
	}

	//cout << "t1: " << t1 << " t2: " << t2 << endl;

	if (t1 < 0) {
		t = t2; 
	}
	else if (t1 < t2) {
		t = t1;
	}
	else {
		
		t = t2;
	}

	if (t <= 0) return false;
	//cout << "t: " << t << endl;



	if (rec.anyHit && rec.tHit > t) { rec.tHit = t; rec.primIndex = i; }
	else if (!rec.anyHit) { rec.tHit = t; rec.primIndex = i; }
	/*if (A != A || B != B || C != C || D != D || CenToOrigin.x != CenToOrigin.x)
		cout << "A: " << A << " B: " << B << " C: " << C << " D: " << D << " Cen.x: " << CenToOrigin.x <<
			" V: " << v.x << " c: " << c.x << " r.o.x: " << r.o.x << endl;*/
	if(rec.tHit <= 0) cout << "tHit: " << rec.tHit << endl;
	rec.anyHit = true;
	return true;
}


void Sphere::computeSurfaceHitFields(const Ray & r, HitRec & rec) const {
	rec.p = r.o + r.d * rec.tHit;
	rec.n = (rec.p - c).normalize();
}
