#include "Camera.h"
#include "Vec3D.h"


Camera::Camera(Vec3Df &cameraPos, Vec3Df up_vector) 
{
	camPos = cameraPos;
	lookAtPos = Vec3Df(0.0,0.0,0.0);
	up = up_vector;
	lensRadius = 20.0;
	focalDistance = 40.0;
	reCalcWUV();
}