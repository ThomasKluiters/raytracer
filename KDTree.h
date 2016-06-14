#pragma once

#include "Vec3D.h"
#include "mesh.h";
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <mutex>

#define min3(x, y, z) min(x, min(y, z))
#define max3(x, y, z) max(x, max(y, z))

using namespace std;

typedef unsigned char byte;

const float traveralCost = 40;
const float triangleTestCost = 14;

const byte LEFT = 0;
const byte RIGHT = 1;
const byte BOTH = 2;

const byte MIDDLE = 2;

const byte END = 0;
const byte PLANAR = 1;
const byte START = 2;

const byte X_AXIS = 0;
const byte Y_AXIS = 1;
const byte Z_AXIS = 2;


/**
*	A rectangular prism in space representing a region of the scene.
*/
struct Voxel
{
	Vec3Df v_min;
	Vec3Df v_max;

	inline bool intersects(const Vec3Df & origin, const Vec3Df & dir)
	{
		float tmin =-FLT_MAX;
		float tmax = FLT_MAX;

		for (byte dimension = X_AXIS; dimension <= Z_AXIS; dimension++)
		{
			float t1 = (v_min[dimension] - origin[dimension]) / dir[dimension];
			float t2 = (v_max[dimension] - origin[dimension]) / dir[dimension];

			tmin = max(tmin, min(t1, t2));
			tmax = min(tmax, max(t1, t2));
		}

		return tmax > max(tmin, 0.0f);
	}

};

struct Intersection
{
	float distance;

	Vec3Df origin;

	Vec3Df direction;

	Vec3Df position;

	Vec3Df normal;

	int triangle;

	bool hit()
	{
		return distance < FLT_MAX;
	}

};


struct KDTree
{
	Voxel voxel;

	KDTree* left;
	KDTree* right;

	vector<int> indices;

	bool isLeaf()
	{
		return left == NULL && right == NULL;
	}

	Intersection trace(const Vec3Df & origin, const Vec3Df & direction, vector<Triangle> & triangles, vector<Vertex> & vertices)
	{
		Intersection intersection;
		intersection.origin = origin;
		intersection.direction = direction;
		intersection.distance = FLT_MAX;

		trace(origin, direction, intersection, triangles, vertices);

		

		if (intersection.hit())
		{
			const int triangle = intersection.triangle;


			Vec3Df N(0, 0, 0);
			for (int i = 0; i < 3; i++)
				N += vertices[triangles[triangle].v[i]].n;

			N.normalize();

			intersection.normal = N;
		}

		return intersection;
	}

	void trace(const Vec3Df & origin, const Vec3Df & direction, Intersection & intersection, vector<Triangle> & triangles, vector<Vertex> & vertices)
	{
		if (isLeaf())
		{
			for (auto index : indices)
			{
				Triangle & triangle = triangles[index];

				Vec3Df v0 = vertices[triangle.v[0]].p;
				Vec3Df v1 = vertices[triangle.v[1]].p;
				Vec3Df v2 = vertices[triangle.v[2]].p;

				Vec3Df e1 = v1 - v0;
				Vec3Df e2 = v2 - v0;

				Vec3Df P = Vec3Df::crossProduct(direction, e2);

				register float determinant = Vec3Df::dotProduct(P, e1);

				if (determinant > -FLT_EPSILON && determinant < FLT_EPSILON)
					continue;

				float inverse = 1.0f / determinant;

				Vec3Df T = (origin - v0);

				float u = Vec3Df::dotProduct(T, P) * inverse;

				if (u < 0 || u > 1.0f)
					continue;

				Vec3Df Q = Vec3Df::crossProduct(T, e1);

				float v = Vec3Df::dotProduct(direction, Q) * inverse;

				if (v < 0 || u + v > 1.0f)
					continue;

				float t = Vec3Df::dotProduct(e2, Q) * inverse;

				if (t > FLT_EPSILON && t < intersection.distance)
				{
					intersection.distance = t;
					intersection.position = origin + t * direction;
					intersection.triangle = index;
				}
			}
		}
		else
		{
			if (left != NULL && left->voxel.intersects(origin, direction))
			{
				left->trace(origin, direction, intersection, triangles, vertices);
			}
			if (right != NULL && right->voxel.intersects(origin, direction))
			{
				right->trace(origin, direction, intersection, triangles, vertices);
			}
		}
	}

};


