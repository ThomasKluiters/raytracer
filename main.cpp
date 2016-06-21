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

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "raytracing.h"
#include "mesh.h"
#include "traqueboule.h"
#include "Light.h"
#include "SpotLight.h"
#ifndef _CAMERA
#define _CAMERA
#include "Camera.h"
#endif

/**
* This is the main application. Most of the code in here does not need to be modified. It is enough to take a look at 
* the function "drawFrame", in case you want to provide your own different drawing functions.
*/
Vec3Df MyCameraPosition;

/**
* MyLightPositions stores all the light positions to use for the ray tracing. Please notice, the light that is used for 
* the real-time rendering is NOT one of these, but following the camera instead.
*/
std::vector<Vec3Df> MyLightPositions;

Mesh MyMesh;						// Main mesh
unsigned int WindowSize_X = 800;	// X-resolution
unsigned int WindowSize_Y = 800;	// Y-resolution

Camera myCamera = Camera(WindowSize_X, WindowSize_Y, MyCameraPosition, 
						Vec3Df(0.0,1.0,0.0), 50.0, 11.0, 8.0);

/**
 * Drawing function, which draws an image (frame) on the screen.
 */
void drawFrame()
{
	yourDebugDraw();
}


/**
* Animation is called once for every image on the screen.
*/
void animate()
{
	MyCameraPosition = getCameraPosition();
	glutPostRedisplay();
}

void display(void);
void reshape(int w, int h);
Vec3Df sampleLens(float a, float b);
void keyboard(unsigned char key, int x, int y);

/**
 * Main function.
 */
int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	// Set up framebuffer.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	// Positioning and size of window.
	glutInitWindowPosition(200, 100);
	glutInitWindowSize(WindowSize_X, WindowSize_Y);
	glutCreateWindow(argv[0]);

	// Initialize viewpoint.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -4);
	tbInitTransform();				// (This is for the trackball, please ignore.)
	tbHelp();						// (idem.)
	MyCameraPosition = getCameraPosition();

	// Activate the light following the camera.
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	int LightPos[4] = { 0, 0, 2, 0 };
	int MatSpec[4] = { 1, 1, 1, 1 };
	glLightiv(GL_LIGHT0, GL_POSITION, LightPos);

	// Normals will be normalized in the graphics pipeline.
	glEnable(GL_NORMALIZE);

	// Clear color of the background is black.
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// Activate rendering mode and activate depth test,
	glEnable(GL_DEPTH_TEST);

	// Draw front-facing triangles filled and back-facing triangles as wires.
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_LINE);

	// Interpolate vertex colors over the triangles.
	glShadeModel(GL_SMOOTH);

	// Glut setup. (ignore.)
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display);
	glutMouseFunc(tbMouseFunc);    // trackball
	glutMotionFunc(tbMotionFunc);  // uses mouse
	glutIdleFunc(animate);

	init(&myCamera);

	// Main loop for glut. This just runs your application.
	glutMainLoop();

	// Execution never reaches this point.
	return 0;
}

/**
 * OpenGL setup.
 * 
 * Functions do not need to be changed! You can skip ahead to the keyboard function.
 */

/**
* What to do before drawing an image.
*/
void display(void)
{
	// Store GL state.
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// Clear everything.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	// Init trackball.
	tbVisuTransform();

	// Actually draw.
	drawFrame();

	// Glut internal switch.
	glutSwapBuffers();

	// Return to old GL state.
	glPopAttrib();
}

/**
* Window changes size.
*/
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//glOrtho (-1.1, 1.1, -1.1,1.1, -1000.0, 1000.0);
	gluPerspective(35, (float)w / h, 0.01, 30);
	glMatrixMode(GL_MODELVIEW);
}

/**
* Transform the x, y position on the screen into the corresponding 3D world position.
*/
void produceRay(int x_I, int y_I, Vec3Df * origin, Vec3Df * dest)
{
	int viewport[4];
	double modelview[16];
	double projection[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);	// Recover matrices.
	glGetDoublev(GL_PROJECTION_MATRIX, projection); // Recover matrices.
	glGetIntegerv(GL_VIEWPORT, viewport);			// Viewport.
	int y_new = viewport[3] - y_I;

	double x, y, z;

	gluUnProject(x_I, y_new, 0, modelview, projection, viewport, &x, &y, &z);
	origin->p[0] = float(x);
	origin->p[1] = float(y);
	origin->p[2] = float(z);

	gluUnProject(x_I, y_new, 1, modelview, projection, viewport, &x, &y, &z);
	dest->p[0] = float(x);
	dest->p[1] = float(y);
	dest->p[2] = float(z);
}

