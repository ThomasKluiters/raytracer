#include "Lens.h"


// Default lens
Lens::Lens()
{
	// Default lens
	LensElement f0 = LensElement(58.950, 7.520, 1.670, 47.1, 50.4);
	LensElement f1 = LensElement(169.660, 0.240, -1.0, -1.0, 50.4);
	LensElement f2 = LensElement(38.550, 8.050, 1.670, 47.1, 46.0);
	LensElement f3 = LensElement(81.540, 6.550, 1.699, 30.1, 46.0);
	LensElement f4 = LensElement(25.500, 11.410, -1.0, -1.0, 36.0);
	LensElement f5 = LensElement(0.0, 9.0, -1.0, -1.0, 34.2);		// Diaphragm
	LensElement f6 = LensElement(-28.990, 2.360, 1.603, 38.0, 34.0);
	LensElement f7 = LensElement(81.540, 12.130, 1.658, 57.3, 40.0);
	LensElement f8 = LensElement(-40.770, 0.380, -1.0, -1.0, 40.0);
	LensElement f9 = LensElement(81.540, 12.130, 1.658, 57.3, 40.0);
	LensElement f10 = LensElement(874.130, 6.440, 1.717, 48.0, 40.0);
	LensElement f11 = LensElement(-79.460, 72.228, -1.0, -1.0, 40.0);
	lensElements.push_back(f0);
	lensElements.push_back(f1);
	lensElements.push_back(f2);
	lensElements.push_back(f3);
	lensElements.push_back(f4);
	lensElements.push_back(f5);
	lensElements.push_back(f6);
	lensElements.push_back(f7);
	lensElements.push_back(f8);
	lensElements.push_back(f9);
	lensElements.push_back(f10);
	lensElements.push_back(f11);
}


Lens::~Lens()
{
}

void Lens::addLensElement(LensElement element)
{
	lensElements.push_back(element);
}


const std::vector<LensElement>::const_iterator Lens::itFront()
{
	return lensElements.cbegin();
}

const std::vector<LensElement>::const_iterator Lens::itEnd()
{
	return lensElements.cend();
}
