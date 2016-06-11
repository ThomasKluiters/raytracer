#include "KDTree.h"

int main() {
	vector<Triangle> triangles;
	
	// Triangle 1
	float t1v1[] = { -7, 0, 0 };
	float t1v2[] = { -5, -1, 0 };
	float t1v3[] = { -5, -4, 0 };
	Triangle t1{ t1v1, t1v2, t1v3 };

	// Triangle 2
	float t2v1[] = { -3, 5, 0 };
	float t2v2[] = { -2, 2, 0 };
	float t2v3[] = { -4, 1, 0 };
	Triangle t2{ t2v1, t2v2, t2v3 };

	// Triangle 3
	float t3v1[] = { -3, 6, 0 };
	float t3v2[] = { -2, 7, 0 };
	float t3v3[] = { -2, 6.5, 0 };
	Triangle t3{ t3v1, t3v2, t3v3 };

	// Triangle 4
	float t4v1[] = { -1, 6, 0 };
	float t4v2[] = { 8, 6, 0 };
	float t4v3[] = { 6, 2, 0 };
	Triangle t4{ t4v1, t4v2, t4v3 };

	// Triangle 5
	float t5v1[] = { -1, -2, 0 };
	float t5v2[] = { 4, -2, 0 };
	float t5v3[] = { 6, -3, 0 };
	Triangle t5{ t5v1, t5v2, t5v3 };

	triangles.push_back(t1);
	triangles.push_back(t2);
	triangles.push_back(t3);
	triangles.push_back(t4);
	triangles.push_back(t5);

	int k;
	
	build(triangles);	
}