#ifndef RAYTRACING_Hjdslkjfadjfasljf
#define RAYTRACING_Hjdslkjfadjfasljf

#ifndef _CAMERA
#define _CAMERA
#include "Camera.h"
#endif

#ifndef _DEBUG_SCREEN
#define _DEBUG_SCREEN
#include "DebugScreen.h"
#endif

#ifndef _LIGHTS
#define _LIGHTS
#include "Light.h"
#include "SpotLight.h"
#endif

#pragma once
#include <vector>
#include "mesh.h"
#include "imageWriter.h"



//Welcome to your MAIN PROJECT...
//THIS IS THE MOST relevant code for you!
//this is an important file, raytracing.cpp is what you need to fill out
//In principle, you can do the entire project ONLY by working in these two files

extern Mesh MyMesh; //Main mesh
extern std::vector<Vec3Df> MyLightPositions;
extern Vec3Df MyCameraPosition; //currCamera
extern unsigned int WindowSize_X;//window resolution width
extern unsigned int WindowSize_Y;//window resolution height
extern unsigned int RayTracingResolutionX;  // largeur fenetre
extern unsigned int RayTracingResolutionY;  // largeur fenetre
extern bool rendering;

//use this function for any preprocessing of the mesh.
void init(Camera * camera, DebugScreen * debugScreen);

//you can use this function to transform a click to an origin and destination
//the last two values will be changed. There is no need to define this function.
//it is defined elsewhere
void produceRay(int x_I, int y_I, Vec3Df & origin, Vec3Df & dest);


//your main function to rewrite
Vec3Df performRayTracing(const Vec3Df & origin, const Vec3Df & dest, int depth);

void lichtbak(Vec3Df origin, Vec3Df dest);

//a function to debug --- you can draw in OpenGL here
void yourDebugDraw();

//want keyboard interaction? Here it is...
void yourKeyboardFunc(char t, int x, int y, const Vec3Df & rayOrigin, const Vec3Df & rayDestination);

bool lightobstructed(const Vec3Df & origin, const Vec3Df & dest);
void clearAllLines();
void drawLine(Vec3Df origin, Vec3Df dest, Vec3Df color);
Vec3Df lambertshading(Vec3Df location, Vec3Df normal, Vec3Df origin, Vec3Df light, int material);
Vec3Df softshading(Vec3Df location, Vec3Df normal, Vec3Df origin, Light l, int material);

#endif