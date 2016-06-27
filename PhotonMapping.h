#pragma once

#include "Vec3D.h"
#include <unordered_map>

struct Photon {
	
	Vec3Df position;
	Vec3Df power;
	
	float phi, theta;

	short flag;

};

struct Box
{
	Vec3Df max;
	Vec3Df min;
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

};

class PhotonMapBuilder
{
public:
	std::vector<Photon> photons;

	const int pointTestCost = 4;
	const int sphereTestCost = 150;

	void add(Photon & photon)
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
		V_l.max[dimension] = position;

		Box V_r = box;
		V_r.min[dimension] = position;

		return make_pair(V_l, V_r);
	}

	float SA(Box box)
	{
		float lengths[3] = {
			abs(box.max[0] - box.min[0]),
			abs(box.max[1] - box.min[1]),
			abs(box.max[2] - box.min[2])
		};

		return 2 * (lengths[0] * lengths[1] + lengths[1] * lengths[2] + lengths[2] * lengths[0]);
	}

	float SAH(Box box, float position, int dimension, int N_l, int N_r)
	{
		pair<Box, Box> split = splitBox(box, position, dimension);
		Box V_l = split.first;
		Box V_r = split.second;

		float P_l = SA(V_l) / SA(box);
		float P_r = SA(V_r) / SA(box);

		float C_l = C(P_l, P_r, N_l + 1, N_r);
		float C_r = C(P_l, P_r, N_l, N_r + 1);

		return fminf(C_l, C_r);
	}


	Split findPlane(vector<Event> & events, Box box)
	{

		const int size = events.size();

		int left[3] = { 0, 0, 0 };
		
		int right[3] = { size, size, size };

		float best = FLT_MAX;

		Split split;

		for (int eventIndex = 0; eventIndex < size; eventIndex++)
		{
			Event & event = events[eventIndex];

			const float position = event.position;
			const int dimension = event.dimension;

			float cost = SAH(box, position, dimension, left[dimension], right[dimension]);

			if (cost < best)
			{
				cost = best;

				split = {
					position,
					dimension,
					cost,
					left[dimension],
					right[dimension]
				};
			}

			left[dimension]--;
		}

		return split;
	}

#define L 0
#define R 1

	PhotonMap * construct(vector<Event> & events, Box box)
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

				if (eventPosition < position)
				{
					classifications[events[eventIndex].photon] = L;
				}
				else
				{
					classifications[events[eventIndex].photon] = R;
				}
			}
		}

		vector<Event> left;
		vector<Event> right;

		for (int eventIndex = 0; eventIndex < events.size(); eventIndex++)
		{
			const int photon = events[eventIndex].photon;
			const int side = classifications[photon];

			if (side == L)
			{
				left.push_back(events[eventIndex]);
			}
			else if (side == R)
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

bool operator <(const PhotonMapBuilder::Event& a, const PhotonMapBuilder::Event& b) {
	return
		(a.position < b.position) ||
		(a.position == b.position && a.dimension < b.dimension) ||
		(a.position == b.position && a.dimension == b.dimension);
}