#include <stdio.h>
#include <limits.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include "raytracing.h"

// Temporary variables. (These are only used to illustrate a simple debug drawing.) 
Vec3Df testRayOrigin;
Vec3Df testRayDestination;

std::vector<Vec3Df> testArraystart;
std::vector<Vec3Df> testArrayfinish;
std::vector<Vec3Df> testArraycolor;

int maxDepth;
bool draw;

std::vector<Light> lights;


/**
* Use this function for any preprocessing of the mesh.
*/
void init()
{
	//load the mesh file
	//please realize that not all OBJ files will successfully load.
	//Nonetheless, if they come from Blender, they should, if they 
	//are exported as WavefrontOBJ.
	//PLEASE ADAPT THE LINE BELOW TO THE FULL PATH OF THE dodgeColorTest.obj
	//model, e.g., "C:/temp/myData/GraphicsIsFun/dodgeColorTest.obj", 
	//otherwise the application will not load properly
	//MyMesh.loadMesh("cube.obj", true);
	MyMesh.loadMesh("cube.obj", true);
 //   MyMesh.loadMesh("dodgeColorTest.obj", true);
	MyMesh.computeVertexNormals();

	//one first move: initialize the first light source
	//at least ONE light source has to be in the scene!!!
	//here, we set it to the current location of the camera
	MyLightPositions.push_back(MyCameraPosition);
	maxDepth = 4;
	draw = false;
}

/**
* Return the color of your pixel.
*/
//return the color of your pixel.
Vec3Df performRayTracing(const Vec3Df & origin, const Vec3Df & dest)
{
	return recursiveRaytracer(origin, dest, 1);
}

Vec3Df recursiveRaytracer(const Vec3Df & origin, const Vec3Df & dest, int depth) {

	Vec3Df ray = dest - origin;
	Vec3Df val = Vec3Df(0, 0, 0);
	float dist = -1;
	float newdist = 0;
	int triangle = -1;
	Vec3Df normalintersect, closestIntersect;
	for (int i = 0; i<MyMesh.triangles.size(); ++i) {
		Vec3Df Sg = MyMesh.vertices[MyMesh.triangles[i].v[0]].p;
		Vec3Df Sone = MyMesh.vertices[MyMesh.triangles[i].v[1]].p - Sg;
		Vec3Df Stwo = MyMesh.vertices[MyMesh.triangles[i].v[2]].p - Sg;

		Vec3Df normal = Vec3Df::crossProduct(Sone, Stwo);
		normal.normalize();

		float np = Vec3Df::dotProduct(ray, normal);

		//float np  = Vec3Df::dotProduct(ray, MyMesh.vertices[MyMesh.triangles[i].v[1]].n);

		if (np != 0) {
			double d = Vec3Df::dotProduct((Sg - origin), normal) / np;
			//  float d = Vec3Df::dotProduct((Sg - origin), MyMesh.vertices[MyMesh.triangles[i].v[1]].n)/np;
			Vec3Df intersect = d * ray + origin;
			Vec3Df v0 = Sone;
			Vec3Df v1 = Stwo;
			Vec3Df v2 = intersect - Sg;


			//    Vector v0 = b - a, v1 = c - a, v2 = p - a;
			float d00 = Vec3Df::dotProduct(v0, v0);
			float d01 = Vec3Df::dotProduct(v0, v1);
			float d11 = Vec3Df::dotProduct(v1, v1);
			float d20 = Vec3Df::dotProduct(v2, v0);
			float d21 = Vec3Df::dotProduct(v2, v1);
			float denom = d00 * d11 - d01 * d01;
			float v = (d11 * d20 - d01 * d21) / denom;
			float w = (d00 * d21 - d01 * d20) / denom;
			float u = 1.0f - v - w;
			if (v>0 && w > 0 && v + w <1 && d>0) {
				Vec3Df temp = origin - intersect;
				newdist = temp.getLength();
				if ((newdist < dist || dist == -1.0f) && newdist > 0.1) {
					dist = newdist;
					triangle = i;
					normalintersect = normal;
					closestIntersect = intersect;
				}

				//return Vec3Df(0.4, 0, 0.6);
			}

		}
		//return Vec3Df(1,0,0);
		//return Vec3Df(dest[0],dest[1],dest[2]);
	}
	if (triangle != -1) {

		//testArraystart.push_back(origin);
		//testArrayfinish.push_back(closestIntersect);
		//testArraycolor.push_back(Vec3Df(1, 1, 0));

		drawLine(origin, closestIntersect, Vec3Df(1, 1, 0));
		drawLine(closestIntersect, (normalintersect + closestIntersect), Vec3Df(1, 0.5, 1));
		// local light values
		Vec3Df localcolor = MyMesh.materials[MyMesh.triangleMaterials[triangle]].Ka();
		for (int i = 0; i < lights.size(); ++i) {
			localcolor = localcolor + softshading(closestIntersect, normalintersect, lights[i], triangle);
		}
		if (MyMesh.materials[MyMesh.triangleMaterials[triangle]].has_Ni() && maxDepth > depth) {

			Vec3Df in = closestIntersect - origin;
			float Ni = MyMesh.materials[MyMesh.triangleMaterials[triangle]].Ni();
			in.normalize();
			float debug = Vec3Df::dotProduct(in, normalintersect);
			Vec3Df out = in - (2 * Vec3Df::dotProduct(in, normalintersect) * normalintersect);
		//	drawLine(closestIntersect, out + closestIntersect, Vec3Df(0.5, 1, 0.5));
			Vec3Df reflection = recursiveRaytracer(closestIntersect, (closestIntersect + out), depth +1);
			val = Ni * reflection + (1 - Ni) * localcolor;
		}
		else {
			val = localcolor;

		}

	}
	else {
		drawLine(origin, dest, Vec3Df(1, 0, 0));
	}



	//	testArraystart.push_back(origin);
	//	testArrayfinish.push_back(dest);
	//	testArraycolor.push_back(Vec3Df(1, 0, 0));
	return val;
}

