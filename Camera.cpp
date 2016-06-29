#pragma once
#include "Camera.h"
#include <GL/glut.h>

Camera::Camera(int width, int height, Vec3Df &cameraPos, Vec3Df up_vector,
				float fov, float aperture, float focaldistance)
{
	// General options
	camPos = cameraPos;
	lookAtPos = Vec3Df(0.0,0.0,0.0);
	field_of_view = fov;
	up = up_vector;
	lensRadius = aperture;
	focalDistance = focaldistance;
	setFilmPlane(width, height);
	reCalcAUS();
}

void Camera::reCalcAUS()
{
	Vec3Df atDirection = lookAtPos - camPos;
	atDirection.normalize();
	Vec3Df world_up = Vec3Df(0.0, 1.0, 0.0);
	if (Vec3Df::dotProduct(world_up, atDirection) == 0) {
		world_up = Vec3Df(0.0, 0.0, 1.0);
	}
	side = Vec3Df::crossProduct(world_up, atDirection);
	up = Vec3Df::crossProduct(atDirection, side);
	side.normalize();
	up.normalize();
}

void Camera::setFilmPlane(int w, int h)
{
	sensor.initImagePlane();
	sensor.resizeImagePlane(w, h);
}

void Camera::traceThick()
{
	thickLens = true;
	thinLens = false;
	fullLens = false;
}

void Camera::traceThin()
{
	thickLens = false;
	thinLens = true;
	fullLens = false;
}

void Camera::traceFull()
{
	thickLens = false;
	thinLens = false;
	fullLens = true;
}

bool Camera::apertureIntersect(const Vec3Df & origin, const Vec3Df & dest)
{
	Vec3Df lensPos = camPos + lookAtDir*focalDistance;
	Vec3Df f;

	if (!diskIntersect(origin, dest, lookAtDir, lensPos, lensRadius, f)) {
		return false;
	}
	else {
		return true;
	}
}

// Given a vector in world space; transform that vector to this camera's camera space.
Vec3Df Camera::transformPoint(const Vec3Df & given)
{
	float x, y, z;
	float xr, yr, zr, wp;

	float m[4][4];

	/* Reference for knowing what goes where
	matrix[0][3] = camPos[0];	// Set the last collumn of the transformation matrix
	matrix[1][3] = camPos[1];
	matrix[2][3] = camPos[2];
	matrix[3][3] = 1;

	reCalcAUS();			// Recalculate the At-Up and Side vectors
	matrix[0][0] = side[0];	// Fill in at-up-side matrix entries...
	matrix[1][0] = side[2];
	matrix[2][0] = side[3];
	matrix[3][0] = 0.0;

	matrix[0][1] = up[0];
	matrix[1][1] = up[1];
	matrix[2][1] = up[2];
	matrix[3][1] = 0.0;


	matrix[0][2] = lookAtDir[0];
	matrix[1][2] = lookAtDir[1];
	matrix[2][2] = lookAtDir[2];
	matrix[3][2] = 0.0;
	*/

	xr = side[0] * x + up[0] * y + lookAtDir[0] * z + camPos[0];
	yr = side[2] * x + up[1] * y + lookAtDir[1] * z + camPos[1];
	zr = side[3] * x + up[2] * y + lookAtDir[2] * z + camPos[2];
	wp = side[2] * x + up[1] * y + lookAtDir[1] * z + camPos[1];
}

void Camera::transformCamera(const Vec3Df newPosition)
{
	camPos = newPosition;
	reCalcAUS();
}

void Camera::lookAt(const Vec3Df newPosition)
{
	lookAtPos = newPosition;
	reCalcAUS();
}

