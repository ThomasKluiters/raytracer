#pragma once
#ifndef _IMAGE_WRITER
#define _IMAGE_WRITER
#include "ImageWriter.h"
#endif
#include "FilmPlane.h"


FilmPlane::FilmPlane(unsigned int w, unsigned int h)
{
	height = h;
	width = w;
	initImagePlane();
}


FilmPlane::~FilmPlane()
{

}

void FilmPlane::setPixel(unsigned int x, unsigned int y, Vec3Df colourData)
{
	imageData[x][y] = colourData;
}

void FilmPlane::writeToDisk(std::string fileName)
{
	Image result(width, height);
	float r, g, b;
	for (int i = 0; i < imageData.size(); i++) {
		for (int k = 0; k < imageData[0].size(); k++) {
			r = imageData[i][k][0];
			g = imageData[i][k][1];
			b = imageData[i][k][2];
			result.setPixel(i, k, RGBValue(r,g,b));
		}
	}
	result.writeImage(fileName.data());
}

/**
 * Fills this 2D array with the base-colour
 */
void FilmPlane::initImagePlane()
{//unsigned int width, unsigned int height
	if(height > 0 && width > 0) 
	{
		imageData.resize(width);
		for (int i = 0; i < width; i++) {
			imageData[i].resize(height);
			std::fill(imageData[i].begin(), imageData[i].end(), baseColour);
		}
	}
	else 
	{
		std::cout << " WARNING: ILLEGAL DATA PASSED TO FILMPLANE" << std::endl;
		return;
	}
}

/** Resizes this imageplane */
void FilmPlane::resizeImagePlane(unsigned int w, unsigned int h)
{
	if (w  <= 0 || h <= 0) {
		std::cout << " WARNING: ILLEGAL DATA PASSED TO FILMPLANE" << std::endl;
		return;
	}
	if (w != width) {		// Resize if changed
		width = w;
	}
	if (h != height) {	// Resize if changed
		height = h;
	}
	initImagePlane();
}

int FilmPlane::getFilmHeight()
{
	return imageData.size();
}

int FilmPlane::getFilmWidth()
{
	return imageData[0].size();
}
