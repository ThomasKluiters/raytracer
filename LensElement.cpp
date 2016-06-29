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

LensElement::LensElement() {
	radius = 0.0;
	thickness = 0.0;
	nd = 0.0;
	v_number = 0.0;
	aperture = 0.0;

}

LensElement::~LensElement()
{
}

float LensElement::getApertureRadius()
{
	return aperture / 2.0;
}
