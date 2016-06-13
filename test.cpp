#include "KDTree.h"
#include <chrono>
#include <random>

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

	// Triangle 6
	float t6v1[] = { -1, -1, 0 };
	float t6v2[] = {  1, -1, 0 };
	float t6v3[] = {  0,  3, 0 };
	Triangle t6{ t6v1, t6v2, t6v3 };

	//triangles.push_back(t1);
	//triangles.push_back(t2);
	//triangles.push_back(t3);
	//triangles.push_back(t4);
	//triangles.push_back(t5);

	triangles.reserve(10000);

	std::default_random_engine generator;
	std::normal_distribution<float> distribution(0.0, 3.0);

	std::normal_distribution<float> small_distribution(0.0, 0.1);

	for (int i = 0; i < 10000; i++)
	{
		float* v1 = new float[3];
		float* v2 = new float[3];
		float* v3 = new float[3];
		
		float o = distribution(generator);

		v1[0] = small_distribution(generator) + o;
		v2[0] = small_distribution(generator) + o;
		v3[0] = small_distribution(generator) + o;

		for (int i = 1; i < 3; i++)
		{
			v1[i] = small_distribution(generator) + v1[i - 1];
			v2[i] = small_distribution(generator) + v2[i - 1];
			v3[i] = small_distribution(generator) + v3[i - 1];
		}

		triangles.push_back({ v1, v2, v3 });
	}

	printf("Done creating triangles.\n");

	auto start = chrono::high_resolution_clock::now();

	build(triangles);

	auto end =   chrono::high_resolution_clock::now();

	std::chrono::duration<double> diff = end - start;

	printf("Done in %fs\n", diff.count());
	getchar();
}