Vec3Df softshading(Vec3Df location, Vec3Df normal, Light l, int material) {
	std::vector<Vec3Df> lights = l.lights(3);
	Vec3Df temp = Vec3Df(0, 0, 0);
	for (int i = 0; i < lights.size(); ++i) {
		temp = temp + lambertshading(location, normal, lights[i], material);
	}
	return 1.0f / lights.size() * temp;
}


Vec3Df lambertshading(Vec3Df location, Vec3Df normal, Vec3Df light, int material) {
	Vec3Df light_in = light - location;
	light_in.normalize();
	if (!lightobstructed(location, light)) {
		float s = Vec3Df::dotProduct(normal, light_in);
		if (s > 0) {
			Vec3Df value = s * MyMesh.materials[MyMesh.triangleMaterials[material]].Kd();
			return  value;
		}
	}
	return Vec3Df(0, 0, 0);
}



bool lightobstructed(const Vec3Df & origin, const Vec3Df & dest){
	Vec3Df ray = dest - origin;
	for (int i = 0; i<MyMesh.triangles.size(); ++i) {
		Vec3Df Sg = MyMesh.vertices[MyMesh.triangles[i].v[0]].p;
		Vec3Df Sone = MyMesh.vertices[MyMesh.triangles[i].v[1]].p - Sg;
		Vec3Df Stwo = MyMesh.vertices[MyMesh.triangles[i].v[2]].p - Sg;

		Vec3Df normal = Vec3Df::crossProduct(Sone, Stwo);
		normal.normalize();

		float np = Vec3Df::dotProduct(ray, normal);


		if (np != 0) {
			float d = Vec3Df::dotProduct((Sg - origin), normal) / np;
			if (d > 0) {
				Vec3Df intersect = d * ray + origin;
				Vec3Df v0 = Sone;
				Vec3Df v1 = Stwo;
				Vec3Df v2 = intersect - Sg;
				//    Vector v0 = b - a, v1 = c - a, v2 = p - a;
				float d00 = Vec3Df::dotProduct(v0, v0);
				float d01 = Vec3Df::dotProduct(v0, v1);
				float d11 = Vec3Df::dotProduct(v1, v1);
				float d20 = Vec3Df::dotProduct(v2, v0);
				float d21 = Vec3Df::dotProduct(v2, v1);
				float denom = d00 * d11 - d01 * d01;
				float v = (d11 * d20 - d01 * d21) / denom;
				float w = (d00 * d21 - d01 * d20) / denom;
				float u = 1.0f - v - w;
				Vec3Df temp = origin - intersect;
				float newdist = temp.getLength();
				if (v > 0 && w > 0 && v + w <1 && newdist > 0.1) {
					
					drawLine(origin, intersect, Vec3Df(1, 0, 1));

					return true;
				}
			}
		}	
		}
	drawLine(origin, dest, Vec3Df(1, 0, 1));
	return false;
	}




