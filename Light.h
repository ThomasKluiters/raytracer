#pragma once
#include "Vec3D.h"
#include <stdlib.h>


/**
 * A Light_I object has a location and a samples-setting.
 */
class Light_I
{
protected:
	int nsamples;
	Vec3Df lightColour;
	Vec3Df lightPosition;
	Light_I(Vec3Df position, Vec3Df lightColour, unsigned int samples);

public:
	virtual Vec3Df getPosition();
	virtual void setPosition(Vec3Df position);
	virtual const Vec3Df& getLightColour();
	virtual void setIntensity(Vec3Df i);
	virtual void setSamples(unsigned int samples);
	virtual int getSamples();
};
