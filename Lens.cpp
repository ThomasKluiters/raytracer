#pragma once
#include "Lens.h"


// Default lens
Lens::Lens()
{
	// Default lens              radius  thick   nd    V-no  Aperture
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


Lens::~Lens()
{
}

void Lens::addLensElement(LensElement element)
{
	lensElements.push_back(element);
}

void Lens::setFishEye() {
	int fishEyeSize = 12;
	/*
# radius	sep	n	aperture
30.2249	0.8335	1.62	30.34
11.3931	7.4136	1		20.68
75.2019	1.0654	1.639	17.8
8.3349	11.1549	1		13.42
9.5882	2.0054	1.654	9.02
43.8677	5.3895	1		8.14
0	1.4163	0			6.08
29.4541	2.1934	1.517	5.96
-5.2265	0.9714	1.805	5.84
-14.2884	0.0627	1	5.96
-22.3726	0.94	1.673	5.96
-15.0404	0	1	6.52*/
	lensElements.clear();
	lensElements.resize(fishEyeSize);
	// Default lens              radius  thick   nd    V-no  Aperture
	LensElement f0 = LensElement(30.2249,	0.8335,	 1.62,  0.0,	30.34); //
	LensElement f1 = LensElement(11.3931,	7.4136	 1.0,   0.0,	20.68); //
	LensElement f2 = LensElement(75.2019,	1.0654,	 1.639, 0.0,	17.8);  //
	LensElement f3 = LensElement(8.3349,	11.1549, 1.0,   0.0,    13.42); //
	LensElement f4 = LensElement(25.500, 11.410, 0.0, 0.0, 36.0);
	LensElement f5 = LensElement(0.0, 9.0, 0.0, 0.0, 34.2);		// Diaphragm
	LensElement f6 = LensElement(-28.990, 2.360, 1.603, 38.0, 34.0);
	LensElement f7 = LensElement(81.540, 12.130, 1.658, 57.3, 40.0);
	LensElement f8 = LensElement(-40.770, 0.380, 0.0, 0.0, 40.0);
	LensElement f9 = LensElement(874.130, 6.440, 1.717, 48.0, 40.0);
	LensElement f10 = LensElement(-79.460, 72.228, 0.0, 0.0, 40.0);
	LensElement f11 = LensElement(-79.460, 72.228, 0.0, 0.0, 40.0);
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


const std::vector<LensElement>::const_iterator Lens::itFront()
{
	return lensElements.cbegin();
}

const std::vector<LensElement>::const_iterator Lens::itEnd()
{
	return lensElements.cend();
}
