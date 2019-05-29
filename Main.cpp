#include<vector>
#include<iostream>
using namespace std;

#include <glut.h>

#include<time.h>

#include "Vec3.h"
#include "Image.h"
#include "Ray.h"
#include "Sphere.h"

#define GLASS 1.0
#define WATER 1.33
#define AIR 1.0
float TEST = 1.0;


class Scene {
public:
	vector<Sphere> spheres;

	Scene(void) {

	}
	void add(Sphere & s) {
		spheres.push_back(s);
		//cout << "Sphere added: " << "r = " << spheres[spheres.size()-1].r << endl;
	}

	void load(char * fileName) {
		// load a file with spheres for your scene here ...
	}

};


void glSetPixel(int x, int y, Vec3f & c) {
	glColor3f(c.x, c.y, c.z);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
}

class Light {
private:
	Vec3f intensity = Vec3f(1.0f, 1.0f, 1.0f);
	Vec3f position = Vec3f(0.0f, 0.0f, -1.0f);

public:
	Vec3f getIntensity() { return this->intensity; }
	void setIntensity(Vec3f I) { this->intensity = I; }
	Vec3f getPosition() { return this->position; }
	void setPosition(Vec3f pos) { this->position = pos; }
	void setX(float x) { this->position.x = x; }
	void setY(float y) { this->position.y = y; }
	void setZ(float z) { this->position.z = z; }
	float getX() { return this->position.x; }
	float getY() { return this->position.y; }
	float getZ() { return this->position.z; }
};


class Camera {
private:
	Vec3f position = Vec3f(0.0f, 0.0f, -1.0f);

public:
	void setPosition(Vec3f pos) { this->position = pos; }
	void setX(float x) { this->position.x = x; }
	void setY(float y) { this->position.y = y; }
	void setZ(float z) { this->position.z = z; }
	Vec3f getPosition() { return this->position; }
	float getX() { return this->position.x; }
	float getY() { return this->position.y; }
	float getZ() { return this->position.z; }
};


Camera cam;
Light light;
Sphere li = Sphere(light.getPosition(), 0.5f, Vec3f(1.0f, 1.0f, 1.0f), Vec3f(0.0f, 0.0f, 0.0f));
float iorX = 1.0;




class SimpleRayTracer {
private:
	Image * image;
	int reflectLimit = 4;
	int reflectCount = 0;

	Vec3f getEyeRayDirection(int x, int y) {
		//Uses a fix camera looking along the negative z-axis
		static float z = -5.0f;		
		static float sizeX = 4.0f; 
		static float sizeY = 3.0f; 

		
		static float left = -sizeX * 0.5f;
		static float bottom = -sizeY *0.5f;
		static float dx =  sizeX / float(image->getWidth());  
		static float dy =  sizeY / float(image->getHeight());

		return Vec3f(left + x * dx, bottom + y * dy, /*cam.getZ()*/z).normalize();

		//int fov = 45;
		//float M_PI = 3.14159;
		//float ImageAspectRatio = float(image->getWidth()) / float(image->getHeight());
		//float Px = (2 * ((x + 0.5) / float(image->getWidth())) - 1) * tan(fov / 2 * M_PI / 180) * ImageAspectRatio;
		//float Py = (1 - 2 * ((y + 0.5) / float(image->getHeight()))) * tan(fov / 2 * M_PI / 180);
		//Vec3f rayOrigin(0.0, 0.0, 0.0);
		//Vec3f rayDirection = Vec3f(Px, Py, -1.0) - rayOrigin; // note that this just equal to Vec3f(Px, Py, -1); 
		//return rayDirection.normalize();

	}


public:

	Scene * scene;

	int rayCount = 0;

	SimpleRayTracer(Scene * scene, Image * image) {
		this->scene = scene;
		this->image = image;
	}



	float max(float x, float y) { return x < y ? y : x; }
	float min(float x, float y) { return x > y ? y : x; }