class KDTreeBuilder
{


	Mesh mesh;

	vector<Vertex> & vertices;
	vector<Triangle> & triangles;

public:
	KDTreeBuilder(Mesh & someMesh) : mesh(someMesh), vertices(someMesh.vertices), triangles(someMesh.triangles) {}

	/**
	*	Represents a plane in the given position and in the given dimension .
	*/
	struct Plane
	{
		float position;
		byte dimension;
	};

	/*
	*	Represents a split in a 3 dimensional plane.
	*/
	struct Split
	{
		float cost;

		Plane plane;

		byte side;

		int N_l;
		int N_r;
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
		int triangle;
	};


	/**
	*	Represents a Classification between a list of events and a list of triangles.
	*/
	struct Classification
	{

		Classification(vector<int> T_l, vector<int> T_r, vector<Event> E_l, vector<Event> E_r)
		{
			this->T_l = T_l;
			this->T_r = T_r;
			this->E_l = E_l;
			this->E_r = E_r;
		}

		vector<int> T_l;
		vector<int> T_r;

		vector<Event> E_l;
		vector<Event> E_r;
	};


	/**
	*	Generates an event based on the triangle's vertices. The Event that is generated will be added to the given Vector
	*	of events.
	*
	*	An event is considered to be PLANAR when the triangle lies on the given dimensionsional plane. Else, two seperate
	*	events, START and END, will be generated.
	*/
	inline void generateEvents(int index, vector<Event> &events)
	{
		Triangle & triangle = triangles[index];
		for (byte dimension = 0; dimension < 3; dimension++)
		{

			float smallest = min3(
				vertices[triangle.v[0]].p[dimension],
				vertices[triangle.v[1]].p[dimension],
				vertices[triangle.v[2]].p[dimension]
			);

			float largest = max3(
				vertices[triangle.v[0]].p[dimension],
				vertices[triangle.v[1]].p[dimension],
				vertices[triangle.v[2]].p[dimension]
			);

			// The event is PLANAR.
			if (smallest == largest)
			{
				events.push_back({ smallest, dimension, PLANAR, index });
			}

			// The triangle has two distinct SRART and END events.
			else
			{
				events.push_back({ smallest, dimension,	START,	index });
				events.push_back({ largest,	 dimension,	END,	index });
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
		return (traveralCost + triangleTestCost * (P_l * N_l + P_r * N_r));
	}


	/**
	*	Splits the given Voxel on the given Plane, splitting it into two new Voxels.
	*/
	pair<Voxel, Voxel> splitBox(Voxel voxel, Plane plane)
	{
		Voxel V_l = voxel;
		V_l.v_max[plane.dimension] = plane.position;

		Voxel V_r = voxel;
		V_r.v_min[plane.dimension] = plane.position;
		
		return make_pair(V_l, V_r);
	}

	/**
	*	Computes the surface area of the given Voxel.
	*/
	float SA(Voxel voxel)
	{
		float lengths[3] = {
			abs(voxel.v_max[0] - voxel.v_min[0]),
			abs(voxel.v_max[1] - voxel.v_min[1]),
			abs(voxel.v_max[2] - voxel.v_min[2])
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
	Split findPlane(int N, Voxel V, vector<Event> E)
	{
		Split best;
		best.cost = FLT_MAX;

		int N_l[3] = { 0, 0, 0 };
		int N_p[3] = { 0, 0, 0 };
		int N_r[3] = { N, N, N };

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
					++E_i;
					p[type]++;
				}
			}

			N_p[plane.dimension] = p[PLANAR];
			N_r[plane.dimension] -= p[PLANAR];
			N_r[plane.dimension] -= p[END];

			pair<float, byte> C = SAH(V, plane, N_l[plane.dimension], N_r[plane.dimension], N_p[plane.dimension]);
			float cost = C.first;
			byte side = C.second;

			if (best.cost >= cost) {
				best = { cost, plane, side, N_l[dimension], N_r[dimension] };
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
			voxel.v_max[i] = -FLT_MAX;
			voxel.v_min[i] = FLT_MAX;
		}


		for (auto triangle = triangles.begin(); triangle != triangles.end(); ++triangle)
		{
			for (int dimension = 0; dimension < 3; dimension++)
			{
				float smallest = min3(
					vertices[triangle->v[0]].p[dimension],
					vertices[triangle->v[1]].p[dimension],
					vertices[triangle->v[2]].p[dimension]
				);

				float largest = max3(
					vertices[triangle->v[0]].p[dimension],
					vertices[triangle->v[1]].p[dimension],
					vertices[triangle->v[2]].p[dimension]
				);

				voxel.v_max[dimension] = max(largest, voxel.v_max[dimension]);
				voxel.v_min[dimension] = min(smallest, voxel.v_min[dimension]);
			}
		}

		return voxel;

	}

	inline void clamp(Vec3Df & p_min, Vec3Df & p_max, Vec3Df & candidate)
	{
		for (byte dimension = X_AXIS; dimension <= Z_AXIS; dimension++)
		{
			p_min[dimension] = min(candidate[dimension], p_min[dimension]);
			p_max[dimension] = max(candidate[dimension], p_max[dimension]);
		}
	}

	inline void clamp(Vec3Df & min_l, Vec3Df & max_l, Vec3Df & min_r, Vec3Df & max_r, Vec3Df & candidate, byte dimension, float position)
	{
		float v = candidate[dimension];
		if (v < position)
		{
			clamp(min_l, max_l, candidate);
		}
		else
		{
			clamp(min_r, max_r, candidate);
		}
	}

	inline Vec3Df intersect(Vec3Df & v1, Vec3Df & v2, byte dimension, float position)
	{
		float dot = (v2[dimension] - v1[dimension]);

		float fac = -(v1[dimension] - position) / (dot);

		return Vec3Df(
			fac * (v2[X_AXIS] - v1[X_AXIS]) + v1[X_AXIS],
			fac * (v2[Y_AXIS] - v1[Y_AXIS]) + v1[Y_AXIS],
			fac * (v2[Z_AXIS] - v1[Z_AXIS]) + v1[Z_AXIS]
		);
	}


	inline void populate(Vec3Df & min_l, Vec3Df & max_l, Vec3Df & min_r, Vec3Df & max_r, Vec3Df & intersection)
	{
		clamp(min_l, max_l, intersection);
		clamp(min_r, max_r, intersection);
	}

	inline void check(Vec3Df & min_l, Vec3Df & max_l, Vec3Df & min_r, Vec3Df & max_r, float position, byte dimension, Vec3Df & v1, Vec3Df & v2)
	{
		if ((v1[dimension] >= position && v2[dimension] <= position) || (v1[dimension] < position && v2[dimension] > position))
		{
			Vec3Df intersection = intersect(v1, v2, dimension, position).p;
			populate(min_l, max_l, min_r, max_r, intersection);
		}
	}
	void clip(vector<Event> & E_bl, vector<Event> & E_br, int index, Plane plane)
	{
		Triangle & triangle = triangles[index];

		float position = plane.position;
		byte dimension = plane.dimension;

		Vec3Df & v1 = vertices[triangle.v[0]].p;
		Vec3Df & v2 = vertices[triangle.v[1]].p;
		Vec3Df & v3 = vertices[triangle.v[2]].p;

		Vec3Df min_l(FLT_MAX, FLT_MAX, FLT_MAX);
		Vec3Df max_l(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		Vec3Df min_r(FLT_MAX, FLT_MAX, FLT_MAX);
		Vec3Df max_r(-FLT_MAX, -FLT_MAX, -FLT_MAX);


		check(min_l, max_l, min_r, max_r, position, dimension, v1, v2);
		clamp(min_l, max_l, min_r, max_r, v1, dimension, position);

		check(min_l, max_l, min_r, max_r, position, dimension, v2, v3);
		clamp(min_l, max_l, min_r, max_r, v2, dimension, position);

		check(min_l, max_l, min_r, max_r, position, dimension, v3, v1);
		clamp(min_l, max_l, min_r, max_r, v3, dimension, position);

		for (byte dimension = X_AXIS; dimension <= Z_AXIS; dimension++)
		{
			E_bl.push_back({ min_l[dimension], dimension, START, index });
			E_bl.push_back({ max_l[dimension], dimension, END, index });

			E_br.push_back({ min_r[dimension], dimension, START, index });
			E_br.push_back({ max_r[dimension], dimension, END, index });
		}
	}

	int n = 0;

	/**
	*	Classifies the given Triangles into a left and right side of the splitting Plane.
	*/
	Classification classify(vector<int> & T, vector<Event> & E, Split p)
	{
		byte planeDimension = p.plane.dimension;
		float planePosition = p.plane.position;

		vector<int> T_l;
		vector<int> T_r;

		vector<Event> E_lo;
		vector<Event> E_ro;

		vector<Event> E_bl;
		vector<Event> E_br;

		vector<Event> E_l;
		vector<Event> E_r;


		unordered_map<int, byte> classifiedTriangles;
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
			switch (triangle.second)
			{
			case LEFT:
				T_l.push_back(triangle.first);
				break;

			case RIGHT:
				T_r.push_back(triangle.first);
				break;

			case BOTH:
				T_l.push_back(triangle.first);
				T_r.push_back(triangle.first);

				clip(E_bl, E_br, triangle.first, p.plane);
				break;
			}
		}

		std::sort(E_bl.begin(), E_bl.end());
		std::sort(E_br.begin(), E_br.end());

		for (auto event : E)
		{
			switch (classifiedTriangles[event.triangle])
			{

			case LEFT:
				E_lo.push_back(event);
				break;

			case RIGHT:
				E_ro.push_back(event);
				break;

			case BOTH:
				// events are discarded
				break;

			default:
				printf("O shit waddup\n");
				break;
			}
		}

		E_l.resize(E_lo.size() + E_bl.size());
		std::merge(E_lo.begin(), E_lo.end(), E_bl.begin(), E_bl.end(), E_l.begin());

		E_r.resize(E_ro.size() + E_br.size());
		std::merge(E_ro.begin(), E_ro.end(), E_br.begin(), E_br.end(), E_r.begin());

		return Classification(T_l, T_r, E_l, E_r);
	}

	/**
	*	Implements the Termiante function according to Wald et al. function (6).
	*/
	bool terminate(vector<int> T, float C_v)
	{
		return (T.size() * triangleTestCost * 0.8) <= C_v;
	}

	/**
	*	Builds the tree according according to Wald et al. algorithm 1.
	*/
	KDTree* construct(vector<int> T, vector<Event> & E, Voxel & V)
	{
		Split split = findPlane(T.size(), V, E);

		if (terminate(T, split.cost))
		{
			return new KDTree{ V, NULL, NULL, T };
		}

		Classification classification = classify(T, E, split);

		pair<Voxel, Voxel> children = splitBox(V, split.plane);

		// no termination -> split into two lists of triangles and buildTree on those
		return new KDTree{
			V,
			classification.T_l.size() == 0 ? NULL : construct(classification.T_l, classification.E_l, children.first),
			classification.T_r.size() == 0 ? NULL : construct(classification.T_r, classification.E_r, children.second),
			vector<int>()
		};
	}



	/**
	*	Builds the KD tree.
	*/
	KDTree* build()
	{
		printf("#### KD-tree generation - File\n");
		printf("Loading file...\n");

		vector<int> initial;


		printf("Done...\n");
		printf("#### KD-tree generation - Events\n");

		vector<Event> events;

		Voxel scene = sceneVoxel(triangles);

		for (int i = 0; i < triangles.size(); i++)
		{
			generateEvents(i, events);
			initial.push_back(i);
		}

		std::sort(events.begin(), events.end());

		printf("Done...\n");

		printf("Events generated.\n");

		printf("#### KD-tree generation - Construction\n");

		KDTree* tree = construct(initial, events, scene);

		printf("Done...\n");

		return tree;
	}

};

/**
*	Compare the two events based on the relation a <_E B described in Wald et al.
*/
bool operator <(const KDTreeBuilder::Event& a, const KDTreeBuilder::Event& b) {
	return
		(a.position < b.position) ||
		(a.position == b.position && a.dimension < b.dimension) ||
		(a.position == b.position && a.dimension == b.dimension && a.type < b.type);
}