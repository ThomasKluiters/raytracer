#pragma once

#include <vector>
#include <algorithm>
#include <mutex>

using namespace std;

typedef int byte;

const byte END = 0;
const byte PLANAR = 1;
const byte START = 2;

/**
 *	A Triangle with 3 arrays representing vertices. 
 */	
struct Triangle 
{

	float* v1;
	float* v2;
	float* v3;

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
inline void generateEvent(Triangle triangle, vector<Event> &events)
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
		}

		// The triangle has two distinct SRART and END events.
		else
		{
			events.push_back({ smallest, dimension,	START,	&triangle });
			events.push_back({ largest,	 dimension,	END,	&triangle });
		}
	}
}

/** 
*	Builds the KD tree.
*/
void build(vector<Triangle>& triangles)
{	
	vector<Event> events;

	for (int i = 0; i < triangles.size(); i++)
	{
		generateEvent(triangles[i], events);
	}

	std::sort(events.begin(), events.end(), compareEvents);

	printf("%d\n", events.size());
}