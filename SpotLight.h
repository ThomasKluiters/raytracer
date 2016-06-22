#pragma once
#include "Light.h"
#include "Vec3D.h"

class SpotLight :
	public Light_I
{
public:
	float radius;
	Vec3Df atPosition;
	Vec3Df diskPos;			// Pre-computed diskLocation (1 unit from lightpos)
	Vec3Df up;				// Up / Side vectors		
	Vec3Df side;



	SpotLight(Vec3Df position, Vec3Df atPosition, 
				Vec3Df lightColour, unsigned int samples, float spotRadius);
	
	// Given a surface-point; indicate whether that surfacepoint hits the cone of this light at all.
	bool intersectsLightCone(const Vec3Df & surface_point);		// Tests whether the shadow-ray is inside of the lightcone.
	void pointAt(Vec3Df newLookAt);								// Points the spotlight at the location.
	void transformTo(Vec3Df newPosition);						// Translates the spotlight to a location.
	void transformTo(Vec3Df newPosition, Vec3Df newLookat);		// Translates to location and points at new location.
	Vec3Df getLookAt();											// Get the lookAt direction vector
	void reCalcObject();	
	// TODO light samples
	
	~SpotLight();
private:	
	
};

