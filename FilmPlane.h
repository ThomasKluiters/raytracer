#pragma once
#include <stdlib.h>
#include <vector>
#include "Vec3D.h"


class FilmPlane
{



public:
	//hold RGB data
	std::vector< std::vector< Vec3Df > > imageData;

	FilmPlane(unsigned int heigth, unsigned int width);
	~FilmPlane();

	Vec3Df top_left, top_right;
	Vec3Df bottom_left, bottom_right;

	// Sets pixel-data in imageData array.
	void setPixel(unsigned int x, unsigned int y, Vec3Df colourData);
	void writeToDisk(std::string fileName);
	void initImagePlane();
	void resizeImagePlane(unsigned int width, unsigned int height);
	int getFilmHeight();
	int getFilmWidth();
	bool isReady();

private:
	bool ready;
	unsigned int height;
	unsigned int width;
	const Vec3Df baseColour = Vec3Df(0.0, 0.3, 0.8);	// Base colour of image
};


