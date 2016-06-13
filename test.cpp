#include "KDTree.h"
#include "mesh.h"

#include <chrono>
#include <random>

int main() {
	Mesh mesh;
	mesh.loadMesh("David.obj", true);

	Intersection I;
	I.distance = FLT_MAX;


	Vec3Df dir = Vec3Df(0, 0.1, -1);
	Vec3Df ori = Vec3Df(0, 0, 1);

	KDTree* tree = build(mesh);

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();


	for (int i = 0; i < 10000; i++)
	{
		tree->trace(ori, dir, I);
	}

	end = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsed_seconds = end - start;

	printf("Done in %fs\n", elapsed_seconds);

	getchar();
}