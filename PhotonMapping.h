#pragma once

#include "Vec3D.h"

struct Photon {
	
	Vec3Df position;
	Vec3Df power;
	
	float phi, theta;

	short flag;

};

class PhotonMap
{

};

class PhotonMapBuilder
{
public:
	std::vector<Photon> photons;

	void add(Photon & photon)
	{
		photons.push_back(photon);
	}

};