/**
* Draw open gl debug stuff here. This function is called every frame.
*/
void yourDebugDraw()
{
	// Draw the mesh.
	MyMesh.draw();
	
	// Draw the lights in the scene as points.
	glPushAttrib(GL_ALL_ATTRIB_BITS);				// (Store all GL attributes.)
	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);
	glPointSize(10);
	glBegin(GL_POINTS);
	for (int i=0;i<MyLightPositions.size();++i)
		glVertex3fv(MyLightPositions[i].pointer());
	glEnd();
	glPopAttrib();									// (Restore all GL attributes.)
	
	// Draw the test ray, which is set by the keyboard function.
	/*
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
	glColor3f(0,1,1);
	glVertex3f(testRayOrigin[0], testRayOrigin[1], testRayOrigin[2]);
	glColor3f(0,0,1);
	glVertex3f(testRayDestination[0], testRayDestination[1], testRayDestination[2]);
	glEnd();
	glPointSize(10);
	glBegin(GL_POINTS);
	glVertex3fv(MyLightPositions[0].pointer());
	glEnd();
	glPopAttrib();
	*/
	for (int i = 0; i < testArrayfinish.size(); ++i) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glLineWidth(2.5);
		glColor3f(testArraycolor[i][0],testArraycolor[i][1], testArraycolor[i][2]);
		glBegin(GL_LINES);
		glVertex3f(testArraystart[i][0], testArraystart[i][1], testArraystart[i][2]);
		glVertex3f(testArrayfinish[i][0], testArrayfinish[i][1], testArrayfinish[i][2]);
		//glVertex3f(source[0], source[1], source[2]);
		//glVertex3f(dest[0],dest[1],dest[2]);
		glEnd();
		glPopAttrib();
	}
	
	// Draw whatever else you want...
	////glutSolidSphere(1,10,10);
	////allows you to draw a sphere at the origin.
	////using a glTranslate, it can be shifted to whereever you want
	////if you produce a sphere renderer, this 
	////triangulated sphere is nice for the preview
}

void clearAllLines() {
	testArraystart.clear();
	testArrayfinish.clear();
	testArraycolor.clear();
}

void drawLine(Vec3Df origin, Vec3Df dest, Vec3Df color) {
	if (draw) {
		testArraystart.push_back(origin);
		testArrayfinish.push_back(dest);
		testArraycolor.push_back(color);
	}
}

void lichtbak(Vec3Df origin, Vec3Df dest) {
	Vec3Df normal = dest - origin;
	normal.normalize();
	Vec3Df v1 = Vec3Df(1, normal[0] / normal[1], 0);
	v1.normalize();
	Vec3Df v2 = Vec3Df::crossProduct(normal, v1);
	v2.normalize();
	v1 = Vec3Df::crossProduct(normal, v2);
	v1.normalize();
	v1 = 0.2 * v1;
	v2 = 0.2 * v2;

	drawLine(origin, origin + normal, Vec3Df(1, 0, 0));
	drawLine(origin, origin + v1, Vec3Df(0, 1, 1));
	drawLine(origin, origin + v2, Vec3Df(0, 0, 1));
	drawLine(origin + v1, origin + v1 + v2, Vec3Df(0,0,1));
	drawLine(origin + v2, origin + v1 + v2, Vec3Df(0, 0, 1));

	Light l(origin, origin + v1, origin + v2);
	lights.push_back(l);


}




/**
* yourKeyboardFunc is used to deal with keyboard input.
* 
* - t is the character that was pressed.
* - x is the mouse's x position in pixels.
* - y is the mouse's y position in pixels.
* - rayOrigin, rayDestination represent the ray that is going in the view direction underneath your mouse position.
*
* Reserved keys:
* - 'L' adds a light positioned at the camera location to the MyLightPositions vector.
* - 'l' modifies the last added light to the current camera position (by default, there is only one light, so move it 
*	with l). ATTENTION: These lights do NOT affect the real-time rendering. You should use them for the raytracing.
* - 'r' calls the function performRaytracing on EVERY pixel, using the correct associated ray. It then stores the result
*	in an image "result.ppm". Initially, this function is fast (performRaytracing simply returns the target of the ray -
*	see the code above), but once you replaced this function and raytracing is in place, it might take a while to 
*	complete...
*/
void yourKeyboardFunc(char t, int x, int y, const Vec3Df & rayOrigin, const Vec3Df & rayDestination)
{
	//here, as an example, I use the ray to fill in the values for my upper global ray variable
	//I use these variables in the debugDraw function to draw the corresponding ray.
	//try it: Press a key, move the camera, see the ray that was launched as a line.
	testRayOrigin=rayOrigin;	
	testRayDestination=rayDestination;
	//testArraystart.push_back(Vec3Df(0.0, 0.0, 0.0));
	//testArrayfinish.push_back(Vec3Df(15, 0, 0));
	//testArraycolor.push_back(Vec3Df(1, 1, 0));

	if (t == 't') {
		clearAllLines();
		draw = true;
		Vec3Df z = performRayTracing(testRayOrigin, testRayDestination);
		std::cout <<" traced ray for" << rayOrigin << "," << rayDestination << "," << z  << std::endl;
		draw = false;
	}

	if (t == 'c') {
		clearAllLines();
		std::cout << MyCameraPosition << std::endl;
	}

	if (t == 'n') {
		draw = true;

		lichtbak(rayOrigin, rayDestination);

		draw = false;
	}
	
	// do here, whatever you want with the keyboard input t.
	
	//...
	
	std::cout<<t<<" pressed! The mouse was in location "<<x<<","<<y<<"!"<<std::endl;	
}
