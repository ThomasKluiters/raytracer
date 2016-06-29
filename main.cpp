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
#include <thread>
#include <vector>
#include <ctime>
#include <map>
#include <numeric>
#ifdef _WIN32
#include <io.h>
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include "Communication.h"
#include "raytracing.h"
#include "mesh.h"
#include "traqueboule.h"
#include "Light.h"
#include <iterator>

#ifndef _LIGHTS
#define _LIGHTS
#include "Light.h"
#include "SpotLight.h"
#endif

void display(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);

void renderStart();

void rayTraceStart(Vec3Df origin00, Vec3Df dest00, Vec3Df origin01, Vec3Df dest01, Vec3Df origin10, Vec3Df dest10, Vec3Df origin11, Vec3Df dest11);

void performRaytracingBlock(unsigned int xStart, unsigned int xEnd, unsigned int yStart, unsigned int yEnd, Vec3Df origin00, Vec3Df origin01, Vec3Df origin10, Vec3Df origin11, Vec3Df dest00, Vec3Df dest01, Vec3Df dest10, Vec3Df dest11, FilmPlane* sensor, int threadNumber, std::vector<float>* progress);


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

Mesh MyMesh; // Main mesh
unsigned int WindowSize_X = 1440; // X-resolution
unsigned int WindowSize_Y = 900; // Y-resolution
bool rendering = false;
bool realtime_rendering = false;

std::string sceneData = "cube.obj";
std::string renderOutput = "result.ppm";
#define ANTIALIASING false

Camera myCamera = Camera(WindowSize_X, WindowSize_Y, MyCameraPosition,
                         Vec3Df(0.0, 1.0, 0.0), 50.0, 11.0, 8.0);

DebugScreen myDebugScreen = DebugScreen(sceneData, &myCamera, &WindowSize_X, &WindowSize_Y, GLUT_BITMAP_9_BY_15);

unsigned int startX = 0;
unsigned int endX = 800;
unsigned int startY = 0;
unsigned int endY = 800;

boolean headlessMode;

#define NUM_THREADS 16              // Max number of threads
#define NUM_BLOCKS_X 4              // Number of blocks in x direction
#define NUM_BLOCKS_Y 4              // Number of blocks in y direction


void renderScene();