bool planeIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos, float & res)
{
	float dot_normalLine = Vec3Df::dotProduct(pointB, planeNormal);
	if (dot_normalLine > 1e-5) {
		Vec3Df crossingVector = planePos - pointA;
		res = Vec3Df::dotProduct(crossingVector, planeNormal) / dot_normalLine;
		return (res >= 0);
	}
}


// Return a random float between 0 and 1.
float randFloat1() {
	srand(34095830962);
	return rand() / RAND_MAX;
}

// Modifies the given initial eyeray by this camera's lens
// Does nothing if the lensradius is less than or equal to 0
// Changes ray by reference
Vec3Df traceLensRay(float x, float y, Vec3Df & origin, Vec3Df & destination, Vec3Df at) {
	float lensRadius = 80.0;
	float focalDistance = 14.0;		// Distance to focal PLANE
	float focalPointWeight, a, b;
	Vec3Df normalized_at = at;
	normalized_at.normalize();

	if (lensRadius > 0.0) {
		// Focalpoint is found by intersecting the focalPlane with origin/destination ray
		Vec3Df planePoint = normalized_at * focalDistance;
		planeIntersect(origin, destination, normalized_at, planePoint, focalPointWeight);

		Vec3Df imagePlanePoint = origin + at;		// Get middle of planePoint
		//Get some vector in image-plane
		Vec3Df im_vec1 = Vec3Df::crossProduct(imagePlanePoint+normalized_at, Vec3Df(0.0,1.0,0.0));
		Vec3Df im_vec2 = Vec3Df::crossProduct(imagePlanePoint + normalized_at, im_vec1);

		im_vec1.normalize();
		im_vec2.normalize();
		Vec3Df focalPoint = origin + destination * focalPointWeight;		// Intersect with focalPlane is focalpoint
		
		Vec3Df displacement_vector;
		Vec3Df result = performRayTracing(origin, destination);
		float a, b;
		float pixelWidth = 1.0 / (float)WindowSize_X;
		float pixelHeight = 1.0 / (float)WindowSize_Y;

		// random sampling
		for (int i = 0; i < 16; i++) {
			a = randFloat1() * 2.0 - 1.0; 
			b = randFloat1() * 2.0 - 1.0;
			a = a * lensRadius * pixelWidth; 
			b = b * lensRadius * pixelHeight;

			displacement_vector = im_vec1 * a + im_vec2 * b;
			result = result + performRayTracing(destination + displacement_vector, focalPoint);
		}
		
		result = result / 16.0;
		return result;
	}
	else {
		return performRayTracing(origin, destination);
	}
}




void traceThickLensRay(Vec3Df & origin, Vec3Df & destination, Vec3Df at) {
	float lensRadius = 1.2;
	float focalDistance = 14.0;

	if (lensRadius <= 0.0)
		return;

	float projx, projy, projz, projw;
	float lensThickness = 0.3;

	Vec3Df normalized_at = at;
	normalized_at.normalize();
	Vec3Df side = Vec3Df::crossProduct(normalized_at, Vec3Df(0.0, 1.0,0.0));
	Vec3Df up = Vec3Df::crossProduct(normalized_at, side);

	// - Map ray to lens-point
	float a, b;
	a = randFloat1();
	b = randFloat1();

	// Randomly sample a lenspoint on the aperture. (z = 1.0 by default)
	Vec3Df lensPoint = lensRadius * sampleLens(a,b);
	//Transform X/Y plane of lenspoint
	Vec3Df lensCenter = origin + at*2*focalDistance; //z
	float zprime = 1.0 / lensCenter.getLength();
	Vec3Df lensPos = lensCenter + (side * lensPoint[0] + up * lensPoint[1]);
	
	// thick-lens projection.
	projx = destination[0];
	projy = destination[1];
	projz = destination[2] + destination[2] * (lensThickness / focalDistance) + lensThickness;
	projw = destination[2] * (1 / focalDistance) + 1;
	
	// Divide by projw
	Vec3Df focalPoint((projx / projw), (projy / projw), (projz / projw));


	origin = destination;						// Origin now destination
	destination = focalPoint;					// Destination now focal point.
}

