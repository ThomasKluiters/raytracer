#ifndef _FILM_PLANE
#define _FILM_PLANE
#include "FilmPlane.h"
#endif

#pragma once
#include "Vec3D.h"
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include "Lens.h"
#include "LensElement.h"


using namespace std;
// - Used PBRT book for reference! 


class Camera
{
private: 
	

public:
	Camera(int width, int height, Vec3Df &cameraPos, Vec3Df up_vector, 
			float fov, float aperture, float focaldistance);
	Vec3Df camPos;			
	Vec3Df lookAtPos;			// Lookat Position!
	Vec3Df lookAtDir;			// Lookat direction (normalized)
	Vec3Df up;
	Vec3Df side;

	// Image sensor properties
	FilmPlane sensor = FilmPlane(1, 1);

	// Rendermodes
	bool thinLens;
	bool thickLens;
	bool fullLens;

	float shutterOpen;		// motion blurs
	float shutterClose;		

	float lensRadius;		// DoF	
	float focalDistance;	
	float field_of_view;	// field of view in ANGLES
	
	// Precomputed values for FOV transform
	float fovX;				//Horizontal field of view in RADIANS
	float fovY;				// Vertical fov derived from ^
	float tan_fovX;			// precomputed tan
	float tan_fovY;			// ^


	// (Re-)calculate the At, Up and Side vectors
	void reCalcAUS();

	void setFilmPlane(int width, int height);

	void traceThick();

	void traceThin();

	void traceFull();
	
	bool apertureIntersect(const Vec3Df & origin, const Vec3Df & dest);

	// Camera transforms

	Vec3Df Camera::transformPoint(const Vec3Df & given);

	// transform camera to given position
	void transformCamera(const Vec3Df newPosition);


	// Update lookAt
	void lookAt(const Vec3Df newPosition);

	// Modifies the given initial eyeray by this camera's lens
	// Does nothing if the lensradius is less than or equal to 0
	// Changes ray by reference
	void traceLensRay(float x, float y, Vec3Df & origin, Vec3Df & destination);

private:
	bool planeIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos, float & res);
	
	bool diskIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos, float radius);

	// Uses monte-carlo based concentric mapping to determine a point on this camera's lens
	// Feed it random integers
	Vec3Df sampleLens(float a, float b);


};