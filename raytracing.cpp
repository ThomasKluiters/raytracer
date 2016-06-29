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
#include "KDTree.h"


// Temporary variables. (These are only used to illustrate a simple debug drawing.) 
Vec3Df testRayOrigin;
Vec3Df testRayDestination;

std::vector<Vec3Df> testArraystart;
std::vector<Vec3Df> testArrayfinish;
std::vector<Vec3Df> testArraycolor;

Camera * myCamera;
DebugScreen * myDebugScreen;

int maxDepth;
bool draw;

std::vector<Light> lights;



KDTree* tree;

/**
* Use this function for any preprocessing of the mesh.
*/
void init(Camera * camera, DebugScreen * debugScreen)
{
	myDebugScreen = debugScreen;
	myCamera = camera;
	//load the mesh file
	//please realize that not all OBJ files will successfully load.
	//Nonetheless, if they come from Blender, they should, if they 
	//are exported as WavefrontOBJ.
	//PLEASE ADAPT THE LINE BELOW TO THE FULL PATH OF THE dodgeColorTest.obj
	//model, e.g., "C:/temp/myData/GraphicsIsFun/dodgeColorTest.obj", 
	//otherwise the application will not load properly
	//MyMesh.loadMesh("cube.obj", true);
	MyMesh.loadMesh(myDebugScreen->sceneData.data(), true);
	MyMesh.computeVertexNormals();
	GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 10.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	
	glLightf(GL_LIGHT0, GL_DIFFUSE, *mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);


	//one first move: initialize the first light source
	//at least ONE light source has to be in the scene!!!
	//here, we set it to the current location of the camera
	MyLightPositions.push_back(MyCameraPosition);
	maxDepth = 4;
	draw = false;

	KDTreeBuilder builder(MyMesh);
	tree = builder.build();
}

/**
* Return the color of your pixel.
*/
//return the color of your pixel.
Vec3Df performRayTracing(const Vec3Df & origin, const Vec3Df & direction, int depth)
{
	Intersection intersection = tree->trace(origin, direction, MyMesh.triangles, MyMesh.vertices);

	if (!intersection.hit())
	{
		drawLine(origin, origin , Vec3Df(1, 0, 0));
		return Vec3Df(0, 0, 0);
	}

	const int triangle = intersection.triangle;

	const int material = MyMesh.triangleMaterials[triangle];

	Vec3Df normal = intersection.normal;
	Vec3Df inv_normal = -1.0f * normal;
	Vec3Df location = intersection.position;
	Vec3Df localColor = MyMesh.materials[material].Kd();

	for (auto light : lights)
	{	
		localColor = localColor + softshading(location, normal, origin, light, material);
	}

	if (depth < 6) {
		Vec3Df N = intersection.normal;
		N.normalize();

		Vec3Df R = -1.0f * direction;
		R.normalize();

		Vec3Df reflection = -R + 2 * Vec3Df::dotProduct(R, N) * N;

		drawLine(intersection.position, intersection.position + reflection, Vec3Df(1, 1, 0));
		drawLine(intersection.position, intersection.position + N, Vec3Df(0, 1, 1));
		drawLine(intersection.position, intersection.position + R, Vec3Df(0, 1, 1));

		return localColor * 0.5f + 0.5f * performRayTracing(location, reflection, depth + 1);

		//return (Tr)* localColor + Tr * T * performRayTracing(location, out_refraction, depth + 1) + (1 - Tr) * R * performRayTracing(location, out_reflection, depth + 1);


	}

	return localColor;
}


Vec3Df softshading(Vec3Df location, Vec3Df normal, Vec3Df origin, Light l, int material) {
	std::vector<Vec3Df> lights = l.lights(5);
	Vec3Df temp = Vec3Df(0, 0, 0);
	for (int i = 0; i < lights.size(); ++i) {
		temp = temp + lambertshading(location, normal, origin, lights[i], material);
	}
	return 1.0f / lights.size() * temp;
}

Vec3Df lambertshading(Vec3Df location, Vec3Df normal, Vec3Df origin, Vec3Df light, int material) {
	Vec3Df light_in = light - location;
	light_in.normalize();
	if (!lightobstructed(location, light)) {
		float s = Vec3Df::dotProduct(normal, light_in);
		if (s > 0) {
			Vec3Df color = s * MyMesh.materials[material].Kd();

			if (MyMesh.materials[material].has_Ks()) {
				Vec3Df specular = MyMesh.materials[material].Ks();;

				float shininess = MyMesh.materials[material].Ns();

				Vec3Df view = location - origin;
				view.normalize();

				Vec3Df reflection = light_in - 2 * Vec3Df::dotProduct(light_in, normal) * normal;
				float z = Vec3Df::dotProduct(view, reflection);


				if (Vec3Df::dotProduct(normal, light_in) >= 0 && z > 0)
				{
					color += pow(z, shininess) * specular;
				}
			}

			return  color;
		}
	}
	return Vec3Df(0, 0, 0);
}



