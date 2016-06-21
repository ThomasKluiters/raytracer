#pragma once
#include "Light.h"
#include "PointLight.h"
#include "Vec3D.h"


PointLight::PointLight(Vec3Df position, Vec3Df lightColour, unsigned int samples)
	: Light_I(position, lightColour, samples)
{	
}


