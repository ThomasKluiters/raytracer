#pragma once
#include "SpotLight.h"
#include "Light.h"
#include "Vec3D.h"


SpotLight::SpotLight(Vec3Df position, Vec3Df atPos, Vec3Df lightColour, unsigned int samples, float spotRadius)
	: Light_I(position, lightColour, samples)
{
	radius = spotRadius;
	pointAt(atPos);
}

// Transforms

// Returns normalized lookAt vector
Vec3Df SpotLight::getLookAt() {
	Vec3Df lookAt = diskPos - lightPosition;
	lookAt.normalize();
	return lookAt;
}

void SpotLight::pointAt(const Vec3Df at_position)
{
	atPosition = at_position;
	reCalcObject();
}

// Transforms and rebuilds this light at the given position.
void SpotLight::transformTo(Vec3Df newPosition)
{
	lightPosition = newPosition;
	pointAt(atPosition);
}

// Concatenates transform of lightpoint with a new point to look At.
void SpotLight::transformTo(Vec3Df newPosition, Vec3Df newLookAt)
{
	lightPosition = newPosition;
	pointAt(newLookAt);
}

// Updates all derived orientations and positions related to this object.
void SpotLight::reCalcObject() {
	Vec3Df atDirection = getLookAt();								// Determine at-direction
	Vec3Df world_up = Vec3Df(0.0, 1.0, 0.0);
	if (Vec3Df::dotProduct(world_up, atDirection) == 0) {			// If (0,1,0) is parallel with at; fall back to (0,0,1)
		world_up = Vec3Df(0.0, 0.0, 1.0);
	}
	side = Vec3Df::crossProduct(world_up, atDirection);				
	up = Vec3Df::crossProduct(atDirection, side);
	side.normalize();
	up.normalize();
	diskPos = lightPosition + getLookAt();								// Update the disk position
}

// Intersections <!> Request integration with SIMD branch

// Intersection-test with the disk defined on this spotlight.
bool SpotLight::intersectsLightCone(const Vec3Df & suface_point)
{
	Vec3Df atDirection = getLookAt();
	float dot_normalLine = Vec3Df::dotProduct(lightPosition, atDirection);
	if (dot_normalLine > 1e-6) {
		Vec3Df crossingVector =  diskPos - suface_point;
		float res = Vec3Df::dotProduct(crossingVector, atDirection) / dot_normalLine;

		// If true; intersects the plane of the disk
		if (res >= 0) {
			Vec3Df pointOnPlane = suface_point + res * lightPosition;
			Vec3Df vectorToPoint = pointOnPlane - diskPos;
			float magnitude = Vec3Df::dotProduct(vectorToPoint, vectorToPoint);
			//disk test
			return (magnitude <= (radius * radius));
		}
	}
}


SpotLight::~SpotLight()
{
}
