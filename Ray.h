#ifndef _RAY_H_
#define _RAY_H_

#include "Vec3.h"

class Ray {
public:
	Vec3f o, d;
	float tClip;	
	static float rayEps;
	static float tMax;
	float inMaterial = 1;
public:
	Ray() : tClip(tMax) { }	
	Ray(const Vec3f & o, const Vec3f& d, float tclip = tMax) : o(o), d(d), tClip(tMax) { }
	
	void epsMoveStartAlongSurfaceNormal(const Vec3f & n) { o = o + n * rayEps; }
	void epsMoveStartAlongDir() { o = o + d * rayEps; }
	
	static void incRayEps() { rayEps *= 10.0f; }
	static void decRayEps() { rayEps /= 10.0f; }
};


class HitRec {
public:	
	float tHit;
	float tHit2;
	int primIndex;
	Vec3f p;
	Vec3f p2;
	Vec3f n;
	Vec3f n2;
	bool anyHit;
};


#endif
