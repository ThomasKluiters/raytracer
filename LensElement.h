#pragma once
class LensElement
{
public:
	float radius;
	float thickness;
	float nd;
	float v_number;
	float aperture;

	LensElement(float radius, float thickness, float nd, float v_number, float aperture);

	~LensElement();

	float LensElement::getApertureRadius();

};