// At should be normalized
void Camera::traceThisRay(Vec3Df at, Vec3Df opticalOrigin, Vec3Df & rayOrigin, Vec3Df & rayDest)
{
	int cursor = currentLens.lensElements.size() - 1;	// Set cursor on first lensElement from filmPlane
	Vec3Df pos_optical_axis = opticalOrigin;
	at.normalize();

	glPointSize(10.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(opticalOrigin[0], opticalOrigin[1], opticalOrigin[2]);
	glVertex3f(at[0]*100.0 + opticalOrigin[0], at[1] * 100.0 + opticalOrigin[1], at[2] * 100.0 + opticalOrigin[2]);
	glEnd();

	while (cursor >= 0) {
		LensElement currentEl = currentLens.lensElements[cursor];
		// Offset the center of the lens element by its thickness
		pos_optical_axis = pos_optical_axis - at * currentEl.thickness;
		Vec3Df intersectPos, normal, ray;
		ray = rayDest - rayOrigin;
		ray.normalize();

		//Determine intersection with either a spherical element or with the aperture stop.
		if (currentEl.radius == 0.0) {
			// We need to rescale the ray <origin, dest> so that dest falls on the aperture-disk
			// We can use linear scaling / similar triangles
			// Define the origin-point of the ray as our base, now;
			// -> Subtract pos_optical_axis(x-axis) from rayOrigin (x-axis) to get the offset.
			// -> Determine the same kind of offset between origin and dest.
			// -> These two form a scaling factor that we can use to rescale the dest-position on the aperture-plane.
			float scaleFactor = (rayOrigin - pos_optical_axis).getLength() / rayDest.getLength();		// ..... mooo
			intersectPos = rayOrigin - ray * scaleFactor;
			glPointSize(10.0);
			glColor3f(1.0,0.0,0.0);
			glBegin(GL_POINTS);
			glVertex3f(intersectPos[0], intersectPos[1], intersectPos[2]);
			glEnd();
		}
		else {
			// Otherwise; we interact with the spherical lens
			// Determine the intersection and surface-normal of our ray and the lens-element
			Vec3Df lensCenter = Vec3Df(pos_optical_axis[0] + currentEl.radius, pos_optical_axis[1], pos_optical_axis[2]);
			if (!intersectWithSphere(currentEl.radius, lensCenter, rayOrigin, rayDest, intersectPos, normal))
			{
				break;
			}
		}

		float sqLength = (intersectPos - pos_optical_axis).getLength();		// Get length from lenscenter to intersect
		float sqAperture = currentEl.aperture / 2.0;

		rayOrigin = intersectPos;	// Set ray origin on the new Intersection

									// Test whether the ray passes the aperture or not
		if (sqLength > sqAperture) {
			break;
		}


		//Refraction with lens element
		if (currentEl.radius != 0.0) {
			Vec3Df newDest;
			float refractIn = currentEl.nd;
			float refractOut;

			// If the refractory index of the next lens-element is 0; avoid division by zero
			// The refractory index should by standard be 1.0 because the tabular data assumes
			// the correct order refractory indices. E.g. after a lens with nd = 1,6 there might
			// follow a lens with an nd of 0.0 (or 1.0) not because the physical lens has 
			// that refraction, but because there was air between the lenses.
			if (cursor > 0 && currentLens.lensElements[cursor - 1].nd != 0) {
				refractOut = currentLens.lensElements[cursor - 1].nd;
			}
			else {
				refractOut = 1;
			}

			Vec3Df transmissiveRay;

			// Refract the ray
			if (!refractMyRay(refractIn, refractOut, normal, -1 * ray, transmissiveRay)) {
				cout << "   > No transmission! - " << endl;
				break;
			}
			cout << "   > Transmission! - " << endl;
			rayDest = intersectPos + transmissiveRay;	// Re-direct the ray-destination by refraction
		}
		glPointSize(10.0);
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_LINES);
		glVertex3f(rayOrigin[0], rayOrigin[1], rayOrigin[2]);
		glVertex3f(rayDest[0], rayDest[1],rayDest[2]);
		glEnd();
		cursor--;
	}
}

void Camera::traceLensRay(float x, float y, Vec3Df & origin, Vec3Df & destination)
{

	if (lensRadius > 0.0) {

		float a, b;
		a = rand() / RAND_MAX;
		b = rand() / RAND_MAX;

		//Map ray to lens-point
		Vec3Df lensPoint = lensRadius * sampleLens(a, b);
		//Scale by lensradius
		lensPoint = lensRadius * lensPoint;

		// Determine multiplier from dest to plane of focus(z is constant on viewplane :) )
		float rayMultiplier = focalDistance / destination[2];
		// And then the point on the focus-plane
		Vec3Df focusPoint = destination * rayMultiplier;

		// Externally change the ray origin and destination according to above findings
		origin[0] = lensPoint[0];	// Origin x is now on lensPoint x
		origin[1] = lensPoint[1];	// Origin y is now on lenspoint y
		origin[2] = 0.0;		// Might be a huge mistake..?

		destination = focusPoint - origin;
		destination.normalize();
	}
}

bool Camera::planeIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos, float & res)
{
	float dot_normalLine = Vec3Df::dotProduct(pointB, planeNormal);
	if (dot_normalLine > 1e-5) {
		Vec3Df crossingVector = planePos - pointA;
		res = Vec3Df::dotProduct(crossingVector, planeNormal) / dot_normalLine;
		return (res >= 0);
	}
}

Vec3Df Camera::sampleLens(float a, float b)
{
	float u, v;
	u = 2.0 * a - 1;
	v = 2.0 * b - 1;

	// At origin we should just return the origin instead of passing it to the monte-carlo integrator
	if (u == 0 && v == 0) return Vec3Df(0.0, 0.0, 0.0);			// I use Vec3Df for 2D points with z ignored

	float rPolar;
	float polarAngle;

	if (abs(u) > abs(v)) {
		rPolar = u;
		polarAngle = (M_PI / 4.0) * (v / rPolar);
	}
	else {
		rPolar = v;
		polarAngle = (M_PI / 2.0) - (M_PI / 4.0) * (v / rPolar);
	}
	return Vec3Df(cosf(polarAngle), sinf(polarAngle), 0.0);
}







