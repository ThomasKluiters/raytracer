#pragma once
#include "Light.h"
#include "Vec3D.h"
#include "AreaLight.h"



AreaLight::AreaLight(Vec3Df position, Vec3Df atPos, Vec3Df lightColour, unsigned int samples, float l_height, float l_width)
	: Light_I(position, lightColour, samples)
{
	pointAt(atPos);
	height = l_height;
	width = l_width;
}
/*
bool AreaLight::intersects(const Vec3Df & surface_point) {
	float dot_normalLine = Vec3Df::dotProduct(pointB, atDirection);
	if (dot_normalLine > 1e-5) {
		Vec3Df crossingVector = planePos - pointA;
		res = Vec3Df::dotProduct(crossingVector, planeNormal) / dot_normalLine;
		return (res >= 0);
	}
}
*/

bool AreaLight::inPlane(const Vec3Df & point)
{
	Vec3Df v_to_center = point - lightPosition;
	
	//return v_to_center.getSquaredLength() <= 
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
