#pragma once

#include "Vec3D.h"
#include "KDTree.h"
#include <unordered_map>
#include <queue>
#include <functional>

const int N = 50;

#define SHADOW 0
#define DIFFUSE 1
#define CAUSTIC 2 

struct Photon {
	
	Vec3Df position;

	Vec3Df power;
	
	Vec3Df incident;

	int type;

};

struct Radiance {

	Photon photon;

	float distance;

};

struct Box
{
	Vec3Df v_max;
	Vec3Df v_min;

	bool contains(Vec3Df & point)
	{
		for (int i = 0; i < 3; i++)
		{
			if (v_max[i] < point[i] || v_min[i] > point[i])
				return false;
		}
		return true;
	}

	float distance(Vec3Df & point)
	{
		Vec3Df center = (v_max + v_min) * 0.5f;
		Vec3Df dir = point - center;

		float tmin = -FLT_MAX;
		float tmax = FLT_MAX;

		for (byte dimension = X_AXIS; dimension <= Z_AXIS; dimension++)
		{
			float t1 = (v_min[dimension] - point[dimension]) / dir[dimension];
			float t2 = (v_max[dimension] - point[dimension]) / dir[dimension];

			tmin = max(tmin, min(t1, t2));
			tmax = min(tmax, max(t1, t2));
		}

		return tmin;
	}
};


class PhotonMap
{
public:
	vector<Photon> photons;
	Box box;

	PhotonMap * left;
	PhotonMap * right;

	PhotonMap(Box box) : box(box) {}

	PhotonMap(vector<Photon> photons, Box box) : box(box), photons(photons) {}

	bool leaf()
	{
		return left == NULL && right == NULL;
	}

	void locatePhotons(Vec3Df position, vector<Radiance> & heap)
	{
		if (leaf())
		{
			for (auto photon : photons)
			{
				float distance = Vec3Df::distance(photon.position, position);
				float radius = distance * distance;

				if (heap.size() < N)
				{
					heap.push_back({
						photon,
						radius
					});
					push_heap(heap.begin(), heap.end());
				}
				else
				{
					Radiance & top = heap.front();
					if (top.distance > radius)
					{
						pop_heap(heap.begin(), heap.end());
						heap.pop_back();

						heap.push_back({
							photon,
							radius
						});

						push_heap(heap.begin(), heap.end());
					}
				}
			}
		}
		else
		{

			float leftDistance = left->box.distance(position);
			float rightDistance = right->box.distance(position);

			if (rightDistance > leftDistance)
			{
				left->locatePhotons(position, heap);

				if (heap.size() > 0)
				{
					float max = heap.front().distance;

					if (rightDistance > max)
					{
						return;
					}
				}

				right->locatePhotons(position, heap);

			}
			else
			{
				right->locatePhotons(position, heap);

				if (heap.size() > 0)
				{
					float max = heap.front().distance;

					if (leftDistance > max)
					{
						return;
					}
				}

				left->locatePhotons(position, heap);
			}

		}
	}

};

class PhotonMapBuilder
{
public:
	std::vector<Photon> photons;

	const int pointTestCost = 4;
	const int sphereTestCost = 150;

	void add(Photon photon)
	{
		photons.push_back(photon);
	}

	struct Split
	{
		float position;

		int dimension;

		float cost;

		int left;

		int right;

		int side;

		int photon;
	};

	struct Event
	{
		float position;

		int dimension;

		int photon;
	};

	void generateEvent(int photonIndex, Vec3Df & position, vector<Event> & events)
	{
		Photon & photon = photons[photonIndex];

		for (int i = 0; i < 3; i++)
		{
			events.push_back({
				position[i],
				i,
				photonIndex
			});
		}

	}

	inline float lambda(int T_l, int T_r)
	{
		if (T_l == 0 || T_r == 0) return 0.8;
		return 1;
	}

	float C(float P_l, float P_r, int N_l, int N_r)
	{
		return lambda(N_l, N_r) * (sphereTestCost + pointTestCost * (P_l * N_l + P_r * N_r));
	}

	pair<Box, Box> splitBox(Box box, float position, int dimension)
	{
		Box V_l = box;
		V_l.v_max[dimension] = position;

		Box V_r = box;
		V_r.v_min[dimension] = position;

		return make_pair(V_l, V_r);
	}

	float SA(Box box)
	{
		float lengths[3] = {
			abs(box.v_max[0] - box.v_min[0]),
			abs(box.v_max[1] - box.v_min[1]),
			abs(box.v_max[2] - box.v_min[2])
		};

		return 2 * (lengths[0] * lengths[1] + lengths[1] * lengths[2] + lengths[2] * lengths[0]);
	}

