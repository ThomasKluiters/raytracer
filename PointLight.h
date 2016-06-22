#pragma once
#include "Light.h"
#include "Vec3D.h"

class PointLight :
	public Light_I
{
public:
	PointLight::PointLight(Vec3Df position, Vec3Df lightColour, unsigned int samples);
};