bool lightobstructed(const Vec3Df & origin, const Vec3Df & dest)
{
	Intersection intersection = tree->trace(origin, dest - origin, MyMesh.triangles, MyMesh.vertices);
	if (intersection.hit())
	{
		float distanceToLight = Vec3Df::distance(origin, dest);
		float distanceToIntersetion = Vec3Df::distance(intersection.position, dest);

		return distanceToIntersetion <= distanceToLight;
	}
	return false;
}




/**
* Draw open gl debug stuff here. This function is called every frame.
*/
void yourDebugDraw()
{
	// Draw the mesh.
	MyMesh.draw();

	// Update cameraposition
	myCamera->transformCamera(MyCameraPosition);	
	myDebugScreen->drawPlane(Vec3Df(0.0, 0.0, 0.0), 12.0, 12.0);
	myDebugScreen->drawDebugInfo();
	myDebugScreen->indicateOrigin();

	// Draw the lights in the scene as points.
	glPushAttrib(GL_ALL_ATTRIB_BITS);				// (Store all GL attributes.)
	glDisable(GL_LIGHTING);
	glColor3f(1, 1, 0.6);
	glPointSize(10);
	glBegin(GL_POINTS);
	for (int i = 0; i<MyLightPositions.size(); ++i)
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
		glColor3f(testArraycolor[i][0], testArraycolor[i][1], testArraycolor[i][2]);
		glBegin(GL_LINES);
		glVertex3f(testArraystart[i][0], testArraystart[i][1], testArraystart[i][2]);
		glVertex3f(testArrayfinish[i][0], testArrayfinish[i][1], testArrayfinish[i][2]);
		glEnd();
		glPopAttrib();			
	}

	glEnable(GL_LIGHTING);
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
	drawLine(origin + v1, origin + v1 + v2, Vec3Df(0, 0, 1));
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
	if (rendering) {
		return;
	}

	//here, as an example, I use the ray to fill in the values for my upper global ray variable
	//I use these variables in the debugDraw function to draw the corresponding ray.
	//try it: Press a key, move the camera, see the ray that was launched as a line.
	testRayOrigin = rayOrigin;
	testRayDestination = rayDestination;
	//testArraystart.push_back(Vec3Df(0.0, 0.0, 0.0));
	//testArrayfinish.push_back(Vec3Df(15, 0, 0));
	//testArraycolor.push_back(Vec3Df(1, 1, 0));

	if (t == 't') {
		clearAllLines();
		draw = true;
		Vec3Df z = performRayTracing(testRayOrigin, testRayDestination - testRayOrigin, 4);
		drawLine(testRayOrigin, testRayDestination, Vec3Df(1.0, 1.0, 0.0));
		std::cout << " traced ray for" << rayOrigin << "," << rayDestination << "," << z << std::endl;
		draw = false;
	}
	//
	
	if (t == 'T') {
		clearAllLines();
		draw = true;
		Vec3Df at = testRayDestination - testRayOrigin;
		at.normalize();

		myCamera->traceThisRay(at, myCamera->camPos, testRayOrigin, testRayDestination);
		Vec3Df z = performRayTracing(testRayOrigin, testRayDestination - testRayOrigin, 4);
		drawLine(testRayOrigin, testRayDestination, Vec3Df(1.0, 1.0, 0.0));
		std::cout << " traced ray for" << rayOrigin << "," << rayDestination << "," << z << std::endl;
		draw = false;
	}

	if (t == 'c') {
		myCamera->lookAt(rayDestination);
		std::cout << MyCameraPosition << std::endl;
	}

	if (t == 'C') {
		clearAllLines();
		draw = true;
		drawLine(myCamera->lookAtPos, myCamera->camPos, Vec3Df(1.0,0.6,0.0));
		draw = false;
	}

	if (t == 'z') {
		clearAllLines();
		draw = true;
		for (int i = 0; i < MyMesh.vertices.size(); ++i) {
			draw = true;
			drawLine(MyMesh.vertices[i].p, MyMesh.vertices[i].p + MyMesh.vertices[i].n, Vec3Df(1, 0.5, 0.5));

		}
		draw = false;
	}

	// do here, whatever you want with the keyboard input t.

	//...

	std::cout << t << " pressed! The mouse was in location " << x << "," << y << "!" << std::endl;
}