#pragma once
#include "Light.h"
#include "Vec3D.h"

void Light_I::setPosition(Vec3Df position)
{
	lightPosition = position;
}

const Vec3Df& Light_I::getLightColour()
{
	return lightColour;
}

void Light_I::setIntensity(Vec3Df colours)
{
	lightColour = colours;
}

void Light_I::setSamples(unsigned int samples)
{
	nsamples = samples;
}

int Light_I::getSamples() {
	return nsamples;
}

Light_I::Light_I(Vec3Df position, Vec3Df l_colour, unsigned int samples)
{
	lightPosition = position;
	nsamples = samples;
	lightColour = l_colour;
}

Vec3Df Light_I::getPosition()
{
	return lightPosition;
}

