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
#include "raytracing.h"
#include "mesh.h"
#include "traqueboule.h"
#include "imageWriter.h"
#include "DebugScreen.h"

/**
 *  RENDERING CONSTANTS
 */

const std::string SCENE_FILE = "dodgeColorTest.obj";




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

typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint;
typedef std::chrono::high_resolution_clock ProfileClock;
typedef std::chrono::duration<double> TimeDiff;

Mesh MyMesh;						// Main mesh
unsigned int WindowSize_X = 800;	// X-resolution
unsigned int WindowSize_Y = 500;	// Y-resolution
void * Debug_font = GLUT_BITMAP_9_BY_15;

DebugScreen myDebugScreen = DebugScreen(SCENE_FILE, &WindowSize_X, &WindowSize_Y, Debug_font);

// Only used to catch the timeDiff once.
bool catchTime = true;

#define NUM_THREADS 18              // Max number of threads
#define NUM_BLOCKS_X 6              // Number of blocks in x direction
#define NUM_BLOCKS_Y 3              // Number of blocks in y direction

/**
 * Drawing function, which draws an image (frame) on the screen.
 */
void drawFrame()
{
	// Updates the windowsize
	WindowSize_X = glutGet(GLUT_WINDOW_WIDTH);
	WindowSize_Y = glutGet(GLUT_WINDOW_HEIGHT);

	yourDebugDraw();
	myDebugScreen.drawDebugInfo();
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
void keyboard(unsigned char key, int x, int y);

/**
 * Main function.
 */
int main(int argc, char** argv)
{
	TimePoint start, end;
	start = ProfileClock::now();

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
    
    init(&myDebugScreen);

	end = ProfileClock::now();
	TimeDiff test_duration = end - start;

	if (catchTime) {
		myDebugScreen.putChrono("Load_main", test_duration);
		catchTime = false;

		// Initialize debug-screen values.
		myDebugScreen.putBool("Some bool", false);
		// Explicit cast to string; otherwise treated as bool
		myDebugScreen.putString("Scene file", myDebugScreen.sceneData);
		myDebugScreen.putVector("Camera position", &MyCameraPosition);
		//myDebugScreen.put("Some Value", 23);

	}
		

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
    gluPerspective(50, (float)w / h, 0.01, 10);
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


/**
 * Perform ray tracing for a certain block.
 */
void performRaytracingBlock( unsigned int xStart,
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
                             Image *result,
                             int threadNumber,
                             std::vector<float> *progress
                             ) {
    
    Vec3Df origin, dest;
    float localProgress;
    
    for (unsigned int y = yStart; y < yEnd && y < WindowSize_Y; ++y) {
        
        for (unsigned int x = xStart; x < xEnd && x < WindowSize_X; ++x)
        {
            // Progress indication
            localProgress = ( (float) y - yStart) / (yEnd - yStart) + (1.0f / (yEnd - yStart)) * ( (float) x / (xEnd - 1) );
            (*progress)[threadNumber] = localProgress;
            
            // Produce the rays for each pixel, by interpolating the four rays of the frustum corners.
            float xscale = 1.0f - float(x) / (WindowSize_X - 1);
            float yscale = 1.0f - float(y) / (WindowSize_Y - 1);
            
            origin = yscale*(xscale*origin00 + (1 - xscale)*origin10) +
            (1 - yscale)*(xscale*origin01 + (1 - xscale)*origin11);
            dest = yscale*(xscale*dest00 + (1 - xscale)*dest10) +
            (1 - yscale)*(xscale*dest01 + (1 - xscale)*dest11);
            
            // Launch raytracing for the given ray.
            Vec3Df rgb = performRayTracing(origin, dest);
            
            // Store the result in an image.
            result->setPixel(x, y, RGBValue(rgb[0], rgb[1], rgb[2]));
        }
    }
    
}



/**
* Print progress of ray tracing.
**/
void printProgress(std::vector<float> *progress, bool *rayTracingDone) {

	myDebugScreen.traceUpdate(0.0);

    while(! *rayTracingDone) {
        float progressPercentage = 100 * std::accumulate((*progress).begin(), (*progress).end(), 0.0f) / NUM_THREADS;
        
        std::cout << (((float)((int)(progressPercentage * 100)) / 100)) << " %" << std::endl;
        
		myDebugScreen.traceUpdate(progressPercentage);

#ifdef _WIN32
		Sleep(500);
#else
		usleep(500000);
#endif
    }
    
    std::cout << "100 %" << std::endl;

	myDebugScreen.traceEnd();
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
            
            // Click 'r'.
        case 'r':
        {
            // Pressing r will launch the raytracing.
            cout << "Raytracing" << endl;
            
            // Setup an image with the size of the current image.
            Image result(WindowSize_X, WindowSize_Y);
            
            // Produce the rays for each pixel, by first computing the rays for the corners of the frustum.
            Vec3Df origin00, dest00;
            Vec3Df origin01, dest01;
            Vec3Df origin10, dest10;
            Vec3Df origin11, dest11;
            
            produceRay(0, 0, &origin00, &dest00);
            produceRay(0, WindowSize_Y - 1, &origin01, &dest01);
            produceRay(WindowSize_X - 1, 0, &origin10, &dest10);
            produceRay(WindowSize_X - 1, WindowSize_Y - 1, &origin11, &dest11);
            
            // Starting time, used to display running time
            std::time_t startTime = std::time(nullptr);
            
            // Block size
            unsigned int numberOfXPixelsInBlock = ceil(WindowSize_X / NUM_BLOCKS_X);
            unsigned int numberOfYPixelsInBlock = ceil(WindowSize_Y / NUM_BLOCKS_Y);
            
            // Vector to store threads
            std::vector<std::thread> threads;
            
            // Vector to store progress of the threads
            std::vector<float> progress (NUM_THREADS, 0.f);
            
            bool rayTracingDone = false;
            
            // Create thread that prints progress
            std::thread progressThread(printProgress, &progress, &rayTracingDone);
            
            // Create threads for the actual ray tracing
            for (unsigned int xBlock = 0; xBlock < NUM_BLOCKS_X; ++xBlock) {
                
                for (unsigned int yBlock = 0; yBlock < NUM_BLOCKS_Y; ++yBlock) {
                
                
                    threads.push_back(std::thread(
                                     performRaytracingBlock,
                                     xBlock * numberOfXPixelsInBlock, // start x
                                     (xBlock+1) * numberOfXPixelsInBlock, // end x
                                     yBlock * numberOfYPixelsInBlock, // start y
                                     (yBlock+1) * numberOfYPixelsInBlock, // end y
                                     origin00,
                                     origin01,
                                     origin10,
                                     origin11,
                                     dest00,
                                     dest01,
                                     dest10,
                                     dest11,
                                     &result,
                                     xBlock * NUM_BLOCKS_Y + yBlock, // thread number
                                     &progress
                    ));
                    
                    
                }
             
            }
            
            
            // Join all ray tracing threads
            for (std::vector<std::thread>::iterator thread = threads.begin(); thread != threads.end(); ++thread) {
                thread->join();
            }
            
            rayTracingDone = true;
            
            // Store result
            result.writeImage("result.ppm");
            
            progressThread.join();
            
            // Print running time
            std::cout << "The rendering took " << std::time(nullptr) - startTime << " seconds" << std::endl;
            
            break;
        }
            
            // CLick 'Esc'.
        case 27:
            exit(0);
    }
    
    // Produce the ray for the current mouse position.
    Vec3Df testRayOrigin, testRayDestination;
    produceRay(x, y, &testRayOrigin, &testRayDestination);
    
    yourKeyboardFunc(key, x, y, testRayOrigin, testRayDestination);
}