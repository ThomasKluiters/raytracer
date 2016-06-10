#pragma once

#include <vector>
#include <algorithm>
#include <mutex>

using namespace std;

typedef unsigned char byte;

const float traveralCost = 100;
const float triangleTestCost = 14;

const byte LEFT = 0;
const byte RIGHT = 1;

const byte END = 0;
const byte PLANAR = 1;
const byte START = 2;

const byte X_AXIS = 0;
const byte Y_AXIS = 1;
const byte Z_AXIS = 2;

/**
*	Represents a plane in the given position and in the given dimension .
*/
struct Plane 
{

	float position;
	byte dimension;
	
};

/**
*	A rectangular prism in space representing a region of the scene.
*/
struct Voxel
{

	float min[3];
	float max[3];

};

/**
 *	A Triangle with 3 arrays representing vertices. 
 */	
struct Triangle 
{

	float* v1;
	float* v2;
	float* v3;

};

/*
*	Represents a split in a 3 dimensional plane.
*/
struct Split
{
	float cost;
	Plane plane;
	byte side;
};

struct Event
{

	// The poisition of this event
	float position;

	// The dimension of this event
	byte dimension;

	// The type of this event
	byte type;

	// A reference to the triangle this Event belongs to
	Triangle* triangle;

};

/**
*	Compare the two events based on the relation a <_E B described in Wald et al.
*/
inline bool compareEvents(const Event &a, const Event &b)
{
	return 
		(a.position < b.position) || 
		(a.position == b.position && a.dimension <  b.dimension) ||
		(a.position == b.position && a.dimension == b.dimension && a.type < b.type);
}

inline float min3(float a, float b, float c)
{
	if (a <= b && a <= c)	return a;
	if (b <= a && b <= c)	return b;
	return c;
}

inline float max3(float a, float b, float c)
{
	if (a >= b && a >= c)	return a;
	if (b >= a && b >= c)	return b;
	return c;
}

/**
*	Generates an event based on the triangle's vertices. The Event that is generated will be added to the given Vector 
*	of events.
*
*	An event is considered to be PLANAR when the triangle lies on the given dimensionsional plane. Else, two seperate
*	events, START and END, will be generated.
*/
inline void generateEvent(Triangle triangle, vector<Event> &events, int N[3])
{
	for (byte dimension = 0; dimension < 3; dimension++)
	{
		float smallest = min3(
			triangle.v1[dimension],
			triangle.v2[dimension],
			triangle.v3[dimension]
		);

		float largest = max3(
			triangle.v1[dimension],
			triangle.v2[dimension],
			triangle.v3[dimension]
		);
		
		// The event is PLANAR.
		if (smallest == largest)
		{
			events.push_back({ smallest, dimension, PLANAR, &triangle });

			N[dimension]++;
		}

		// The triangle has two distinct SRART and END events.
		else
		{
			events.push_back({ smallest, dimension,	START,	&triangle });
			events.push_back({ largest,	 dimension,	END,	&triangle });

			N[dimension] += 2;
		}
	}
}

/**
*	Computes Lambda(T_l, T_r) according Wald et al equation (7).
*/
inline float lambda(int T_l, int T_r)
{
	if (T_l == 0 || T_r == 0) return 0.8;
	return 1;
}

/**
*	The cost heuristic according to Wald et al equation (5).
*/
float C(float P_l, float P_r, int N_l, int N_r)
{
	return lambda(N_l, N_r) * (traveralCost + triangleTestCost * (P_l * N_l	 + P_r * N_r));
}


/**
*	Splits the given Voxel on the given Plane, splitting it into two new Voxels.
*/
pair<Voxel, Voxel> splitBox(Voxel voxel, Plane plane)
{
	Voxel V_l = voxel;
	V_l.max[plane.dimension] = plane.position;

	Voxel V_r = voxel;
	V_r.min[plane.dimension] = plane.position;

	return make_pair(V_l, V_r);
}

/**
*	Computes the surface area of the given Voxel.
*/
float SA(Voxel voxel)
{
	float lengths[3] = {
		voxel.max[0] - voxel.min[0],
		voxel.max[1] - voxel.min[1],
		voxel.max[2] - voxel.min[2]
	};

	return 2 * (lengths[0] * lengths[1] + lengths[1] * lengths[2] + lengths[2] * lengths[0]);
}

/**
*	Final cost heuristic for given configuration according to Wald et al Algorithm 2.
*/
pair<float, byte> SAH(Voxel V, Plane p, int N_l, int N_r, int N_p)
{
	pair<Voxel, Voxel> split = splitBox(V, p);
	Voxel V_l = split.first;
	Voxel V_r = split.second;

	float P_l = SA(V_l) / SA(V);
	float P_r = SA(V_r) / SA(V);

	float C_l = C(P_l, P_r, N_l + N_p, N_r);
	float C_r = C(P_l, P_r, N_l, N_r + N_p);

	if (C_l < C_r) return make_pair(C_l, LEFT);
	return make_pair(C_r, RIGHT);
}

/**
*	Incrementally finds the best split candidate plane in O(n) time according to Wald et al Algorithm 5.
*/
Plane findPlane(int N[3], Voxel V, vector<Event> E)
{
	Split best;
	best.cost = FLT_MAX;

	int N_l[3] = { 0, 0, 0 };
	int N_p[3] = { 0, 0, 0 };
	int N_r[3] = { N[0], N[1], N[2] };

	for (auto E_i = E.begin(); E_i != E.end(); )
	{
		const Plane plane = { E_i->position, E_i->dimension };
		int p[3] = { 0, 0, 0 };

		for (byte type = END; type <= START; type++)
		{
			while (E_i != E.end()
				&& E_i->dimension == plane.dimension
				&& E_i->type == type
				&& E_i->position == plane.position)
			{
				E_i++;
				p[type]++;
			}
		}

		N_p[plane.dimension] = p[PLANAR];
		N_r[plane.dimension] -= p[PLANAR];
		N_r[plane.dimension] -= p[END];

		pair<float, byte> C = SAH(V, plane, N_l[plane.dimension], N_r[plane.dimension], N_p[plane.dimension]);
		float cost = C.first;
		byte side = C.second;

 		if (best.cost > cost && plane.dimension != 2) {
			best = { cost, plane, side };
		}

		N_l[plane.dimension] += p[START];
		N_l[plane.dimension] += p[PLANAR];
		N_p[plane.dimension] = 0;
	}

	return best.plane;
}

Voxel sceneVoxel(vector<Triangle> &triangles)
{
	Voxel voxel = { *triangles[0].v1, *triangles[0].v1};
	
	for (auto triangle = triangles.begin(); triangle != triangles.end(); ++triangle)
	{
		for (int i = 0; i < 3; i++)
		{
			float smallest = min3(triangle->v1[i], triangle->v2[i], triangle->v3[i]);
			float largest = max3(triangle->v1[i], triangle->v2[i], triangle->v3[i]);

			voxel.max[i] = max(largest, voxel.max[i]);
			voxel.min[i] = min(smallest, voxel.min[i]);
		}
	}

	return voxel;

}

/** 
*	Builds the KD tree.
*/
void build(vector<Triangle>& triangles)
{	
	vector<Event> events;

	Voxel scene = sceneVoxel(triangles);

	int N[3] = { 0, 0, 0 };

	for (int i = 0; i < triangles.size(); i++)
	{
		generateEvent(triangles[i], events, N);
	}

	std::sort(events.begin(), events.end(), compareEvents);

	Plane plane = findPlane(N, scene, events);
}