// Uses monte-carlo based concentric mapping to determine a point on this camera's lens
// Feed it random integers and it will return two appropriate weights for the up/side plane.
Vec3Df sampleLens(float a, float b) {
	float u, v;
	u = 2.0 * a - 1;
	v = 2.0 * b - 1;

	// At origin we should just return the origin instead of passing it to the monte-carlo integrator
	if (u == 0 && v == 0) return Vec3Df(0.0, 0.0, 0.0);			// I use Vec3Df for 2D points with z ignored

	float rPolar;
	float polarAngle;

	if (abs(u) > abs(v)) {
		rPolar = u;
		polarAngle = (M_PI / 4.0) * (v / rPolar);
	}
	else {
		rPolar = v;
		polarAngle = (M_PI / 2.0) - (M_PI / 4.0) * (v / rPolar);
	}
	return Vec3Df(cosf(polarAngle), sinf(polarAngle), 1.0);
}


/**
* React to keyboard input.
*/
void keyboard(unsigned char key, int x, int y)
{
	printf("key %d pressed at %d,%d\n", key, x, y);
	fflush(stdout);
	switch (key)
	{

	// Press 'L'.
	case 'L':
		MyLightPositions.push_back(getCameraPosition());
		break;

	// Click 'l'.
	case 'l':
		MyLightPositions[MyLightPositions.size() - 1] = getCameraPosition();
		break;

	case 'd':
	{
		Vec3Df mag = Vec3Df(0.0, 0.0, 0.0) - getCameraPosition();
		cout << "Length to origin is: " << mag.getLength() << endl;
		break;
	}


	// Click 'r'.
	case 'r':
	{
		// Pressing r will launch the raytracing.
		cout << "Raytracing" << endl;

		// Produce the rays for each pixel, by first computing the rays for the corners of the frustum.
		Vec3Df origin00, dest00;
		Vec3Df origin01, dest01;
		Vec3Df origin10, dest10;
		Vec3Df origin11, dest11;
		Vec3Df origin, dest;
		produceRay(0, 0, &origin00, &dest00);
		produceRay(0, WindowSize_Y - 1, &origin01, &dest01);
		produceRay(WindowSize_X - 1, 0, &origin10, &dest10);
		produceRay(WindowSize_X - 1, WindowSize_Y - 1, &origin11, &dest11);

		cout << origin00 << " | " << dest00 <<endl;
		cout << origin01 << " | " << dest01 << endl;
		cout << origin10 << " | " << dest10 << endl;
		cout << origin11 << " | " << dest11 << endl;

		// Find Middle at-vector
		float mx = 1.0f - float(x) / ((WindowSize_X - 1) / 2);
		float my = 1.0f - float(y) / ((WindowSize_Y - 1) / 2);

		Vec3Df at_vec = my*(mx*origin00 + (1 - mx)*origin10) +
			(1 - my)*(mx*origin01 + (1 - mx)*origin11);

		// I'm NOT normalizing the at-vector, because otherwise I have to hassle around to find the backplane distance.
		//at_vec.normalize();

		for (unsigned int y = 0; y < WindowSize_Y; ++y)
			for (unsigned int x = 0; x < WindowSize_X; ++x)
			{
				// Produce the rays for each pixel, by interpolating the four rays of the frustum corners.
				float xscale = 1.0f - float(x) / (WindowSize_X - 1);
				float yscale = 1.0f - float(y) / (WindowSize_Y - 1);

				origin = yscale*(xscale*origin00 + (1 - xscale)*origin10) +
					(1 - yscale)*(xscale*origin01 + (1 - xscale)*origin11);
				dest = yscale*(xscale*dest00 + (1 - xscale)*dest10) +
					(1 - yscale)*(xscale*dest01 + (1 - xscale)*dest11);

				float a, b;
				a = randFloat1();
				b = randFloat1();

				//Modify by lens
				//traceLensRay(a,b,origin, dest, at_vec);
				
				// Launch raytracing for the given ray.
				Vec3Df rgb = traceLensRay(a, b, origin, dest, at_vec);//performRayTracing(origin ,dest);

				// Store the result in camera backplane
				myCamera.sensor.setPixel(x, y, rgb);
			}

		myCamera.sensor.writeToDisk("result.ppm");
		break;
	}
	case 'a':
		cout << "Dumping Colour Buffer" << endl;
		myCamera.sensor.writeToDisk("buffer.ppm");
		break;

	// CLick 'Esc'.
	case 27:
		exit(0);
	}

	// Produce the ray for the current mouse position.
	Vec3Df testRayOrigin, testRayDestination;
	produceRay(x, y, &testRayOrigin, &testRayDestination);

	yourKeyboardFunc(key, x, y, testRayOrigin, testRayDestination);
}