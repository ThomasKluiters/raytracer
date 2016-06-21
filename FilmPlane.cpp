#include "FilmPlane.h"



FilmPlane::FilmPlane(unsigned int h, unsigned int w)
{
	height = h;
	width = w;
}


FilmPlane::~FilmPlane()
{
}

void FilmPlane::writeToDisk(std::string fileName)
{
	Image result(width, height);
	for (int i = 0; i < imageData.size(); i++) {
		for (int k = 0; k < imageData[0].size(); k++) {
			result.setPixel(i, k, imageData[i][k]);
		}
	}
	result.writeImage(fileName.data());
}
