#pragma once
#include "Vec3D.h"
#include <random>

class Camera
{
public:
	Camera(Vec3Df &cameraPos, Vec3Df up_vector);
	Vec3Df camPos;			
	Vec3Df lookAtPos;			// Lookat Position!
	Vec3Df up;
	Vec3Df side;
	Vec3Df lookAtDir;

	//float shutterOpen;		// motion blurs
	//float shutterClose;		
	float lensRadius;			
	float focalDistance;	

	void reCalcWUV() {
		lookAtDir = lookAtPos - camPos;
		lookAtDir.normalize();
		up = (Vec3Df::crossProduct(side, lookAtDir));
		up.normalize();

		side = (Vec3Df::crossProduct(up, lookAtDir));
		side.normalize();
	}


	bool apertureIntersect(const Vec3Df & origin, const Vec3Df & dest) 
	{
		Vec3Df lensPos = camPos + lookAtDir*focalDistance;

		if (!diskIntersect(origin, dest, lookAtDir, lensPos, lensRadius)) {
			return false;
		}
		else {
			return true;
		}

	}

	// transform camera to given position
	void transformCamera(const Vec3Df newPosition) {
		camPos = newPosition;
		reCalcWUV();
	}


	// Update lookAt
	void lookAt(const Vec3Df newPosition) {
		lookAtPos = newPosition;
		reCalcWUV();
	}

	// Produces a ray, traced from the camera's film
	Vec3Df produceRay(const Vec3Df origin) {
		return origin - camPos;
	}

private:
	bool planeIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos)
	{
	
		float dot_normalLine = Vec3Df::dotProduct(pointB, planeNormal);
		if (dot_normalLine > 1e-5) {
			Vec3Df crossingVector = planePos - pointA;
			float res = Vec3Df::dotProduct(crossingVector, planeNormal) / dot_normalLine;
			return (res >= 0);
		}
	}

	bool diskIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos, float radius) 
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

};