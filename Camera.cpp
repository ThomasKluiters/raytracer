#pragma once
#include "Camera.h"

Camera::Camera(int width, int height, Vec3Df &cameraPos, Vec3Df up_vector,
				float fov, float aperture, float focaldistance)
{
	postInit(width, height, cameraPos, up_vector, fov, aperture, focaldistance);
}

Camera::Camera()
{
	
}

void Camera::postInit(int width, int height, Vec3Df& cameraPos, Vec3Df up_vector, float fov, float aperture, float focaldistance)
{
	// General options
	camPos = cameraPos;
	lookAtPos = Vec3Df(0.0, 0.0, 0.0);
	field_of_view = fov;
	up = up_vector;
	lensRadius = aperture;
	focalDistance = focaldistance;
	setFilmPlane(width, height);
	reCalcAUS();
}


void Camera::reCalcAUS()
{
	Vec3Df atDirection = lookAtPos - camPos;
	atDirection.normalize();
	Vec3Df world_up = Vec3Df(0.0, 1.0, 0.0);
	if (Vec3Df::dotProduct(world_up, atDirection) == 0) {
		world_up = Vec3Df(0.0, 0.0, 1.0);
	}
	side = Vec3Df::crossProduct(world_up, atDirection);
	up = Vec3Df::crossProduct(atDirection, side);
	side.normalize();
	up.normalize();
}

void Camera::setFilmPlane(int w, int h)
{
	sensor.initImagePlane();
	sensor.resizeImagePlane(w, h);
}

void Camera::traceThick()
{
	thickLens = true;
	thinLens = false;
	fullLens = false;
}

void Camera::traceThin()
{
	thickLens = false;
	thinLens = true;
	fullLens = false;
}

void Camera::traceFull()
{
	thickLens = false;
	thinLens = false;
	fullLens = true;
}

bool Camera::apertureIntersect(const Vec3Df & origin, const Vec3Df & dest)
{
	Vec3Df lensPos = camPos + lookAtDir*focalDistance;

	if (!diskIntersect(origin, dest, lookAtDir, lensPos, lensRadius)) {
		return false;
	}
	else {
		return true;
	}
}

// Given a vector in world space; transform that vector to this camera's camera space.
Vec3Df Camera::transformPoint(const Vec3Df & given)
{
	float x, y, z;
	float xr, yr, zr, wp;

	float m[4][4];

	/* Reference for knowing what goes where
	matrix[0][3] = camPos[0];	// Set the last collumn of the transformation matrix
	matrix[1][3] = camPos[1];
	matrix[2][3] = camPos[2];
	matrix[3][3] = 1;

	reCalcAUS();			// Recalculate the At-Up and Side vectors
	matrix[0][0] = side[0];	// Fill in at-up-side matrix entries...
	matrix[1][0] = side[2];
	matrix[2][0] = side[3];
	matrix[3][0] = 0.0;

	matrix[0][1] = up[0];
	matrix[1][1] = up[1];
	matrix[2][1] = up[2];
	matrix[3][1] = 0.0;


	matrix[0][2] = lookAtDir[0];
	matrix[1][2] = lookAtDir[1];
	matrix[2][2] = lookAtDir[2];
	matrix[3][2] = 0.0;
	*/

	xr = side[0] * x + up[0] * y + lookAtDir[0] * z + camPos[0];
	yr = side[2] * x + up[1] * y + lookAtDir[1] * z + camPos[1];
	zr = side[3] * x + up[2] * y + lookAtDir[2] * z + camPos[2];
	wp = side[2] * x + up[1] * y + lookAtDir[1] * z + camPos[1];
}

void Camera::transformCamera(const Vec3Df newPosition)
{
	camPos = newPosition;
	reCalcAUS();
}

void Camera::lookAt(const Vec3Df newPosition)
{
	lookAtPos = newPosition;
	reCalcAUS();
}

void Camera::traceLensRay(float x, float y, Vec3Df & origin, Vec3Df & destination)
{

	if (lensRadius > 0.0) {

		//Map ray to lens-point
		Vec3Df lensPoint = lensRadius * sampleLens(x, y);
		//Scale by lensradius
		lensPoint = lensRadius * lensPoint;

		// Determine multiplier from dest to plane of focus(z is constant on viewplane :) )
		float rayMultiplier = focalDistance / destination[2];
		// And then the point on the focus-plane
		Vec3Df focusPoint = destination * rayMultiplier;

		// Externally change the ray origin and destination according to above findings
		origin[0] = lensPoint[0];	// Origin x is now on lensPoint x
		origin[1] = lensPoint[1];	// Origin y is now on lenspoint y
		origin[2] = 0.0;		// Might be a huge mistake..?

		destination = focusPoint - origin;
		destination.normalize();
	}
}

bool Camera::planeIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos, float & res)
{
	float dot_normalLine = Vec3Df::dotProduct(pointB, planeNormal);
	if (dot_normalLine > 1e-5) {
		Vec3Df crossingVector = planePos - pointA;
		res = Vec3Df::dotProduct(crossingVector, planeNormal) / dot_normalLine;
		return (res >= 0);
	}
}

bool Camera::diskIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos, float radius)
{
	float dot_normalLine = Vec3Df::dotProduct(pointB, planeNormal);
	if (dot_normalLine > 1e-5) {
		Vec3Df crossingVector = planePos - pointA;
		float res = Vec3Df::dotProduct(crossingVector, planeNormal) / dot_normalLine;

		// If true; intersects the plane of the disk
		if (res >= 0) {
			Vec3Df pointOnPlane = pointA + res * pointB;
			Vec3Df vectorToPoint = pointOnPlane - planePos;
			float magnitude = Vec3Df::dotProduct(vectorToPoint, vectorToPoint);
			//disk test
			return (magnitude <= (radius * radius));
		}
	}
}

Vec3Df Camera::sampleLens(float a, float b)
{
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
		polarAngle = (M_PI / 2.0) - (M_PI / 4.0) * (v / rPolar);
	}
	return Vec3Df(cosf(polarAngle), sinf(polarAngle), 0.0);
}