	Vec3f clamp(Vec3f color) {	
		color.x = max(0.0, color.x);
		color.y = max(0.0, color.y);
		color.z = max(0.0, color.z);
		color.x = min(1.0, color.x);
		color.y = min(1.0, color.y);
		color.z = min(1.0, color.z);
		return color;
	}

	//Returns Phong without ambient
	Vec3f phong(Ray  ray, HitRec hitRec) {
		Sphere s = scene->spheres[hitRec.primIndex];
		Vec3f L = (light.getPosition() - hitRec.p).normalize();
		Vec3f diffuse = light.getIntensity().multCoordwise(s.diffuse) * max(hitRec.n.dot(L), 0.0);
		Vec3f R = (hitRec.n * (L.dot(hitRec.n)) * 2.0 - L).normalize();
		Vec3f V = (cam.getPosition() - hitRec.p).normalize();
		Vec3f specular = light.getIntensity().multCoordwise(s.specular) * pow(max((R.dot(V)), 0.0), s.shiny);
		return (diffuse + specular) * (1.0 - s.reflection);
	}

	void searchClosestHit(const Ray & ray, HitRec & hit_rec) {
		hit_rec.anyHit = false;
		for (int i = 0; i < scene->spheres.size(); i++) {
			if (scene->spheres[i].hit(ray, hit_rec))
				hit_rec.primIndex = i;
			rayCount++;
		}
		if (hit_rec.anyHit)
			scene->spheres[hit_rec.primIndex].computeSurfaceHitFields(ray, hit_rec);
	}


	Vec3f refraction(Ray &I, const HitRec &N, Sphere s) {//I = Ray.d, N = Hitrec.n
		//Put this in a function and check hit
		float ior = iorX; //Index of Refraction
		Vec3f t;
		Vec3f color = Vec3f(0.0, 0.0, 0.0);

		float DdotN = max(-1.0, I.d.dot(N.n));
		DdotN = min(1.0, DdotN);


		float cosi = DdotN;
		float etai = 1, etat = ior;
		Vec3f n = N.n;
		if (cosi < 0) {
			cosi = cosi;
		}
		else {
			std::swap(etai, etat);
			n = -n;
		}
		float eta = etai / etat;
		float k = 1 - eta * eta * (1 - cosi * cosi);
		if (k < 0)
		{
			t = I.d;
		}
		else
		{
			t = (I.d * eta) + (n * (eta * cosi - sqrtf(k)));

		}


		Ray refractRay;
		refractRay.d = t.normalize();
		refractRay.o = N.p + (t*I.rayEps);
		HitRec refractHit;

		s.hit(refractRay, refractHit);
		s.computeSurfaceHitFields(refractRay, refractHit);

		Ray outindex;
		HitRec outHit;
		outindex.o = refractHit.p2 + (I.d * I.rayEps);
		outindex.d = I.d;


		//Shoot ray!
		return raytrace(outindex, outHit);

	}

	Vec3f raytrace(Ray ray, HitRec & hitrec) {

		Vec3f color = Vec3f(0.0, 0.0, 0.0);
		hitrec.anyHit = false;
		searchClosestHit(ray, hitrec);
		if (hitrec.anyHit) {
			return lightning(ray, hitrec, color);
		}
		else {
			return Vec3f(0.0, 0.0, 0.0);
		}
	}



	Vec3f lightning(Ray ray, HitRec  hitRec, Vec3f  color) {
		Sphere s = scene->spheres[hitRec.primIndex];

		//Do refractions seperately
		if (s.trans == true) {
			singleray++;
			color += refraction(ray, hitRec, s);
		}
		
		Vec3f ambient = light.getIntensity().multCoordwise(s.ambient) * (1.0 - s.reflection);
		color += ambient;

		//Prepare shadowray
		HitRec ShadowR;
		Ray ShadowRay;
		Vec3f LDir = (light.getPosition() - hitRec.p).normalize();
		ShadowRay.d = LDir;
		ShadowRay.o = hitRec.p + (ShadowRay.d * ray.rayEps);
		ShadowR.anyHit = false;

		searchClosestHit(ShadowRay, ShadowR);
		if (ShadowR.anyHit == false)
		{
			color += phong(ray, hitRec);
		}
		//Handle shadowrays hit differently when hitting glass ball(spheres defaulted to glass)
		else if (ShadowR.anyHit == true && scene->spheres[ShadowR.primIndex].trans == true) {
			color += phong(ray, hitRec)*0.8;
		}

		//keep counter on reflections
		if (reflectCount < reflectLimit) {		
			color += Reflection(ray, hitRec, s);
		}

		return color;
	}