bool Camera::diskIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos, float radius, Vec3Df & pointOnPlane)
{
	float dot_normalLine = Vec3Df::dotProduct(pointB, planeNormal);
	if (dot_normalLine > 1e-5) {
		Vec3Df crossingVector = planePos - pointA;
		float res = Vec3Df::dotProduct(crossingVector, planeNormal) / dot_normalLine;

		// If true; intersects the plane of the disk
		if (res >= 0) {
			pointOnPlane = pointA + res * pointB;
			Vec3Df vectorToPoint = pointOnPlane - planePos;
			float magnitude = Vec3Df::dotProduct(vectorToPoint, vectorToPoint);
			//disk test
			return (magnitude <= (radius * radius));
		}
	}
}

// Intersect brol

bool Camera::refractMyRay(float n1, float n2, const Vec3Df & normal, const Vec3Df rayIncident, Vec3Df & transmissiveRay) {
	float index = n1 / n2;
	Vec3Df rayNorm = rayIncident;
	rayNorm.normalize();
	float cosIn = Vec3Df::dotProduct(normal, rayNorm);
	float sqSine = (0.0, 1 - cosIn * cosIn) * index * index;		// sin2(Oi) = 1-cos2(Oi) -> re-use cos2 term

	if (sqSine > 1.0) {								// Reject ray if there is complete internal reflection
		cout << "TIR: " << sqSine << endl;
		cout << "N1/N2: " << n1 << " / " << n2 << endl;

		return false;
	}

	// Compute the transmissive ray by t> = n1/n2 * i> - (n1/n2 * cos(Oi) + sqrt(1 - sqSine) * normal))
	transmissiveRay = (index * rayNorm - (index + sqrtf(1 - sqSine)) * normal) * normal;
	transmissiveRay.normalize();
	return true;
}

// Geometric intersection test
bool Camera::intersectWithSphere(float radius, Vec3Df spherePos, const Vec3Df & origin, const Vec3Df & dest, Vec3Df & intersectPos, Vec3Df & normal)
{

	Vec3Df ray_sphereCenter = spherePos - origin;
	Vec3Df ray = dest - origin;
	ray.normalize();					// ayylmao never 5get 2 normalize

	cout << "Ray to sphereC: " << ray_sphereCenter << endl;
	cout << "Ray to Destination: " << ray << endl;
	cout << "SpherePos: " << spherePos << endl;
	cout << "origin: " << origin << endl;
	cout << "dest: " << dest << endl;

	/*
	glColor3f(0.0, 1.0, 0.6);
	glBegin(GL_LINES);
	glVertex3f(spherePos[0], spherePos[1], spherePos[2]);
	glVertex3f(origin[0], origin[1], origin[2]);
	glEnd();
	*/

	// Angle between displacement vector of ray and sphere center
	float angle_SphereC_dest = Vec3Df::dotProduct(ray, ray_sphereCenter);
	cout << "Angle between " << angle_SphereC_dest << endl;

	//if (angle_SphereC_dest < 0.0) { cout << "1" << endl; return false; }

	float length_diff = Vec3Df::dotProduct(ray_sphereCenter, ray_sphereCenter)
		- (angle_SphereC_dest * angle_SphereC_dest);

	if (length_diff > (radius*radius)) { cout << "2" << endl;  return false; }


	float s1, s2, discr;
	discr = sqrtf((radius*radius) - length_diff);
	s1 = angle_SphereC_dest - discr;			// Two intersections....
	s2 = angle_SphereC_dest + discr;



	s1 = std::fminf(fabs(s1), fabs(s2));				// Pick closest of intersections

	if (s1 < 0.0) {
		s1 = s2;							// Try other option (If inside of the sphere)
		if (s1 < 0.0) {						// Tough luck
			cout << "3" << endl;
			return false;
		}
	}

	intersectPos = origin + s1 * ray;


	normal = intersectPos - spherePos;
	normal.normalize();

	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(intersectPos[0], intersectPos[1], intersectPos[2]);
	glVertex3f(intersectPos[0] + normal[0] * 4, intersectPos[1] + normal[1] * 4, intersectPos[2] + normal[2] * 4);
	glEnd();
	/*
	glColor3f(1.0, 1.0, 0.6);
	glBegin(GL_LINES);
	glVertex3f(spherePos[0], spherePos[1], spherePos[2]);
	glVertex3f(intersectPos[0], intersectPos[1], intersectPos[2]);
	glEnd();
	*/

	return true;
}