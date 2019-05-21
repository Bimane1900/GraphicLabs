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
	glColor3f(c.x, c.y, c.z);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
}

class Light {
private:
	Vec3f intensity = Vec3f(1.0f, 1.0f, 1.0f);
	Vec3f position = Vec3f(0.0f, 0.0f, -3.0f);

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




class SimpleRayTracer {
private:
	Image * image;
	int reflectDepth = 0;
	int reflectLimit = 1;
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

		//int fov = 20;
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

	void searchAnyHit(const Ray & ray, HitRec & hitRec) {
		for (int i = 0; i < scene->spheres.size(); i++) {
			rayCount++;
			if (scene->spheres[i].hit(ray, hitRec)) return;


		}
	}


	//void searchClosestHit(const Ray & ray, HitRec & hitRec) {
	//	for (int i = 0; i < scene->spheres.size(); i++) {
	//		rayCount++;
	//		scene->spheres[i].hit(ray, hitRec, i);



	//	}

	//	if (hitRec.anyHit)
	//		scene->spheres[hitRec.primIndex].computeSurfaceHitFields(ray, hitRec);
	//}

	void searchClosestHit(const Ray & ray, HitRec & hit_rec) const {
		hit_rec.anyHit = false;

		for (auto i = 0; i < scene->spheres.size(); i++) {
			if (scene->spheres[i].hit(ray, hit_rec))
				hit_rec.primIndex = i;


			// Get intersection point(p) and normal(n) for the closest hitting sphere
			if (hit_rec.anyHit)
				scene->spheres[hit_rec.primIndex].computeSurfaceHitFields(ray, hit_rec);
		}
	}

	float max(float x, float y) { return x < y ? y : x; }
	float min(float x, float y) { return x > y ? y : x; }

	Vec3f phong(Ray  ray, HitRec hitRec) {
		Sphere s = scene->spheres[hitRec.primIndex];
		//s.computeSurfaceHitFields(ray, hitRec);
		//Vec3f ambient = light.getIntensity().multCoordwise(s.ambient);
		Vec3f L = (light.getPosition() - hitRec.p).normalize();
		Vec3f diffuse = light.getIntensity().multCoordwise(s.diffuse) * max(hitRec.n.dot(L), 0.0);

		Vec3f R = (hitRec.n * (L.dot(hitRec.n)) * 2.0 - L).normalize();
		Vec3f V = (cam.getPosition() - hitRec.p).normalize();
		Vec3f specular = light.getIntensity().multCoordwise(s.specular) * pow(max((R.dot(V)), 0.0), s.shiny);
		return (diffuse + specular);
	}


	Vec3f lightning(Ray ray, HitRec  hitRec, Vec3f  color) {
		Sphere s = scene->spheres[hitRec.primIndex];
		Vec3f ambient = light.getIntensity().multCoordwise(s.ambient);
		color += ambient;

		HitRec ShadowR;
		Ray ShadowRay;
		Vec3f LDir = (light.getPosition() - hitRec.p).normalize();
		ShadowRay.d = LDir;

		ShadowRay.o = hitRec.p + (ShadowRay.d * ray.rayEps);

		ShadowR.anyHit = false;
		searchAnyHit(ShadowRay, ShadowR);
		if (ShadowR.anyHit == false)
		{
			color += phong(ray, hitRec);
		}

		if (reflectCount < reflectLimit) {
			reflectCount++;
			Vec3f R = (ray.d - (hitRec.n * (ray.d.dot(hitRec.n)) * 2.0)).normalize();

			Ray reflect;
			HitRec Ref;
			reflect.o = hitRec.p + (R * ray.rayEps);
			reflect.d = R;

			Vec3f color2 = raytrace(reflect, Ref).multCoordwise(s.specular);
			color = color + color2;

			/*Ray r_ray;
			HitRec r_rec;
			float s = 2 * (ray.d.dot(hit_rec.n));
			Vec3f scalar = Vec3f(s, s, s);
			r_ray.d = ray.d - (scalar.mult_coordwise(hit_rec.n));
			r_ray.o = hit_rec.p + (r_ray.d * 0.5f);

			color += ray_trace(r_ray, r_rec, calls - 1).mult_coordwise(sphere.ks * 0.2);*/

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
			//return phong(ray, hitrec) + Vec3f(0.3,0.3,0.3);
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

				color = raytrace(ray, hitRec);

				color.x = max(0.0, color.x);
				color.y = max(0.0, color.y);
				color.z = max(0.0, color.z);

				color.x = min(1.0, color.x);
				color.y = min(1.0, color.y);
				color.z = min(1.0, color.z);

				if (color.x > 1.0 || color.x < 0.0 || color.y > 1.0 || color.y < 0.0 || color.z > 1.0 || color.z < 0.0)
				{
					cout << "it went to shit" << endl;
				}
				image->setPixel(x, y, color);
				glSetPixel(x, y, color);
			}
		}
	}
};


SimpleRayTracer * rayTracer;

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT);
	//glTranslatef(cam.getX(), cam.getY(), cam.getZ());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	time_t timer = time(NULL);
	rayTracer->rayCount = 0;
	rayTracer->fireRays();
	timer = time(NULL) - timer;
	cout << "Time: " << timer << " Rays: " << rayTracer->rayCount << endl;
	cout << "Cam: " << cam.getX() << " " << cam.getY() << " " << cam.getZ() << endl;
	cout << "Light: " << light.getX() << " " << light.getY() << " " << light.getZ() << endl;
	for (int i = 0; i < rayTracer->scene->spheres.size(); i++)
	{
		cout << "Sphere " << i << ": " << rayTracer->scene->spheres[i].c.x << " " << rayTracer->scene->spheres[i].c.y << " " << rayTracer->scene->spheres[i].c.z << endl;
	}


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

	scene->add((Sphere&)Sphere(Vec3f(-0.6f, 0.0f, -7.0f), 0.5f, Vec3f(0.1f, 0.0f, 0.0f), Vec3f(0.8f, 0.0f, 0.0f)));
	scene->add((Sphere&)Sphere(Vec3f(0.6f, 0.0f, -7.0f), 0.5f, Vec3f(0.1f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 0.9f)));
	//scene->add((Sphere&)Sphere(Vec3f(0.0f, 0.0f, -10.0f), 2.0f));

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