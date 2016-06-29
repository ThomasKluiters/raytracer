#pragma once
#include <stdlib.h>
#include "LensElement.h"
#include <vector>


class Lens
{
public:

	Lens();
	~Lens();
	void addLensElement(LensElement element);
	void setFishEye();
	const std::vector< LensElement >::const_iterator itFront();
	const std::vector< LensElement >::const_iterator itEnd();
	std::vector< LensElement > lensElements;
private:
	
};

