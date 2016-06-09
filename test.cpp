#include "KDTree.h"

int main() {
	vector<Triangle> triangles;
	
	// Triangle 1
	float t1v1[] = { -7, 0, 0 };
	float t1v2[] = { -5, -1, 0 };
	float t1v3[] = { -5, -4, 0 };

	// Triangle 2
	float t2v1[] = { -3, 5, 0 };
	float t2v2[] = { -2, 2, 0 };
	float t2v3[] = { -4, 1, 0 };

	// Triangle 3
	float t3v1[] = { -3, 6, 0 };
	float t3v2[] = { -2, 7, 0 };
	float t3v3[] = { -2, 6, 0 };

	// Triangle 4
	float t4v1[] = { -1, 6, 0 };
	float t4v2[] = { 8, 6, 0 };
	float t4v3[] = { 6, 2, 0 };

	// Triangle 5
	float t5v1[] = { -1, -2, 0 };
	float t5v2[] = { 4, -2, 0 };
	float t5v3[] = { 6, -3, 0 };

	// Triangle 5
	float t5v1[] = { -1, -2, 0 };
	float t5v2[] = { 4, -2, 0 };
	float t5v3[] = { 6, -3, 0 };


	triangles.push_back(Triangle{ t1v1, t1v2, t1v3 });
	triangles.push_back(Triangle{ t2v1, t2v2, t2v3 });
	triangles.push_back(Triangle{ t3v1, t3v2, t3v3 });
	triangles.push_back(Triangle{ t4v1, t4v2, t4v3 });
	triangles.push_back(Triangle{ t5v1, t5v2, t5v3 });

	build(triangles);

}