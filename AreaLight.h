#ifndef _LIGHT_AREA
#define _LIGHT_AREA
#include "Light.h"
#include "Vec3D.h"
#endif

class AreaLight :
	public Light_I
{
private:
	float height;
	float width;
	Vec3Df atDirection;
	Vec3Df up;
	Vec3Df side;

public:
	AreaLight(Vec3Df position, Vec3Df atPos, Vec3Df lightColour, unsigned int samples, float height, float width);
	bool planeIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos, float & res);
//	std::vector<Vec3Df> getSamplePoints();
	bool inPlane(const Vec3Df & point);		// Returns whether the given point is a linear combination of height and width;
	void pointAt(const Vec3Df & at_position);					// Points the  areaLight at lookAt position
	void setPlane(float height, float width);
	void AreaLight::transformTo(const Vec3Df & newPosition);
	void calculateUS();
	~AreaLight();
};

/*

SpotLight::SpotLight(Vec3Df position, Vec3Df atPos, float lightColour, unsigned int samples, float spotRadius)
: Light_I(position, lightColour, samples)
{
pointAt(atPos);
radius = spotRadius;
}

*/