	Vec3f Reflection(Ray ray, HitRec hitRec, Sphere s) {
		reflectCount++;

		//Reflection vector
		Vec3f R = (ray.d - (hitRec.n * (ray.d.dot(hitRec.n)) * 2.0)).normalize();

		//Prepare reflection ray
		Ray reflect;
		HitRec Ref;
		reflect.o = hitRec.p + (R * ray.rayEps);
		reflect.d = R;

		//Shoot ray!
		Vec3f color2 = raytrace(reflect, Ref).multCoordwise(s.specular);

		return color2 * s.reflection;
	}

	int singleray = 0;
	int fewtimes = 0;
	void fireRays(void) {
		Ray ray;
		HitRec hitRec;
		ray.o = cam.getPosition();//Set the start position of the eye rays to the origin
		for (int y = 0; y < image->getHeight(); y++) {
			for (int x = 0; x < image->getWidth(); x++) {
				reflectCount = 0;
				ray.d = getEyeRayDirection(x, y);
				Vec3f color = Vec3f(0.0f, 0.0f, 0.0f);

				color = raytrace(ray, hitRec);

				if (singleray > 2)
				{
					fewtimes = fewtimes < singleray ? singleray : fewtimes;
					singleray = 0;
				}

				color = clamp(color);
				image->setPixel(x, y, color);
				glSetPixel(x, y, color);
			}
		}
		printf("Max Refraction hit: %d \n", fewtimes);
	}
};


SimpleRayTracer * rayTracer;

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	time_t timer = time(NULL);
	rayTracer->rayCount = 0;
	rayTracer->fireRays();
	timer = time(NULL) - timer;
	cout << "Time: " << timer << " Rays: " << rayTracer->rayCount << endl;
	//rayTracer->scene->spheres[3].c.x = (sin(time(NULL)));
	//rayTracer->scene->spheres[3].c.y = (cos(time(NULL)));
	//rayTracer->scene->spheres[2].c.y = (sin(time(NULL)));

	glFlush();
}
void keypress(unsigned char key, int x, int y)
{

	float speed = 0.1f;
	switch (key) {
	case 't':
		TEST += 0.1;
		/*while (true)
		{
			display();
		}*/
		break;
	case 'T':
		TEST -= -0.1;
		break;
	case 'x':
		cam.setX(cam.getX() + speed);
		break;
	case 'X':
		cam.setX(cam.getX() - speed);
		break;

	case 'y':
		cam.setY(cam.getY() + speed);
		break;
	case 'Y':
		cam.setY(cam.getY() - speed);
		break;

	case 'z':
		cam.setZ(cam.getZ() + speed);
		break;
	case 'Z':
		cam.setZ(cam.getZ() - speed);
		break;

	case 'a':
		light.setX(light.getX() + speed);
		li.c.x += speed;
		break;
	case 'A':
		light.setX(light.getX() - speed);
		li.c.x -= speed;
		break;

	case 'w':
		li.c.y += speed;
		light.setY(light.getY() + speed);
		break;
	case 'W':
		li.c.y -= speed;
		light.setY(light.getY() - speed);
		break;

	case 'r':
		li.c.z += speed;
		light.setZ(light.getZ() + speed);
		break;
	case 'R':
		li.c.z -= speed;
		light.setZ(light.getZ() - speed);
		break;

	case 'k':
		iorX += 0.05;
		printf("IORX: %f\n", iorX);
		break;
	case 'K':
		iorX -= 0.05;
		printf("IORX: %f\n",iorX);
		break;


	case 'Q':
	case 'q':
		//glutLeaveMainLoop();
		break;

	}
	glutPostRedisplay();
	//display();
}

