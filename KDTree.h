#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <mutex>

using namespace std;

typedef unsigned char byte;

const float traveralCost = 10;
const float triangleTestCost = 14;

const byte LEFT = 0;
const byte RIGHT = 1;
const byte BOTH = 2;

const byte END = 0;
const byte PLANAR = 1;
const byte START = 2;

const byte X_AXIS = 0;
const byte Y_AXIS = 1;
const byte Z_AXIS = 2;

struct KDTree
{
	Voxel voxel;

	KDTree* left;
	KDTree* right;

	vector<Triangle*> triangles;

	bool isLeaf()
	{
		return left == NULL && right == NULL;
	}
};

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
	int N[3];
};

/**
*	Represents a Classification between a list of events and a list of triangles.
*/
struct Classification
{
	vector<Triangle*> T_l;
	vector<Triangle*> T_r;

	vector<Event> E_l;
	vector<Event> E_r;
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

/**
*	Returns the lowest of three floats.
*/
inline float min3(float a, float b, float c)
{
	if (a <= b && a <= c)	return a;
	if (b <= a && b <= c)	return b;
	return c;
}

/**
*	Returns the highest of three floats.
*/
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
inline void generateEvent(Triangle & triangle, vector<Event> &events, int N[3])
{
	for (byte dimension = 0; dimension < 3; dimension++)
	{
		Triangle* ptr = &triangle;

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
			events.push_back({ smallest, dimension, PLANAR, ptr });

			N[dimension]++;
		}

		// The triangle has two distinct SRART and END events.
		else
		{
			events.push_back({ smallest, dimension,	START,	ptr });
			events.push_back({ largest,	 dimension,	END,	ptr });

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
Split findPlane(int N[3], Voxel V, vector<Event> E)
{
	Split best;
	best.cost = FLT_MAX;

	int N_l[3] = { 0, 0, 0 };
	int N_p[3] = { 0, 0, 0 };
	int N_r[3] = { N[0], N[1], N[2] };

	for (auto E_i = E.begin(); E_i != E.end(); )
	{
		const Plane plane = { E_i->position, E_i->dimension };
		byte dimension = E_i->dimension;
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

 		if (best.cost > cost && dimension != 2) {
			best = { cost, plane, side };
		}

		N_l[plane.dimension] += p[START];
		N_l[plane.dimension] += p[PLANAR];
		N_p[plane.dimension] = 0;
	}

	return best;
}

Voxel sceneVoxel(vector<Triangle> &triangles)
{
	Voxel voxel;
	for (int i = 0; i < 3; i++) {
		voxel.max[i] = triangles[0].v1[i];
		voxel.min[i] = triangles[0].v1[i];
	}


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
*	Classifies the given Triangles into a left and right side of the splitting Plane.
*/
Classification classify(vector<Triangle*> & T, vector<Event> & E, Split p)
{
	byte planeDimension = p.plane.dimension;
	float planePosition = p.plane.position;

	vector<Triangle*> T_l;
	vector<Triangle*> T_r;

	vector<Event> E_l;
	vector<Event> E_r;

	unordered_map<Triangle*, byte> classifiedTriangles;
	classifiedTriangles.reserve(T.size());

	for (auto triangle = T.begin(); triangle != T.end(); ++triangle)
	{
		classifiedTriangles[(*triangle)] = BOTH;
	}

	for (auto event = E.begin(); event != E.end(); event++)
	{
		if (event->dimension == planeDimension)
		{
			switch (event->type)
			{
			case END:
				if (event->position <= planePosition)
				{
					classifiedTriangles[event->triangle] = LEFT;
				}
				break;

			case START:
				if (event->position >= planePosition)
				{
					classifiedTriangles[event->triangle] = RIGHT;
				}
				break;

			case PLANAR:
				if (event->position < planePosition || (event->position == planePosition && p.side == LEFT))
				{
					classifiedTriangles[event->triangle] = LEFT;
				}
				else
				{
					classifiedTriangles[event->triangle] = RIGHT;
				}
				break;

			default:
				printf("Dat boi\n"); // pls don't hurt us
			}
		}
	}

	for (auto triangle : classifiedTriangles)
	{
		if (triangle.second == LEFT || triangle.second == BOTH)
			T_l.push_back(triangle.first);

		if (triangle.second == RIGHT || triangle.second == BOTH)
			T_r.push_back(triangle.first);
	}

	for (auto event : E)
	{
		switch(classifiedTriangles[event.triangle])
		{

		case LEFT:
			E_l.push_back(event);
			break;

		case RIGHT:
			E_r.push_back(event);
			break;

		default:
			printf("O shit waddup\n");
			break;
		}
	}

	return { T_l, T_r, E_l, E_r };
}

/**
*	Implements the Termiante function according to Wald et al. function (6).
*/
bool terminate(vector<Triangle*> T, float C_v)
{
	return T.size() * triangleTestCost < C_v;
}

/**
*	Builds the tree according according to Wald et al. algorithm 1.
*/
KDTree construct(vector<Triangle*> & T, vector<Event> & E, Voxel V, int N[3])
{
	Split split = findPlane(N, V, E);

	if (terminate(T, split.cost))
	{
		return KDTree{ V, NULL, NULL, T };
	}

	Classification classification = classify(T, E, split);
	
	pair<Voxel, Voxel> children = splitBox(V, split.plane);
	Voxel V_l = children.first;
	Voxel V_r = children.second;

	// no termination -> split into two lists of triangles and buildTree on those
	return KDTree{
		V,
		&construct(classification.T_l, classification.E_l, V_l, N),
		&construct(classification.T_r, classification.E_r, V_r, N),
		vector<Triangle*>()
	};
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

	Split split = findPlane(N, scene, events);
}
