#pragma once
#include "LensElement.h"


LensElement::LensElement(float r, float t, float n, float v, float ap)
{
	radius = r;
	thickness = t;
	nd = n;
	v_number = v;
	aperture = ap;
}

LensElement::~LensElement()
{
}

float LensElement::getApertureRadius()
{
	return aperture / 2.0;
}
