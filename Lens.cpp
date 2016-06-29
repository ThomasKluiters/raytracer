#pragma once
#include "Lens.h"
#include <stdlib.h>

// Default lens
Lens::Lens()
{
	setTele();
}


Lens::~Lens()
{
}

void Lens::setDGauss()
{
	focalLength = 50.0;
	int dGaussSize = 11;
	lensElements.resize(dGaussSize);
	// Default lens   50mm       radius  thick   nd    V-no  Aperture
	LensElement f0 = LensElement(58.950, 7.520, 1.670, 47.1, 50.4);
	LensElement f1 = LensElement(169.660, 0.240, 0.0, 0.0, 50.4);
	LensElement f2 = LensElement(38.550, 8.050, 1.670, 47.1, 46.0);
	LensElement f3 = LensElement(81.540, 6.550, 1.699, 30.1, 46.0);
	LensElement f4 = LensElement(25.500, 11.410, 0.0, 0.0, 36.0);
	LensElement f5 = LensElement(0.0, 9.0, 0.0, 0.0, 34.2);		// Diaphragm
	LensElement f6 = LensElement(-28.990, 2.360, 1.603, 38.0, 34.0);
	LensElement f7 = LensElement(81.540, 12.130, 1.658, 57.3, 40.0);
	LensElement f8 = LensElement(-40.770, 0.380, 0.0, 0.0, 40.0);
	LensElement f9 = LensElement(874.130, 6.440, 1.717, 48.0, 40.0);
	LensElement f10 = LensElement(-79.460, 72.228, 0.0, 0.0, 40.0);
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
}

void Lens::setFishEye() 
{
	// Initial focal point is 10mm
	int fishEyeSize = 12;
	focalLength = 10.0;
	lensElements.clear();
	lensElements.resize(fishEyeSize);
	// Default lens   10 mm      radius     thick     nd    V-no  Aperture
	LensElement f0 = LensElement(30.2249,	0.8335,	 1.62,  0.0,	30.34); //
	LensElement f1 = LensElement(11.3931,	7.4136,	 1.0,   0.0,	20.68); //
	LensElement f2 = LensElement(75.2019,	1.0654,	 1.639, 0.0,	17.8);  //
	LensElement f3 = LensElement(8.3349,	11.1549, 1.0,   0.0,    13.42); //
	LensElement f4 = LensElement(9.5882,	2.0054,  1.654, 0.0,	9.02); //
	LensElement f5 = LensElement(43.8677,	5.3895,	 1.0,   0.0,	8.14); //
	LensElement f6 = LensElement(0.0,	    1.4163,  0.0,   0.0,	6.08);   // Diaphragm
	LensElement f7 = LensElement(29.4541,	2.1934,	 1.517, 0.0,	5.96); //
	LensElement f8 = LensElement(-5.2265,	0.9714,	 1.805, 0.0,	5.84); //
	LensElement f9 = LensElement(-14.2884,	0.0627,	 1.0,   0.0,	5.96); //
	LensElement f10 = LensElement(-22.3726,	0.94,	 1.673, 0.0,	5.96); //
	LensElement f11 = LensElement(-15.0404,	0.0,	 1.0,   0.0, 	6.52); //?
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

void Lens::setTele() 
{
	int teleLensSize = 7;
	focalLength = 250.0;
	//lensElements.clear();
	//lensElements.resize(teleLensSize);
	// Default lens    250 mm     radius     thick     nd    V-no  Aperture
	LensElement f0 = LensElement(54.6275,	12.52,	1.529, 0.0, 	47.5);
	LensElement f1 = LensElement(-86.365,	3.755,	1.599, 0.0, 	44.5);
	LensElement f2 = LensElement(271.7625,	2.8175,	1.0,   0.0, 	41.5);  
	LensElement f3 = LensElement(0.0, 	    67.4125, 0.0,  0.0,	    44.5); 
	LensElement f4 = LensElement(-32.13,	3.755,	1.613, 0.0, 	31.5); 
	LensElement f5 = LensElement(49.5325,	12.52,	1.603, 0.0, 	33.5); 
	LensElement f6 = LensElement(-50.945,	0.0,	1.0,   0.0,     37.0);  
	lensElements.push_back(f0);
	lensElements.push_back(f1);
	lensElements.push_back(f2);
	lensElements.push_back(f3);
	lensElements.push_back(f4);
	lensElements.push_back(f5);
	lensElements.push_back(f6);
}


const std::vector<LensElement>::const_iterator Lens::itFront()
{
	return lensElements.cbegin();
}

const std::vector<LensElement>::const_iterator Lens::itEnd()
{
	return lensElements.cend();
}
