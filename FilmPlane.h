#pragma once
#include "ImageWriter.h"
#include <stdlib.h>
#include <vector>
#include "Vec3D.h"


class FilmPlane
{



public:
	//hold RGB data
	std::vector< std::vector< RGBValue > > imageData;

	FilmPlane(unsigned int heigth, unsigned int width);
	~FilmPlane();

	Vec3Df top_left, top_right;
	Vec3Df bottom_left, bottom_right;

	void writeToDisk(std::string fileName);

private:
	unsigned int height;
	unsigned int width;

};


