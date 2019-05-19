#include<vector>
#include<iostream>
using namespace std;

#include <glut.h>

#include<time.h>

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
	Vec3f intensity = Vec3f(1.0f, 1.0f, 1.0f);
	Vec3f position = Vec3f(-1.0f, -3.0f, -6.0f);

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
Sphere li = Sphere(light.getPosition(), 0.5f, Vec3f(1.0f,1.0f, 1.0f),Vec3f(0.0f, 0.0f, 0.0f));




class SimpleRayTracer {
private: 
	Scene * scene;
	Image * image;
	int reflectDepth = 0;
	int reflectLimit = 3;
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
	}


public:

	int rayCount = 0;

	SimpleRayTracer(Scene * scene, Image * image) {
		this->scene = scene;
		this->image = image;
	}

	void searchClosestHit(const Ray & ray, HitRec & hitRec) {
		for (int i = 0; i < scene->spheres.size(); i++) {
			rayCount++;
			scene->spheres[i].hit(ray, hitRec, i);
		}
		if(hitRec.anyHit)
			scene->spheres[hitRec.primIndex].computeSurfaceHitFields(ray, hitRec);
	}

	float max(float x, float y) { return x < y ? y : x; }

	Vec3f phong(Ray  ray, HitRec hitRec) {
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

	Vec3f lightning(Ray ray, HitRec  hitRec, Vec3f  color){
		Sphere s = scene->spheres[hitRec.primIndex];
		Vec3f ambient = light.getIntensity().multCoordwise(s.ambient);
		color += ambient;

		HitRec ShadowR;
		Ray ShadowRay;
		ShadowRay.o = hitRec.p;// +Vec3f(ShadowRay.rayEps, ShadowRay.rayEps, ShadowRay.rayEps);
		
		//ShadowRay.incRayEps();
		//ShadowRay.epsMoveStartAlongSurfaceNormal(hitRec.n);
		
		//if(hitRec.p.x  != hitRec.p.x)
		//cout << "hitRec.p: " << hitRec.p.x << endl;
		Vec3f LDir = (light.getPosition() - hitRec.p).normalize();
		ShadowRay.d = LDir;
		ShadowRay.epsMoveStartAlongDir();
		ShadowR.anyHit = false;
		searchClosestHit(ShadowRay, ShadowR);
		if (ShadowR.anyHit == false)
		{
			color += phong(ray, hitRec);
		}		

		if (reflectCount < reflectLimit) {
			reflectCount++;
			Vec3f View = ray.d;
			//Vec3f R = (-LDir + (hitRec.n * (LDir.dot(hitRec.n)) * 2.0)).normalize();
			Vec3f R = (View - (hitRec.n * (View.dot(hitRec.n)) * 2.0)).normalize();
			//Vec3f R = (hitRec.n* 2.0* ray.d.dot(hitRec.n) - ray.d).normalize();
			Ray reflect;
			HitRec Ref;
			reflect.o = hitRec.p;
			reflect.epsMoveStartAlongSurfaceNormal(hitRec.n);
			
			reflect.d = R;
			
			reflect.epsMoveStartAlongDir();
			R = raytrace(reflect, Ref) *
				pow(max(reflect.d.dot(ray.d),0.0f), s.shiny);
			if (R.x < 0.0) R.x = 0.0;
			if (R.x > 1.0) R.x = 1.0;
			if (R.y < 0.0) R.y = 0.0;
			if (R.y > 1.0) R.y = 1.0;
			if (R.z < 0.0) R.z = 0.0;
			if (R.z > 1.0) R.z = 1.0;
			//cout << "R.x: " << R.x << " R.y: " << R.y << " R.z: " << R.z << endl;
			color = color + R;
			/*color += raytrace(reflect, Ref) *
				pow(reflect.d.dot(ray.d), s.shiny);*/
		}
		else {
			//cout << "ray reflect: " << reflectCount << endl;
		}


		return color;		
	}

	Vec3f raytrace(Ray ray, HitRec & hitrec) {
		Vec3f color = Vec3f(0.0, 0.0, 0.0);
		hitrec.anyHit = false;
		//if (ray.o.x != ray.o.x) cout << ray.o.x << endl;
		searchClosestHit(ray, hitrec);
		if (hitrec.anyHit) {
			return lightning(ray, hitrec, color);
			//return phong(ray, hitrec);
		}
		else {
			return Vec3f(0.0, 0.0, 0.0);
		}
	}

	void fireRays(void) { 
		Ray ray;
		HitRec hitRec;
		//bool hit = false;
		ray.o = cam.getPosition();//Vec3f(0.0f, 0.0f, 0.0f); //Set the start position of the eye rays to the origin
		
		for (int y = 0; y < image->getHeight(); y++) {
			for (int x = 0; x < image->getWidth(); x++) {
				reflectCount = 0;
				ray.d = getEyeRayDirection(x, y);
				Vec3f color = Vec3f(0.0f, 0.0f, 0.0f);
				color = raytrace(ray,hitRec);
				image->setPixel(x, y, color);
				glSetPixel(x, y, color);
				//hitRec.anyHit = false;
				//searchClosestHit(ray, hitRec);
				//if (hitRec.anyHit) {
				//	color = lightning(ray,hitRec);	
				//	//color = phong(ray, hitRec);
				//	image->setPixel(x, y, color);
				//	glSetPixel(x, y, color);
				//} else {
				//	image->setPixel(x, y, Vec3f(0.0f, 0.0f, 0.0f));
				//	glSetPixel(x, y, (Vec3f &)Vec3f(0.0f, 0.0f, 0.0f));
				//}
			}
		}
	}
};


SimpleRayTracer * rayTracer;

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT);
	//glTranslatef(cam.getX(), cam.getY(), cam.getZ());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	time_t timer = time(NULL);
	rayTracer->rayCount = 0;
	rayTracer->fireRays();
	timer = time(NULL) - timer;
	cout << "Time: " << timer << " Rays: " << rayTracer->rayCount << endl;
	

	glFlush();
}

void keypress(unsigned char key, int x, int y)
{
	float speed = 0.1f;
	switch (key) {
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
	//light.setPosition(cam.getPosition());
	//li.c = light.getPosition() + Vec3f(0.3f, 0.3f, -0.3f);
	//li.r = 0.1f;
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -10.0f), 2.0f));
	//scene->add((Sphere&)Sphere(Vec3f(5.0f, 0.0f, -10.0f), 2.0f));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -6.0f), 0.5f));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -10.0f), 2.0f));
	scene->add((Sphere&)Sphere(Vec3f(-1.0f, -1.0f, -8.0f), 1.0f,Vec3f(0.0f,.1f,0.0f), Vec3f(0.0f,0.8f,0.0f)));
	scene->add((Sphere&)Sphere(Vec3f(-1.0f, 1.0f, -8.0f), 1.0f, Vec3f(0.1f, 0.0f, 0.0f), Vec3f(0.8f, 0.0f, 0.0f)));
	//scene->add((Sphere&)Sphere(Vec3f(-1.0f, -1.0f, -8.0f), 1.0f));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -10.0f), 2.0f));
	scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -18.0f), 8.0f));
	//scene->add(li);

	Image * image = new Image(640, 480);	
	
	rayTracer = new SimpleRayTracer(scene, image);

}

void main(int argc, char **argv) {
	glutInit(&argc, argv);
	init();
	glutMainLoop();
}