	pair<float, int> SAH(Box box, float position, int dimension, int N_l, int N_r)
	{
		pair<Box, Box> split = splitBox(box, position, dimension);
		Box V_l = split.first;
		Box V_r = split.second;

		float P_l = SA(V_l) / SA(box);
		float P_r = SA(V_r) / SA(box);

		float C_l = C(P_l, P_r, N_l + 1, N_r);
		float C_r = C(P_l, P_r, N_l, N_r + 1);

		if (C_l < C_r)
			return make_pair(C_l, LEFT);
		return make_pair(C_r, RIGHT);
	}


	Split findPlane(vector<Event> & events, Box box)
	{

		const int size = events.size();

		int left[3] = { 0, 0, 0 };
		
		int right[3] = { size / 3, size / 3, size / 3};

		float best = FLT_MAX;

		Split split;

		for (int eventIndex = 0; eventIndex < size; eventIndex++)
		{
			Event & event = events[eventIndex];

			const float position = event.position;
			const int dimension = event.dimension;
			const int photon = event.photon;

			pair<float, int> sah = SAH(box, position, dimension, left[dimension], right[dimension]);
			float cost = sah.first;
			int side = sah.second;

			if (cost <= best)
			{
				best = cost;

				split = {
					position,
					dimension,
					cost,
					left[dimension],
					right[dimension],
					side,
					photon
				};
			}

			left[dimension]++;
			right[dimension]--;
		}

		return split;
	}
	
	PhotonMap * construct(vector<Event> events, Box box)
	{
		Split split = findPlane(events, box);

		if (split.cost > 0.8f * events.size() / 3 * pointTestCost)
		{
			vector<Photon> selectedPhotons;

			for (int i = 0; i < events.size(); i++)
			{
				if (events[i].dimension == 0)
				{
					selectedPhotons.push_back(photons[events[i].photon]);
				}
			}

			return new PhotonMap(selectedPhotons, box);
		}

		const float position = split.position;
		const int dimension = split.dimension;

		pair<Box, Box> boxes = splitBox(box, position, dimension);
		Box leftBox = boxes.first;
		Box rightBox = boxes.second;

		unordered_map<int, int> classifications;

		for (int eventIndex = 0; eventIndex < events.size(); eventIndex++)
		{
			if (events[eventIndex].dimension == dimension)
			{
				const float eventPosition = events[eventIndex].position;

				if (events[eventIndex].photon == split.photon)
				{
					classifications[split.photon] = split.side;
				} 
				else if (eventPosition < position)
				{
					classifications[events[eventIndex].photon] = LEFT;
				}
				else
				{
					classifications[events[eventIndex].photon] = RIGHT;
				}
			}
		}

		vector<Event> left;
		vector<Event> right;

		for (int eventIndex = 0; eventIndex < events.size(); eventIndex++)
		{
			const int photon = events[eventIndex].photon;
			const int side = classifications[photon];

			if (side == LEFT)
			{
				left.push_back(events[eventIndex]);
			}
			else if (side == RIGHT)
			{
				right.push_back(events[eventIndex]);
			}
		}

		PhotonMap* leftMap = construct(left, leftBox);
		PhotonMap* rightMap = construct(right, rightBox);

		PhotonMap* root = new PhotonMap(box);
		root->left = leftMap;
		root->right = rightMap;

		return root;
	}

	PhotonMap* build()
	{
		vector<Event> events;
		events.reserve(photons.size() * 3);

		Vec3Df sceneMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		Vec3Df sceneMin(FLT_MAX, FLT_MAX, FLT_MAX);

		for (int i = 0; i < photons.size(); i++)
		{
			Photon & photon = photons[i];

			Vec3Df & position = photon.position;
			
			for (int i = 0; i < 3; i++)
			{
				sceneMin[i] = fminf(position[i], sceneMin[i]);
				sceneMax[i] = fmaxf(position[i], sceneMax[i]);
			}

			generateEvent(i, position, events);
		}

		Box scene{
			sceneMax,
			sceneMin
		};

		sort(events.begin(), events.end());

		return construct(events, scene);
	}

};

bool operator <(const Radiance& a, const Radiance& b) {
	return a.distance < b.distance;
}

bool operator <(const PhotonMapBuilder::Event& a, const PhotonMapBuilder::Event& b) {
	return
		(a.position < b.position) ||
		(a.position == b.position && a.dimension < b.dimension) ||
		(a.position == b.position && a.dimension == b.dimension);
}