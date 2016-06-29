#pragma once
#include "Light.h"
#include "Vec3D.h"
#include "AreaLight.h"
#include <vector>


AreaLight::AreaLight(Vec3Df position, Vec3Df atPos, Vec3Df lightColour, unsigned int samples, float l_height, float l_width)
	: Light_I(position, lightColour, samples)
{
	pointAt(atPos);
	height = l_height;
	width = l_width;
}

bool AreaLight::planeIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos, float & res)
{
	float dot_normalLine = Vec3Df::dotProduct(pointB, planeNormal);
	if (dot_normalLine > 1e-5) {
		Vec3Df crossingVector = planePos - pointA;
		res = Vec3Df::dotProduct(crossingVector, planeNormal) / dot_normalLine;
		return (res >= 0);
	}
}
/*
std::vector<Vec3Df> AreaLight::getSamplePoints()
{
	int t0 = ceil(height / getSamples());
	int t1 = ceil(width / getSamples());
	std::vector<Vec3Df> points;
	points.resize(t0 * t1);
	for (int i = 0; i < t0; i++) {
		for (int p = 0; p < t1; p++) {
			float d1, d2;
			d1 = rand() / RAND_MAX;
			d2 = rand() / RAND_MAX;
			points[i + p] = Vec3Df();
		}
	}

}*/


bool AreaLight::inPlane(const Vec3Df & point)
{
	Vec3Df v_to_center = point - lightPosition;
	
	return v_to_center.getSquaredLength() <= height * height + width * width;
}

//Note, the plane of lighting is exactly AT the lightposition; not hovering before it
void AreaLight::pointAt(const Vec3Df & at_position)
{
	atDirection = at_position - lightPosition;
	atDirection.normalize();
	calculateUS();
}

// Transforms the arealight to the new position.
void AreaLight::transformTo(const Vec3Df & newPosition) 
{
	lightPosition = newPosition;
	pointAt((newPosition + atDirection));
}

// Sets the plane of the AreaLight
// Essentially just scales the side-up vectors
void AreaLight::setPlane(float l_height, float l_width)
{
	height = abs(l_height);
	width = abs(l_width);
	side = side * width;
	up = up * height;
}

// Calculates the new side-up vectors.
void AreaLight::calculateUS() {
	Vec3Df world_up = Vec3Df(0.0, 1.0, 0.0);
	if (Vec3Df::dotProduct(world_up, atDirection) == 0) {
		world_up = Vec3Df(0.0,0.0,1.0);
	}
	else {

		side = Vec3Df::crossProduct(world_up, atDirection);
		up = Vec3Df::crossProduct(atDirection, side);
		side.normalize();
		up.normalize();
	}
}

AreaLight::~AreaLight()
{
}
