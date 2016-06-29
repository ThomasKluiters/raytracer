#pragma once
#include <stdlib.h>
#include "LensElement.h"
#include <vector>


class Lens
{
public:
	float focalLength;
	Lens();
	~Lens();
	std::vector< LensElement > lensElements;
	void setDGauss();
	void setFishEye();
	void setTele();
	const std::vector< LensElement >::const_iterator itFront();
	const std::vector< LensElement >::const_iterator itEnd();
private:
	
};