/**
* Drawing function, which draws an image (frame) on the screen.
*/
void drawFrame()
{
	if (realtime_rendering)
	{
		renderStart();
	}

	myDebugScreen.putInt("Vertices", MyMesh.vertices.size());
	myDebugScreen.putInt("Lights", MyLightPositions.size());
	myDebugScreen.putBool("Rendering", rendering);
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

/**
* Main function.
*/
int main(int argc, char** argv)
{
	headlessMode = argc > 1 && argv[1][0] == 't';

	if (headlessMode)
	{
		const string serverIP = argv[2];
		const unsigned short serverPort = atoi(argv[3]);
		cout << serverIP << endl;
		cout << serverPort << endl;

		connection = new TCPSocket(serverIP, serverPort);

		// Receive info from server
		vector<int> bounds = Communication::receiveInitMessage();
		WindowSize_X = bounds[0];
		WindowSize_Y = bounds[1];
		startX = bounds[2];
		startY = bounds[3];
		endX = bounds[4];
		endY = bounds[5];

		// Set an initial lichtbak
		lichtbak(Vec3Df(0.00125903, 0.00209838, 3.99), Vec3Df(1.25903, 2.09839, -6.0));

		init(&myCamera, &myDebugScreen);
		rayTraceStart(origin00, dest00, origin01, dest01, origin10, dest10, origin11, dest11);

		myCamera.sensor.writeToDisk("result.ppm");
		Communication::sendImage(myCamera.sensor);
		return 0;
	}

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
	tbInitTransform(); // (This is for the trackball, please ignore.)
	tbHelp(); // (idem.)
	MyCameraPosition = getCameraPosition();

	// Activate the light following the camera.
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	int LightPos[4] = {0, 0, 2, 0};
	int MatSpec[4] = {1, 1, 1, 1};
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
	glutMouseFunc(tbMouseFunc); // trackball
	glutMotionFunc(tbMotionFunc); // uses mouse
	glutIdleFunc(animate);


	myDebugScreen.putBool("AA", ANTIALIASING);
	myDebugScreen.putInt("Max threads", NUM_THREADS);
	myDebugScreen.putString("Mode", "default");
	myDebugScreen.putVector("Camera Position", &myCamera.camPos);
	myDebugScreen.putVector("Camera 'LookAt", &myCamera.lookAtPos);
	myDebugScreen.putString("Render Output", renderOutput);

	init(&myCamera, &myDebugScreen);

	// Main loop for glut. This just runs your application.
	glutMainLoop();

	// Execution never reaches this point.
	return 0;
	//	headlessMode = argc > 1 && argv[1][0] == 't';
	//
	//	if (headlessMode)
	//	{
	//		const string serverIP = argv[2];
	//		const unsigned short serverPort = atoi(argv[3]);
	//		cout << serverIP << endl;
	//		cout << serverPort << endl;
	//
	//		connection = new TCPSocket(serverIP, serverPort);
	//
	//		// Receive info from server
	//		vector<int> bounds = Communication::receiveInitMessage();
	//		WindowSize_X = bounds[0];
	//		WindowSize_Y = bounds[1];
	//		startX = bounds[2];
	//		startY = bounds[3];
	//		endX = bounds[4];
	//		endY = bounds[5];
	//
	//		// Set an initial lichtbak
	//		lichtbak(Vec3Df(0.00125903, 0.00209838, 3.99), Vec3Df(1.25903, 2.09839, -6.0));
	//
	//		init();
	//		renderScene();
	//
	//		return 0;
	//	}
	//
	//    glutInit(&argc, argv);
	//    
	//    // Set up framebuffer.
	//    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	//    
	//    // Positioning and size of window.
	//    glutInitWindowPosition(200, 100);
	//    glutInitWindowSize(WindowSize_X, WindowSize_Y);
	//    glutCreateWindow(argv[0]);
	//    
	//    // Initialize viewpoint.
	//    glMatrixMode(GL_MODELVIEW);
	//    glLoadIdentity();
	//    glTranslatef(0, 0, -4);
	//    tbInitTransform();				// (This is for the trackball, please ignore.)
	//    tbHelp();						// (idem.)
	//    MyCameraPosition = getCameraPosition();
	//    
	//    // Activate the light following the camera.
	//    glEnable(GL_LIGHTING);
	//    glEnable(GL_LIGHT0);
	//    glEnable(GL_COLOR_MATERIAL);
	//    int LightPos[4] = { 0, 0, 2, 0 };
	//    int MatSpec[4] = { 1, 1, 1, 1 };
	//    glLightiv(GL_LIGHT0, GL_POSITION, LightPos);
	//    
	//    // Normals will be normalized in the graphics pipeline.
	//    glEnable(GL_NORMALIZE);
	//    
	//    // Clear color of the background is black.
	//    glClearColor(0.0, 0.0, 0.0, 0.0);
	//    
	//    // Activate rendering mode and activate depth test,
	//    glEnable(GL_DEPTH_TEST);
	//    
	//    // Draw front-facing triangles filled and back-facing triangles as wires.
	//    glPolygonMode(GL_FRONT, GL_FILL);
	//    glPolygonMode(GL_BACK, GL_LINE);
	//    
	//    // Interpolate vertex colors over the triangles.
	//    glShadeModel(GL_SMOOTH);
	//    
	//    // Glut setup. (ignore.)
	//    glutReshapeFunc(reshape);
	//    glutKeyboardFunc(keyboard);
	//    glutDisplayFunc(display);
	//    glutMouseFunc(tbMouseFunc);    // trackball
	//    glutMotionFunc(tbMotionFunc);  // uses mouse
	//    glutIdleFunc(animate);
	//    
	//    init();
	//    
	//    glutMainLoop();
	//
	//    return 0;
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
	WindowSize_X = w;
	WindowSize_Y = h;
	//glOrtho (-1.1, 1.1, -1.1,1.1, -1000.0, 1000.0);
	gluPerspective(50, (float)w / h, 0.01, 60);
	glMatrixMode(GL_MODELVIEW);
}

/**
* Transform the x, y position on the screen into the corresponding 3D world position.
*/
void produceRay(int x_I, int y_I, Vec3Df* origin, Vec3Df* dest)
{
	int viewport[4];
	double modelview[16];
	double projection[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview); // Recover matrices.
	glGetDoublev(GL_PROJECTION_MATRIX, projection); // Recover matrices.
	glGetIntegerv(GL_VIEWPORT, viewport); // Viewport.
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


// Performs anti-aliasing
void performAntiAliasing(int x, int y, Vec3Df& rgb,
                         Vec3Df& origin00,
                         Vec3Df& origin01,
                         Vec3Df& origin10,
                         Vec3Df& origin11,
                         Vec3Df& dest00,
                         Vec3Df& dest01,
                         Vec3Df& dest10,
                         Vec3Df& dest11
)
{
	for (float xOffset = -0.5f; xOffset <= 0.5f; xOffset += 0.5f)
	{
		for (float yOffset = -0.5f; yOffset <= 0.5f; yOffset += 0.5f)
		{
			// Produce the rays for each pixel, by interpolating the four rays of the frustum corners.
			float xscale = 1.0f - (float(x) + xOffset) / (WindowSize_X - 1);
			float yscale = 1.0f - (float(y) + yOffset) / (WindowSize_Y - 1);

			Vec3Df origin = yscale * (xscale * origin00 + (1 - xscale) * origin10) +
				(1 - yscale) * (xscale * origin01 + (1 - xscale) * origin11);
			Vec3Df dest = yscale * (xscale * dest00 + (1 - xscale) * dest10) +
				(1 - yscale) * (xscale * dest01 + (1 - xscale) * dest11);

			rgb = rgb + performRayTracing(origin, dest, 0);
			rgb = rgb / 9.0f;
		}
	}
}

/**
 * Perform ray tracing for a certain block.
 */
//void performRaytracingBlock( unsigned int xStart,
//                             unsigned int xEnd,
//                             unsigned int yStart,
//                             unsigned int yEnd,
//                             Vec3Df origin00,
//                             Vec3Df origin01,
//                             Vec3Df origin10,
//                             Vec3Df origin11,
//                             Vec3Df dest00,
//                             Vec3Df dest01,
//                             Vec3Df dest10,
//                             Vec3Df dest11,
//                             Image *result,
//                             int threadNumber,
//                             std::vector<float> *progress
//                             ) {
//    
//    Vec3Df origin, dest;
//    float localProgress;
//    
//    cout << " ystart " << yStart << " yend " << yEnd << " xstart " << xStart << " xend " << xEnd << endl;
//    
//    for (unsigned int y = yStart; y < yEnd && y < WindowSize_Y; ++y) {
//        
//        for (unsigned int x = xStart; x < xEnd && x < WindowSize_X; ++x)
//        {
//            // Progress indication
//            localProgress = ( (float) y - yStart) / (yEnd - yStart) + (1.0f / (yEnd - yStart)) * ( (float) x / (xEnd - 1) );
//            (*progress)[threadNumber] = localProgress;
//            
//			Vec3Df rgb(0, 0, 0);
//
//			if (ANTIALIASING)
//			{
//				for (float xOffset = -0.5f; xOffset <= 0.5f; xOffset += 0.5f)
//				{
//					for (float yOffset = -0.5f; yOffset <= 0.5f; yOffset += 0.5f)
//					{
//						// Produce the rays for each pixel, by interpolating the four rays of the frustum corners.
//						float xscale = 1.0f - (float(x) + xOffset) / (WindowSize_X - 1);
//						float yscale = 1.0f - (float(y) + yOffset) / (WindowSize_Y - 1);
//
//						origin = yscale*(xscale*origin00 + (1 - xscale)*origin10) +
//							(1 - yscale)*(xscale*origin01 + (1 - xscale)*origin11);
//						dest = yscale*(xscale*dest00 + (1 - xscale)*dest10) +
//							(1 - yscale)*(xscale*dest01 + (1 - xscale)*dest11);
//
//						rgb = rgb + performRayTracing(origin, dest, 0);
//					}
//				}
//
//				rgb = rgb / 9.0f;
//			}
//			else
//			{
//				float xscale = 1.0f - (float(x)) / (WindowSize_X - 1);
//				float yscale = 1.0f - (float(y)) / (WindowSize_Y - 1);
//
//				origin = yscale*(xscale*origin00 + (1 - xscale)*origin10) +
//					(1 - yscale)*(xscale*origin01 + (1 - xscale)*origin11);
//				dest = yscale*(xscale*dest00 + (1 - xscale)*dest10) +
//					(1 - yscale)*(xscale*dest01 + (1 - xscale)*dest11);
//				
//				rgb = performRayTracing(origin, dest - origin, 0);
//			}
//
//            // Launch raytracing for the given ray.
//                       
//            // Store the result in an image.
//            result->setPixel(x, y, RGBValue(rgb[0], rgb[1], rgb[2]));
//        }
//    }
//    
//>>>>>>> master
//}


/**
* Precompute triangle values of triangles starting at given triangle
**/
void precomputeTriangleValues(std::vector<Triangle>::iterator triangle, int offset)
{
	int i = 0;

	for (; i < offset && MyMesh.triangles.end() != triangle; ++triangle)
	{
		triangle->precomputeValues(MyMesh.vertices);
		i++;
	}
}

/**
* Print progress of ray tracing.
**/
void printProgress(std::vector<float>* progress, bool* rayTracingDone)
{
	while (!*rayTracingDone)
	{
		float progressPercentage = 100 * std::accumulate((*progress).begin(), (*progress).end(), 0.0f) / NUM_THREADS;

		std::cout << (((float)((int)(progressPercentage * 100)) / 100)) << " %" << std::endl;

#ifdef _WIN32
		Sleep(500);
#else
		usleep(500000);
#endif
	}

	std::cout << "100 %" << std::endl;
}

//void renderScene()
//{
//	cout << "Raytracing" << endl;
//
//	if (!headlessMode)
//	{
//		// Compute the frustum attributes
//		produceRay(0, 0, &origin00, &dest00);
//		produceRay(0, WindowSize_Y - 1, &origin01, &dest01);
//		produceRay(WindowSize_X - 1, 0, &origin10, &dest10);
//		produceRay(WindowSize_X - 1, WindowSize_Y - 1, &origin11, &dest11);
//	}
//
//	// Setup an image with the size of the current image.
//	Image result(WindowSize_X, WindowSize_Y);
//
//	// Vector to store threads
//	std::vector<std::thread> precomputeThreads;
//
//	// Precompute values for triangles
//	std::time_t startTimePrecompute = std::time(nullptr);
//
//	unsigned int trianglesPerThread = ceil((float)MyMesh.triangles.size() / NUM_THREADS);
//
//	for (unsigned int t = 0; t < NUM_THREADS; ++t) {
//
//		if ((t * trianglesPerThread) < MyMesh.triangles.size()) {
//
//			std::vector<Triangle>::iterator begin = MyMesh.triangles.begin() + (t * trianglesPerThread);
//
//			precomputeThreads.push_back(std::thread(
//				precomputeTriangleValues,
//				begin,
//				trianglesPerThread
//			));
//
//		}
//	}
//
//	// Join all ray tracing threads
//	for (std::vector<std::thread>::iterator thread = precomputeThreads.begin(); thread != precomputeThreads.end(); ++thread) {
//		thread->join();
//	}
//	std::cout << "Precomputing values of " << MyMesh.triangles.size() << " triangles took " << std::time(nullptr) - startTimePrecompute << " seconds" << std::endl;
//
//
//	// Starting time, used to display running time
//	std::time_t startTime = std::time(nullptr);
//
//	// Block size
//	unsigned int numberOfXPixelsInBlock = ceil((endX - startX) / NUM_BLOCKS_X);
//	unsigned int numberOfYPixelsInBlock = ceil((endY - startY) / NUM_BLOCKS_Y);
//
//	// Vector to store threads
//	std::vector<std::thread> threads;
//
//	// Vector to store progress of the threads
//	std::vector<float> progress(NUM_THREADS, 0.f);
//
//	bool rayTracingDone = false;
//
//	// Create thread that prints progress
//	std::thread progressThread(printProgress, &progress, &rayTracingDone);
//
//	// Create threads for the actual ray tracing
//	for (unsigned int xBlock = 0; xBlock < NUM_BLOCKS_X; ++xBlock) {
//
//		for (unsigned int yBlock = 0; yBlock < NUM_BLOCKS_Y; ++yBlock) {
//
//
//			threads.push_back(std::thread(
//				performRaytracingBlock,
//				startX + xBlock * numberOfXPixelsInBlock, // start x
//				startX + (xBlock + 1) * numberOfXPixelsInBlock, // end x
//				startY + yBlock * numberOfYPixelsInBlock, // start y
//				startY + (yBlock + 1) * numberOfYPixelsInBlock, // end y
//				origin00,
//				origin01,
//				origin10,
//				origin11,
//				dest00,
//				dest01,
//				dest10,
//				dest11,
//				&result,
//				xBlock * NUM_BLOCKS_Y + yBlock, // thread number
//				&progress
//			));
//
//
//		}
//
//	}
//
//
//	// Join all ray tracing threads
//	for (std::vector<std::thread>::iterator thread = threads.begin(); thread != threads.end(); ++thread) {
//		thread->join();
//	}
//
//	rayTracingDone = true;
//
//	// Store result
//	result.writeImage("result.ppm");
//
//	if (headlessMode)
//	{
//		Communication::sendImage(result);
//	}
//
//	progressThread.join();
//
//	// Print running time
//	std::cout << "The rendering took " << std::time(nullptr) - startTime << " seconds" << std::endl;
//}


/**
* React to keyboard input.
*/
void keyboard(unsigned char key, int x, int y)
{
	printf("key %d pressed at %d,%d\n", key, x, y);
	fflush(stdout);
	switch (key)
	{
	case 'u':
		{
			myDebugScreen.toggleRenderOverlay();
			break;
		}

		// Put all concurrent-dangerous operations below this guard please :)
		if (rendering)
			break;

	case 'k':
		{
			Vec3Df mag = Vec3Df(0.0, 0.0, 0.0) - getCameraPosition();
			cout << "Length to origin is: " << mag.getLength() << endl;
			break;
		}

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

	case 'o':
		{
			myDebugScreen.toggleOptics();
			break;
		}

	case 'O':
		{
			Vec3Df intersect, normal = Vec3Df(0.0, 0.0, 0.0);
			myDebugScreen.intersectWithSphere(1.0, Vec3Df(0.0, 0.0, 0.0), // Sphere data
			                                  Vec3Df(-3.0, 0.0, 0.0), // Ray data
			                                  Vec3Df(0.0, 0.5, 0.0),
			                                  intersect,
			                                  normal
			);
			cout << "Intersect: " << intersect << endl;
			cout << "normal: " << normal << endl;
			break;
		}

		// Click 'r'.
	case 'r':
		{
			renderStart();
			break;
		}

	case 'R':
		{
			realtime_rendering = !realtime_rendering;
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


void renderStart()
{
	if (rendering)
	{
		return;
	}
	else
	{
		rendering = true;
		Vec3Df origin00, dest00;
		Vec3Df origin01, dest01;
		Vec3Df origin10, dest10;
		Vec3Df origin11, dest11;

		produceRay(0, 0, &origin00, &dest00);
		produceRay(0, WindowSize_Y - 1, &origin01, &dest01);
		produceRay(WindowSize_X - 1, 0, &origin10, &dest10);
		produceRay(WindowSize_X - 1, WindowSize_Y - 1, &origin11, &dest11);

		thread sattelite(rayTraceStart, origin00, dest00,
		                 origin01, dest01,
		                 origin10, dest10,
		                 origin11, dest11
		);
		sattelite.detach();
	}
}


void rayTraceStart(Vec3Df origin00, Vec3Df dest00,
                   Vec3Df origin01, Vec3Df dest01,
                   Vec3Df origin10, Vec3Df dest10,
                   Vec3Df origin11, Vec3Df dest11
)
{
	rendering = true;
	cout << "Scaled render-plane to " << WindowSize_X << " x " << WindowSize_Y << endl;
	myCamera.sensor.resizeImagePlane(WindowSize_X, WindowSize_Y);

	// Pressing r will launch the raytracing.
	cout << "Raytracing" << endl;


	// Vector to store threads
	std::vector<std::thread> precomputeThreads;

	// Precompute values for triangles
	std::time_t startTimePrecompute = std::time(nullptr);

	unsigned int trianglesPerThread = ceil((float)MyMesh.triangles.size() / NUM_THREADS);

	for (unsigned int t = 0; t < NUM_THREADS; ++t)
	{
		if ((t * trianglesPerThread) < MyMesh.triangles.size())
		{
			std::vector<Triangle>::iterator begin = MyMesh.triangles.begin() + (t * trianglesPerThread);

			precomputeThreads.push_back(std::thread(
				precomputeTriangleValues,
				begin,
				trianglesPerThread
			));
		}
	}

	// Join all ray tracing threads
	for (std::vector<std::thread>::iterator thread = precomputeThreads.begin(); thread != precomputeThreads.end(); ++thread)
	{
		thread->join();
	}

	std::cout << "Precomputing values of " << MyMesh.triangles.size() << " triangles took " << std::time(nullptr) - startTimePrecompute << " seconds" << std::endl;

	// Starting time, used to display running time
	std::time_t startTime = std::time(nullptr);

	// Block size
	unsigned int numberOfXPixelsInBlock = ceil(WindowSize_X / NUM_BLOCKS_X);
	unsigned int numberOfYPixelsInBlock = ceil(WindowSize_Y / NUM_BLOCKS_Y);

	// Vector to store threads
	std::vector<std::thread> threads;

	// Vector to store progress of the threads
	std::vector<float> progress(NUM_THREADS, 0.f);

	bool rayTracingDone = false;

	// Create thread that prints progress
	std::thread progressThread(printProgress, &progress, &rayTracingDone);

	// Create threads for the actual ray tracing
	for (unsigned int xBlock = 0; xBlock < NUM_BLOCKS_X; ++xBlock)
	{
		for (unsigned int yBlock = 0; yBlock < NUM_BLOCKS_Y; ++yBlock)
		{
			threads.push_back(std::thread(
				performRaytracingBlock,
				xBlock * numberOfXPixelsInBlock, // start x
				(xBlock + 1) * numberOfXPixelsInBlock, // end x
				yBlock * numberOfYPixelsInBlock, // start y
				(yBlock + 1) * numberOfYPixelsInBlock, // end y
				origin00,
				origin01,
				origin10,
				origin11,
				dest00,
				dest01,
				dest10,
				dest11,
				&myCamera.sensor,
				xBlock * NUM_BLOCKS_Y + yBlock, // thread number
				&progress
			));
		}
	}


	// Join all ray tracing threads
	for (std::vector<std::thread>::iterator thread = threads.begin(); thread != threads.end(); ++thread)
	{
		thread->join();
	}

	rayTracingDone = true;
	progressThread.join();

	rendering = false;
}

/**
* Perform ray tracing for a certain block.
*/
void performRaytracingBlock(unsigned int xStart,
                            unsigned int xEnd,
                            unsigned int yStart,
                            unsigned int yEnd,
                            Vec3Df origin00,
                            Vec3Df origin01,
                            Vec3Df origin10,
                            Vec3Df origin11,
                            Vec3Df dest00,
                            Vec3Df dest01,
                            Vec3Df dest10,
                            Vec3Df dest11,
                            FilmPlane* sensor,
                            int threadNumber,
                            std::vector<float>* progress
)
{
	Vec3Df origin, dest;
	float localProgress;

	for (unsigned int y = yStart; y < yEnd && y < WindowSize_Y; ++y)
	{
		for (unsigned int x = xStart; x < xEnd && x < WindowSize_X; ++x)
		{
			// Progress indication
			localProgress = ((float)y - yStart) / (yEnd - yStart) + (1.0f / (yEnd - yStart)) * ((float)x / (xEnd - 1));
			(*progress)[threadNumber] = localProgress;

			// Produce the rays for each pixel, by interpolating the four rays of the frustum corners.
			float xscale = 1.0f - float(x) / (WindowSize_X - 1);
			float yscale = 1.0f - float(y) / (WindowSize_Y - 1);

			origin = yscale * (xscale * origin00 + (1 - xscale) * origin10) +
				(1 - yscale) * (xscale * origin01 + (1 - xscale) * origin11);
			dest = yscale * (xscale * dest00 + (1 - xscale) * dest10) +
				(1 - yscale) * (xscale * dest01 + (1 - xscale) * dest11);

			// Launch raytracing for the given ray.
			Vec3Df rgb = performRayTracing(origin, dest, 0);

			// Store the result in an image.
			sensor->setPixel(x, y, rgb);
		}
	}
}

//=======
//    printf("key %d pressed at %d,%d\n", key, x, y);
//    fflush(stdout);
//    switch (key)
//    {
//            
//            // Press 'L'.
//        case 'L':
//            MyLightPositions.push_back(getCameraPosition());
//            break;
//            
//            // Click 'l'.
//        case 'l':
//            MyLightPositions[MyLightPositions.size() - 1] = getCameraPosition();
//            break;
//            
//        case 'o':
//            
//            
//            produceRay(0, 0, &origin00, &dest00);
//            produceRay(0, WindowSize_Y - 1, &origin01, &dest01);
//            produceRay(WindowSize_X - 1, 0, &origin10, &dest10);
//            produceRay(WindowSize_X - 1, WindowSize_Y - 1, &origin11, &dest11);
//            
//            
//            cout
//            << "origin00[0] = " << origin00[0] << "; \n"
//            << "origin00[1] = " << origin00[1] << "; \n"
//            << "origin00[2] = " << origin00[2] << "; \n"
//            << "origin01[0] = " << origin01[0] << "; \n"
//            << "origin01[1] = " << origin01[1] << "; \n"
//            << "origin01[2] = " << origin01[2] << "; \n"
//            << "origin10[0] = " << origin10[0] << "; \n"
//            << "origin10[1] = " << origin10[1] << "; \n"
//            << "origin10[2] = " << origin10[2] << "; \n"
//            << "origin11[0] = " << origin11[0] << "; \n"
//            << "origin11[1] = " << origin11[1] << "; \n"
//            << "origin11[2] = " << origin11[2] << "; \n\n"
//            << endl;
//            
//            cout
//            << "dest00[0] = " << dest00[0] << "; \n"
//            << "dest00[1] = " << dest00[1] << "; \n"
//            << "dest00[2] = " << dest00[2] << "; \n"
//            << "dest01[0] = " << dest01[0] << "; \n"
//            << "dest01[1] = " << dest01[1] << "; \n"
//            << "dest01[2] = " << dest01[2] << "; \n"
//            << "dest10[0] = " << dest10[0] << "; \n"
//            << "dest10[1] = " << dest10[1] << "; \n"
//            << "dest10[2] = " << dest10[2] << "; \n"
//            << "dest11[0] = " << dest11[0] << "; \n"
//            << "dest11[1] = " << dest11[1] << "; \n"
//            << "dest11[2] = " << dest11[2] << "; \n"
//            << endl;
//            
//            
//            break;
//            
//            // Click 'r'.
//        case 'r':
//        {
//            // Pressing r will launch the raytracing.
//			renderScene();
//            break;
//        }
//            
//            // CLick 'Esc'.
//        case 27:
//            exit(0);
//    }
//    
//    // Produce the ray for the current mouse position.
//    Vec3Df testRayOrigin, testRayDestination;
//    produceRay(x, y, &testRayOrigin, &testRayDestination);
//    
//    yourKeyboardFunc(key, x, y, testRayOrigin, testRayDestination);
//}
//>>>>>>> master
