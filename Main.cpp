#include<vector>
#include<iostream>
using namespace std;

#include <glut.h>

#include "Vec3.h"
#include "Image.h"
#include "Ray.h"
#include "Sphere.h"


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
	glColor3f(c.r, c.g, c.b);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
}

class Light {
private:
	Vec3f intensity = Vec3f(1.0, 1.0, 1.0);
	Vec3f position = Vec3f(0.0, 0.0, -1.0);

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
	Vec3f position = Vec3f(0.0f, 0.0f, -2.0f);

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
Sphere li = Sphere(light.getPosition(), 0.5f, Vec3f(0.0,0.3,0.0),Vec3f(0.0,0.8,0.0));



class SimpleRayTracer {
private: 
	Scene * scene;
	Image * image;

	Vec3f getEyeRayDirection(int x, int y) {
		//Uses a fix camera looking along the negative z-axis
		//static float z = -5.0f;		
		static float sizeX = 4.0f; 
		static float sizeY = 3.0f; 
		
		static float left = -sizeX * 0.5f;
		static float bottom = -sizeY *0.5f;
		static float dx =  sizeX / float(image->getWidth());  
		static float dy =  sizeY / float(image->getHeight());
	
		return Vec3f(left + x * dx, bottom + y * dy, cam.getZ()).normalize();
	}


public:
	SimpleRayTracer(Scene * scene, Image * image) {
		this->scene = scene;
		this->image = image;
	}

	void searchClosestHit(const Ray & ray, HitRec & hitRec) {
		for (int i = 0; i < scene->spheres.size(); i++) {
			scene->spheres[i].hit(ray, hitRec, i);
		}
		if(hitRec.anyHit)
			scene->spheres[hitRec.primIndex].computeSurfaceHitFields(ray, hitRec);
	}

	float max(float x, float y) { return x < y ? y : x; }

	Vec3f phong(Ray  ray, HitRec  hitRec) {
		Sphere s = scene->spheres[hitRec.primIndex];
		//s.computeSurfaceHitFields(ray, hitRec);
		//Vec3f ambient = light.getIntensity().multCoordwise(s.ambient);
		Vec3f L = (light.getPosition() - hitRec.p).normalize();
		Vec3f diffuse = light.getIntensity().multCoordwise(s.diffuse) * max(hitRec.n.dot(L),0.0);
		
		Vec3f R = (hitRec.n * (L.dot(hitRec.n)) * 2.0 - L).normalize();
		Vec3f V = (cam.getPosition() - hitRec.p).normalize();
		Vec3f specular = light.getIntensity().multCoordwise(s.specular) * pow(max((R.dot(V)),0.0), s.shiny);
		return (diffuse + specular);
	}

	Vec3f lightning(Ray ray, HitRec hitRec){
		Vec3f color;
		HitRec HtR;
		Sphere s = scene->spheres[hitRec.primIndex];
		//s.computeSurfaceHitFields(ray, hitRec);
		Vec3f ambient = light.getIntensity().multCoordwise(s.ambient);
		color = ambient;

		Ray ShadowRay;
		ShadowRay.o = hitRec.p;
		ShadowRay.epsMoveStartAlongSurfaceNormal(hitRec.n);
		Vec3f LDir = (light.getPosition() - ShadowRay.o);
		ShadowRay.d = LDir.normalize();
		HtR.anyHit = false;
		searchClosestHit(ShadowRay, HtR);
		if (HtR.anyHit == false)
		{
			color += phong(ray, hitRec);
		}		
		


		return color;		
	}

	void fireRays(void) { 
		Ray ray;
		HitRec hitRec;
		//bool hit = false;
		ray.o = cam.getPosition();//Vec3f(0.0f, 0.0f, 0.0f); //Set the start position of the eye rays to the origin
		
		for (int y = 0; y < image->getHeight(); y++) {
			for (int x = 0; x < image->getWidth(); x++) {
				ray.d = getEyeRayDirection(x, y);
				hitRec.anyHit = false;
				searchClosestHit(ray, hitRec);
				Vec3f color = Vec3f(0.0f, 0.0f, 0.0f);
				if (hitRec.anyHit) {
					color = lightning(ray,hitRec);	
					//color = phong(ray, hitRec);
					image->setPixel(x, y, color);
					glSetPixel(x, y, color);
				} else {
					image->setPixel(x, y, Vec3f(0.0f, 0.0f, 0.0f));
					glSetPixel(x, y, (Vec3f &)Vec3f(0.0f, 0.0f, 0.0f));
				}
			}
		}
	}
};


SimpleRayTracer * rayTracer;

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	rayTracer->fireRays();
	

	glFlush();
}

void keypress(unsigned char key, int x, int y)
{
	switch (key) {
	case 'x':
		cam.setX(cam.getX() + 0.1f);
		break;
	case 'X':
		cam.setX(cam.getX() - 0.1f);
		break;

	case 'y':
		cam.setY(cam.getY() + 0.1f);
		break;
	case 'Y':
		cam.setY(cam.getY() - 0.1f);
		break;

	case 'z':
		cam.setZ(cam.getZ() + 0.1f);
		break;
	case 'Z':
		cam.setZ(cam.getZ() - 0.1f);
		break;

	case 'a':
		light.setX(light.getX() + 0.1f);
		li.c.x += 0.1f;
		break;
	case 'A':
		light.setX(light.getX() - 0.1f);
		li.c.x -= 0.1f;
		break;

	case 'w':
		li.c.y + 0.1f;
		light.setY(light.getY() + 0.1f);
		break;
	case 'W':
		li.c.y + -0.1f;
		light.setY(light.getY() - 0.1f);
		break;

	case 'r':
		li.c.z += 0.1f;
		light.setZ(light.getZ() + 0.1f);
		break;
	case 'R':
		li.c.z -= 0.1f;
		light.setZ(light.getZ() - 0.1f);
		break;



	case 'Q':
	case 'q':
		//glutLeaveMainLoop();
		break;

	}
	display();
}

void changeSize(int w, int h) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glViewport(0,0,w,h);
}

void init(void)
{
	
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(640, 480);
	glutCreateWindow("SimpleRayTracer");
	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keypress);

	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

	Scene * scene = new Scene;
	//li.c = light.getPosition();
	//li.r = 0.5f;
	scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -10.0f), 2.0f));
	scene->add((Sphere&)Sphere(Vec3f(5.0f, 0.0f, -10.0f), 2.0f));
	scene->add((Sphere&)Sphere(Vec3f(-5.0f, 0.0f, -10.0f), 2.0f));
	scene->add((Sphere&)Sphere(Vec3f(0.0f, 5.0f, -10.0f), 2.0f));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -30.0f), 20.0f));
	//scene->add(li);

	Image * image = new Image(640, 480);	
	
	rayTracer = new SimpleRayTracer(scene, image);

}

void main(int argc, char **argv) {
	glutInit(&argc, argv);
	init();
	glutMainLoop();
}