void changeSize(int w, int h) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glViewport(0, 0, w, h);
}

void init(void)
{

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutCreateWindow("SimpleRayTracer");
	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keypress);

	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

	Scene * scene = new Scene;
	//light.setPosition(cam.getPosition());
	//li.c = light.getPosition() + Vec3f(0.3f, 0.3f, -0.3f);
	//li.r = 0.1f;
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -10.0f), 2.0f));
	//scene->add((Sphere&)Sphere(Vec3f(5.0f, 0.0f, -10.0f), 2.0f));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -6.0f), 0.5f));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -10.0f), 2.0f));
	//scene->add((Sphere&)Sphere(Vec3f(1.5f, 1.5f, -8.0f), .5f,Vec3f(0.0f,.1f,0.0f), Vec3f(0.0f,0.8f,0.0f)));

	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -8.0f), .5f, Vec3f(0.1f, 0.0f, 0.0f), Vec3f(0.8f, 0.0f, 0.0f)));

	scene->add((Sphere&)Sphere(Vec3f(.0f, 0.0f, -5.5f), 0.7f, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 0.0f), true, 1.0f));
	scene->add((Sphere&)Sphere(Vec3f(.0f, 0.5f, -7.5f), 0.3f, Vec3f(0.3f, 0.0f, 0.0f), Vec3f(0.5f, 0.0f, 0.f)));
	scene->add((Sphere&)Sphere(Vec3f(-1.0f, -1.0f, -4.0f), 0.4f, Vec3f(0.3f, 0.0f, 0.0f), Vec3f(0.6f, 0.0f, 0.0f)));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, -.5f, -7.0f), 0.5f, Vec3f(0.0f, 0.0f, 0.6f), Vec3f(1.0f, 0.0f, 0.0f),false,1.0f));
	scene->add((Sphere&)Sphere(Vec3f(1.0f, -.5f, -7.0f), 0.5f, Vec3f(0.0f, 0.0f, 0.5f), Vec3f(0.0f, 0.0f, 0.5f)));
	//scene->add((Sphere&)Sphere(Vec3f(1.0f, 1.0f, -7.0f), 0.5f, Vec3f(0.1f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 0.9f)));
	//scene->add((Sphere&)Sphere(Vec3f(1.0f, -1.0f, -7.0f), 0.5f, Vec3f(0.1f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 0.9f)));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, -1.0f, -7.0f), 0.5f, Vec3f(0.1f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 0.9f)));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 1.0f, -7.0f), 0.5f, Vec3f(0.1f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 0.9f)));
	//scene->add((Sphere&)Sphere(Vec3f(-1.0f, 0.0f, -7.0f), 0.5f, Vec3f(0.1f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 0.9f)));
	scene->add((Sphere&)Sphere(Vec3f(-1.0f, 0.0f, -5.5f), 0.2f, Vec3f(1.0f, 1.0f, 1.0f), Vec3f(1.0f, 1.0f, 1.0f)));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -6.5f), 0.5f, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 0.0f), true));
	//scene->add((Sphere&)Sphere(Vec3f(.5f, -.5f, -5.0f), 0.6f, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 0.6f), true, 0.0f));
	//scene->add((Sphere&)Sphere(Vec3f(.5f, -.5f, -6.0f), 0.2f, Vec3f(0.2f, 0.0f, 0.0f), Vec3f(0.8f, 0.0f, 0.0f)));
	//scene->add((Sphere&)Sphere(Vec3f(-0.5f, -0.5f, -5.0f), 0.5f, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 0.0f), true));
	scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -10.0f), 2.0f, Vec3f(0.0f, 0.2f, 0.0f), Vec3f(0.0f, 0.8f, 0.f)));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -1500.0f), 900.0f));

	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -17.0f), 8.0f));
	//scene->add(li);

	Image * image = new Image(640, 480);

	rayTracer = new SimpleRayTracer(scene, image);

}

void main(int argc, char **argv) {
	glutInit(&argc, argv);
	init();
	glutMainLoop();
}