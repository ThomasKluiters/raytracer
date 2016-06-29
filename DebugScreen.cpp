
#include <GL/glut.h>
#include <string.h>
#include <sstream>
#include <map>
#include "DebugScreen.h"
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include "Vec3D.h"

using namespace std;
typedef std::map<std::string, std::chrono::duration<double> > chronomap;

DebugScreen::DebugScreen(std::string scenedata, Camera *cam, unsigned int * x_res, unsigned int * y_res, void * font)
{
	camera = cam;
	sceneData = scenedata;
	durations = durations;
	x_resolution = x_res;
	y_resolution = y_res;
	Debug_font = font;
	strings_to_track[RESOLUTION_ID] = updateResolution();
	X_OFFSET = 10;
	CURRENT_Y_OFFSET = BORDER_OFFSET;
	color_text = Vec3Df(0.6, 0.6, 0.0);
	show_overlay = true;
	SHOW_RENDER_BUFFER = false;
	SHOW_OPTICS = false;
}

	// Converts boolean to string.
	std::string DebugScreen::toString(bool val) {
		if (val) {
			return "true";
		}
		else {
			return "false";
		}
	}


	// Returns the current resolution as a string.
	std::string DebugScreen::updateResolution()
	{
		std::ostringstream sstream;
		sstream << *x_resolution << " x " << *y_resolution;
		return sstream.str();
	}

	// Concatenates the given integer with description.
	std::string DebugScreen::toString(std::string description, int val) 
	{
		std::ostringstream sstream;
		sstream << description << val;
		return sstream.str();
	}

	// Concatenates the given float with description.
	std::string DebugScreen::format(std::string description, float val) 
	{
		std::ostringstream sstream;
		sstream << description << val;
		return sstream.str();
	}

	// Concatenates the given Vec3D with description.
	std::string DebugScreen::format(std::string description, Vec3Df val) 
	{
		std::ostringstream sstream;
		sstream << description << ": " << "<"
			<< val[0] << ", "
			<< val[1] << ", "
			<< val[2] << ">";

		return sstream.str();
	}

	// Concatenates the given double with description.
	std::string DebugScreen::format(std::string description, double val) 
	{
		std::ostringstream sstream;
		sstream << description << val;
		return sstream.str();
	}

	// Concatenates the given chrono-duration with the given description.
	std::string DebugScreen::format(std::string description, std::chrono::duration<double> val) {
		std::ostringstream sstream;
		sstream << description << ": " << val.count() << "s";
		return sstream.str();
	}

	// Concatenates the given unsigned integer with description.
	std::string DebugScreen::toString(std::string description, unsigned int val)
	{
		std::ostringstream sstream;
		sstream << description << val;
		return sstream.str();
	}

	// Concatenates and formats the given string-string
	std::string DebugScreen::format(std::string descr, std::string val) {
		std::ostringstream sstream;
		sstream << descr << ": " << val;
		return sstream.str();
	}

	// Concatenates and formats the given string-int.
	std::string DebugScreen::format(std::string descr, int val) {
		std::ostringstream sstream;
		sstream << descr << ": "  << val;
		return sstream.str();
	}

	// Concatenates and formats the given string-bool
	std::string DebugScreen::format(std::string descr, bool val) {
		std::ostringstream sstream;
		sstream << descr << ": " << toString(val);
		return sstream.str();
	}

	// Prints the given string to the current viewport.
	void DebugScreen::printToFrame(std::string toPrint)
	{
		for (std::string::iterator it = toPrint.begin(); it != toPrint.end(); ++it) {
			char character = *it;
			glutBitmapCharacter(Debug_font, character);
		}
	}

	// All static strings that are cumbersome to manage by reference can 
	// be updated here on each redraw. Not really efficient, but ok.
	void DebugScreen::updateValues() {
		strings_to_track[RESOLUTION_ID] = updateResolution();
	}

	// Resets the vertical offset to the border-offset.
	void DebugScreen::resetOffsetY() {
		CURRENT_Y_OFFSET = BORDER_OFFSET;
	}

	void DebugScreen::printDurations(int x_offset)
	{
		chronomap::iterator it_durations = durations.begin();
		while (CURRENT_Y_OFFSET <= (int)*y_resolution && it_durations != durations.end()) {
			glRasterPos2i(x_offset, CURRENT_Y_OFFSET);
			std::string line = format((*it_durations).first, (*it_durations).second);
			printToFrame(line);

			++it_durations;
			CURRENT_Y_OFFSET += CHAR_OFFSET;
		}
	}

	// Print the content of the string-map to the location indicated by x_offset.
	void DebugScreen::printStrings(int x_offset) 
	{
		std::map<string, string>::iterator it_strings = strings_to_track.begin();

		while (CURRENT_Y_OFFSET <= (int)*y_resolution && it_strings != strings_to_track.end()) {
			glRasterPos2i(x_offset, CURRENT_Y_OFFSET);
			std::string line = format((*it_strings).first, (*it_strings).second);
			printToFrame(line);

			++it_strings;
			CURRENT_Y_OFFSET += CHAR_OFFSET;
		}
	}

	// Print the content of the string-map to the location indicated by x_offset.
	void DebugScreen::printFloats(int x_offset) 
	{
		std::map<string, float>::iterator it_floats = floats_to_track.begin();

		while (CURRENT_Y_OFFSET <= (int)*y_resolution && it_floats != floats_to_track.end()) {
			glRasterPos2i(x_offset, CURRENT_Y_OFFSET);
			std::string line = format((*it_floats).first, (*it_floats).second);
			printToFrame(line);

			++it_floats;
			CURRENT_Y_OFFSET += CHAR_OFFSET;
		}
	}

	void DebugScreen::printVectors(int x_offset)
	{
		std::map <std::string, Vec3Df*>::iterator it_vectors =  vectors_to_display.begin();
		
		while (CURRENT_Y_OFFSET <= (int)*y_resolution && it_vectors != vectors_to_display.end()) {
			glRasterPos2i(x_offset, CURRENT_Y_OFFSET);
			std::string line = format((*it_vectors).first, *((*it_vectors).second));
			printToFrame(line);

			++it_vectors;
			CURRENT_Y_OFFSET += CHAR_OFFSET;
		}
	}

	// Prints all the debug-info in increments defined in CHAR_OFFSET, over de range of the resolution.
	// The rest is clipped for now. x_offset determines the x_location where writing starts.
	// Currently prints the strings, booleans and integers in one go.
	void DebugScreen::printLines(int x_offset)
	{
		std::map<string, bool>::iterator it_booleans = booleans_to_track.begin();
		std::map<string, int>::iterator it_integers = integers_to_track.begin();
		chronomap::iterator it_durations = durations.begin();

		CURRENT_Y_OFFSET = BORDER_OFFSET;
		std::string line;
		updateValues();

		printStrings(x_offset);

		while (CURRENT_Y_OFFSET <= (int)*y_resolution && it_booleans != booleans_to_track.end()) {
			glRasterPos2i(x_offset, CURRENT_Y_OFFSET);
			line = format((*it_booleans).first, (*it_booleans).second);
			printToFrame(line);

			++it_booleans;
			CURRENT_Y_OFFSET += CHAR_OFFSET;
		}

		while (CURRENT_Y_OFFSET <= (int)*y_resolution && it_integers != integers_to_track.end()) {
			glRasterPos2i(x_offset, CURRENT_Y_OFFSET);
			line = format((*it_integers).first, (*it_integers).second);
			printToFrame(line);

			++it_integers;
			CURRENT_Y_OFFSET += CHAR_OFFSET;
		}
	}






// Public
	void DebugScreen::putString(std::string description, std::string value) 
	{
		strings_to_track[description] = value;
		//cout << "Loaded \n <" << description << ", " << value << ">" << endl;
	}

	void DebugScreen::putBool(std::string description, bool value) 
	{
		booleans_to_track[description] = value;
		//cout << "Loaded \n <" << description << ", " << value << ">" << endl;
	}

	// Puts the given description / integer pair in the map.
	void DebugScreen::putInt(std::string description, int value) 
	{
		integers_to_track[description] = value;
		//cout << "Loaded \n <" << description << ", " << value << ">" << endl;
	}

	// Puts the given description / Vec3Df in the map
	void DebugScreen::putVector(std::string description, Vec3Df * vector) 
	{
		vectors_to_display[description] = vector;
		//cout << "Loaded \n " << format(description, vector) << endl;
	}

	// Puts the given description / Chrono duration in the map
	void DebugScreen::putChrono(std::string description, std::chrono::duration<double> value) 
	{
		durations[description] = value;
		//cout << "Loaded \n " << format(description, value) << endl;
	}

	void DebugScreen::putFloat(std::string description, float value)
	{
		floats_to_display[description] = value;
		//cout << "Loaded \n " << format(description, value) << endl;
	}

	void DebugScreen::toggleOverlay() 
	{
		show_overlay = !show_overlay;
	}

	void DebugScreen::toggleRenderOverlay()
	{
		SHOW_RENDER_BUFFER = !SHOW_RENDER_BUFFER;
	}

	void DebugScreen::toggleOptics()
	{
		SHOW_OPTICS = !SHOW_OPTICS;
	}

	void DebugScreen::drawLineHR(int offset_x) 
	{
		glBegin(GL_LINE_STRIP);
		glColor3f(0.8, 0.1, 0.0);
		glVertex2f(offset_x, 0.0);

		glVertex2f(offset_x, *y_resolution);
		glEnd();

	}

	void DebugScreen::drawLineVR(int offset_y) 
	{
		glBegin(GL_LINE_STRIP);
		glColor3f(0.8, 0.1, 0.0);
		glVertex2f(0.0, offset_y);

		glVertex2f(*x_resolution, offset_y);
		glEnd();

	}
	void DebugScreen::traceUpdate(float renderProgress) 
	{
		strings_to_track["Rendering"] = rayTraceProgress(renderProgress);
	}

	void DebugScreen::traceEnd()
	{
		strings_to_track["Rendering"] = "false";
	}

	std::string DebugScreen::rayTraceProgress(float progressPercentage)
	{
		std:ostringstream sstream;
		sstream << 
				(((float)((int)(progressPercentage * 100)) / 100)) << " %";
		return sstream.str();
	}

	/*	Debug-drawn text  */
	void DebugScreen::drawDebugInfo()
	{
		if (show_overlay) {
			pushGL();				
			glColor3f(color_text[0], color_text[1], color_text[3]);
			
			resetOffsetY();
			printLines(X_OFFSET);
			printDurations(X_OFFSET);
			printFloats(X_OFFSET);

			printVectors(X_OFFSET);

			if(SHOW_RENDER_BUFFER) setupScreenTexture();
			if (SHOW_OPTICS) {
				drawOptics();
			} 

			popGL();
		}
	}


	// Pushes the current frame-stack; freeing you up to write whatever you want.
	// -> Be sure to pop afterwards too.
	// Can be used to write your own data to the screen, but does not support accurate layering yet.
	void DebugScreen::pushGL()
	{
		glPushMatrix();
		glMatrixMode(GL_PROJECTION);

		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0f, (float)*x_resolution, (float)*y_resolution, 0.0f, -1.0f, 1.0f);

		glMatrixMode(GL_MODELVIEW);

		glLoadIdentity();
		glDisable(GL_CULL_FACE);

		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_LIGHTING);
	}

	// After the pushGL function has been called; unpop using this method.
	void DebugScreen::popGL()
	{
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);

		// Reset modelview and projection

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

	}

	void DebugScreen::drawCameraShape(Vec3Df position) {
		pushGL();
		popGL();
	}

	void DebugScreen::indicateOrigin() {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

		glColor4f(0.8, 1.0, 0.8, 0.6);
		
		glBegin(GL_LINES);
		glLineWidth(2.5);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 10.0, 0.0);

		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, -10.0, 0.0);
		glEnd();

		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
		glDisable(GL_BLEND);
		glPopAttrib();
		glEnable(GL_LIGHTING);
	}

	void DebugScreen::drawPlane(const Vec3Df &origin, float height, float width) 
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

		glColor4f(0.8, 0.2, 0.0, 0.2);
		glLineWidth(2.5);
		glBegin(GL_QUADS);
		glVertex3f(origin[0] + width, 0.0, origin[1] + height);
		glVertex3f(origin[0] + width, 0.0, origin[1] - height);
		glVertex3f(origin[0] - width, 0.0, origin[1] - height);
		glVertex3f(origin[0] - width, 0.0, origin[1] + height);
		glEnd();

		glColor4f(1.0, 0.2, 0.0, 0.4);
		glBegin(GL_LINES);
		glLineWidth(2.4);
		glVertex3f(origin[0], origin[1], origin[2]);
		glVertex3f(origin[0], origin[1], origin[2] + height);
		glEnd();

		glColor4f(1.0, 0.2, 0.0, 0.4);
		glBegin(GL_LINES);
		glVertex3f(origin[0], origin[1], origin[2]);
		glVertex3f(origin[0], origin[1], origin[2] - height);
		glEnd();

		glColor4f(1.0, 0.2, 0.0, 0.4);
		glBegin(GL_LINES);
		glVertex3f(origin[0], origin[1], origin[2]);
		glVertex3f(origin[0] + width, origin[1], origin[2]);
		glEnd();

		glColor4f(1.0, 0.2, 0.0, 0.4);
		glBegin(GL_LINES);
		glVertex3f(origin[0], origin[1], origin[2]);
		glVertex3f(origin[0] - width, origin[1], origin[2]);
		glEnd();

		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
		glDisable(GL_BLEND);
		glPopAttrib();
		glEnable(GL_LIGHTING);
	}


	void DebugScreen::drawLine(const Vec3Df &origin, const Vec3Df &dest, const Vec3Df color, float alpha) 
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);

		glColor4f(color[0], color[1], color[2], alpha);
		glBegin(GL_LINES);
		glVertex3f(origin[0], origin[1], origin[2]);
		glVertex3f(dest[0], dest[1], dest[2]);
		glEnd();

		glPopAttrib();
		glEnable(GL_LIGHTING);
	}

	void DebugScreen::drawLine(const Vec3Df &origin, const Vec3Df &dest, const Vec3Df color) 
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);

		glColor3f(color[0], color[1], color[2]);
		glBegin(GL_LINES);
		glVertex3f(origin[0], origin[1], origin[2]);
		glVertex3f(dest[0], dest[1], dest[2]);
		glEnd();

		glPopAttrib();
		glEnable(GL_LIGHTING);
	}

	// Draws all lines in the LineList
	void DebugScreen::drawAllLines() {
		for (LineList::iterator it = lines_to_draw.begin(); it != lines_to_draw.end(); ++it)
		{
			// Nested pairs; consists of <Coordinate-Pair<PointA, PointB>, Colour>
			drawLine(it->first.first, it->first.second, it->second);
		}
	}

	void DebugScreen::putLine(const Vec3Df & origin, const Vec3Df & dest, Vec3Df colour) 
	{
		lines_to_draw.push_back(ColouredLine(Line(origin, dest), colour));
	}

	void DebugScreen::setupScreenTexture() 
	{
		if(camera) {
			if(camera->sensor.isReady()) {
				glScalef(0.6, 0.6, 0.6);
				int x_start, y_start;
				x_start = *x_resolution - (*x_resolution * 0.39);
				y_start = *y_resolution - (*y_resolution * 0.39);
				glRasterPos2i(x_start, y_start);
				glPointSize(1);
				glBegin(GL_POINTS);
				
				for (int i = 0; i < camera->sensor.getFilmWidth(); i++) {
					for (int p = 0; p < camera->sensor.getFilmHeight(); p++) {
						glColor3f(camera->sensor.imageData[i][p][0],			// Accessing the image-data in the sensor of the camera
							camera->sensor.imageData[i][p][1],					// It's a Vec3Df inside a double stl vector.
							camera->sensor.imageData[i][p][2]					// First two brackets are width/height; third is (one of the three) component of the mathematical vector.
						);
						glVertex2i(x_start + i, y_start + p);
					}
				}
				glEnd();
			}	
		} 
		else {
			cout << "WARNING: CAMERA UNDEFINED" << endl;
		}
	}

	void DebugScreen::drawOptics() {
		pushGL();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_LIGHTING);

		float backPlaneOffset = 50.0;
		float x_scale = 0.9 * *x_resolution;
		Vec3Df start = Vec3Df(*x_resolution - backPlaneOffset, *y_resolution / 2.0, 0.0);		// Start at right-side of screen
		Vec3Df at = start - Vec3Df(1.0, 0.0, 0.0);
		at.normalize();

		glRasterPos2i(start[0], start[1]);

		glScalef(4.0,4.0,4.0);
		glTranslatef(-1200, -340, 0.0);
		//glTranslatef(-1200, 0, 0);

		Vec3Df dest0, dest1, dest2, dest3, dest4, dest5, dest6;

		Vec3Df origin = start - Vec3Df(0.0, 8.0, 0.0);

		float range = 7.0;
		for (float offset = range; offset > -range; offset = offset - 1.2) {
			origin = start - Vec3Df(0.0, offset, 0.0);
			dest0 = start - Vec3Df(200.0, offset, 0.0);
			traceLensSystem(at, start, origin, dest0);
			drawLine(origin, (dest0 - origin), Vec3Df(1.0, 0.0, 1.0));
		}

		// Draw optical axis
		drawLine(start, 
				 start - Vec3Df(x_scale, 0.0, 0.0),
				 Vec3Df(0.9,0.1,0.1));

		std::vector<LensElement>::const_iterator it_lens_front = camera->currentLens.itFront();
		std::vector<LensElement>::const_iterator it_lens_end = camera->currentLens.itEnd();
		int current = camera->currentLens.lensElements.size()-1;

		// Draw backplane
		drawLine(start - Vec3Df(0.0, 17.5, 0.0),
				 start + Vec3Df(0.0, 17.5, 0.0),
				 Vec3Df(0.5, 0.5, 0.6));

		// start is the cursor for the position of the current element.

		float lensLength = 0;
		while (current >= 0) {
			start[0] = start[0] - camera->currentLens.lensElements[current].thickness;
			lensLength += camera->currentLens.lensElements[current].radius;

			drawLensElement(start,
				camera->currentLens.lensElements[current].radius,
				camera->currentLens.lensElements[current].aperture / 2.0
			);
			current--;
		}
		glEnable(GL_LIGHTING);
		popGL();
	}

	void DebugScreen::drawLensElement(const Vec3Df & origin, const float radius, const float apertureRadius) {	

		// Is physical aperture
		if (radius == 0.0) {
			drawLine(origin + Vec3Df(0.0,apertureRadius, 0.0), 
					 origin + Vec3Df(0.0, apertureRadius*2, 0.0), 
							  Vec3Df(0.0,0.8,0.0));
			drawLine(origin - Vec3Df(0.0, apertureRadius, 0.0),
				origin - Vec3Df(0.0, apertureRadius*2, 0.0),
				Vec3Df(0.0, 0.8, 0.0));
		}
		// Is lensElement
		else {
			float angle = fabs(asinf(apertureRadius / radius));

			// Case convex
			if (radius > 0) {
				drawCircle(origin + Vec3Df(radius, 0.0, 0.0), radius, M_PI - angle, M_PI + angle);
			}
			// Case concave
			else {
				drawCircle(origin + Vec3Df(radius, 0.0, 0.0), -1.0*radius, -1.0*angle, angle);
			}
		}
	}

	// ORIGIN is origin of SPHERE!
	void DebugScreen::drawCircle(const Vec3Df & origin, float radius, float fromAngle, float toAngle) {
		
		if (fromAngle > toAngle) {
			cout << "WARNING: YOU ARE PASSING ANGLE-VALUES TO DebugScreen::drawCircle THAT ARE OUT OF BOUNDS \n -> Aborting!!";
			return;
		}
		glDisable(GL_LIGHTING);
			
		glColor3f(1.0, 1.0, 0.0);
		glPointSize(5.0);
		glBegin(GL_POINTS);
		glVertex2f(origin[0], origin[1]);
		glEnd();

		float fragments = M_PI_4 / 28;

		glPointSize(1.2);
		if(radius > 500) {
			glColor3f(1.0, 0.1, 0.9);	// If large radius
			cout << "LARGEG " << origin << endl;
			fragments = fragments / 40;
		}
		else
			glColor3f(0.1, 0.1, 0.9);

		glBegin(GL_POLYGON);
		for (double i = fromAngle; i < toAngle; i += fragments)
			glVertex3f(origin[0] + (cos(i) * radius), origin[1] + (sin(i) * radius), 0.0);
		glEnd();
		glEnable(GL_LIGHTING);
	}

	// rayOrigin is a point on the backpane (x and z are ignored in 2D)
	// opticalOrigin indicates the start of the optical axis
	void DebugScreen::traceLensSystem(Vec3Df at, Vec3Df & opticalOrigin, Vec3Df & rayOrigin, Vec3Df & rayDest) 
	{
		int cursor = camera->currentLens.lensElements.size() - 1;	// Set cursor on first lensElement from filmPlane
		Vec3Df pos_optical_axis = opticalOrigin;
		cout << "\n   > START ! - \n - O: " << rayOrigin << "\n - D: " << rayDest<< endl;

		while (cursor >= 0) {
			
			LensElement currentEl = camera->currentLens.lensElements[cursor];
			// Offset the center of the lens element by its thickness
			pos_optical_axis -= Vec3Df(currentEl.thickness, 0.0, 0.0);
			Vec3Df intersectPos, normal, ray;
			ray = rayDest - rayOrigin;
			ray.normalize();

			cout << "CURSOR! - " << cursor << endl;
			cout << "Lens! - " << camera->currentLens.lensElements[cursor].radius << endl;
			cout << "LensPosition! - " << pos_optical_axis << endl;
			cout << "RAYORIG - RAYDEST! - " << (rayDest - rayOrigin) << endl;
			//Determine intersection with either a spherical element or with the aperture stop.
			if (currentEl.radius == 0.0) {
				// We need to rescale the ray <origin, dest> so that dest falls on the aperture-disk
				// We can use linear scaling / similar triangles
				// Define the origin-point of the ray as our base, now;
				// -> Subtract pos_optical_axis(x-axis) from rayOrigin (x-axis) to get the offset.
				// -> Determine the same kind of offset between origin and dest.
				// -> These two form a scaling factor that we can use to rescale the dest-position on the aperture-plane.

				cout << "   > Is aperture! - " << endl;
				Vec3Df apertureNormal = Vec3Df(-1.0, 0.0, 0.0);
				apertureNormal.normalize();
				Vec3Df lensCenter = Vec3Df(pos_optical_axis[0] + 20000.0, pos_optical_axis[1], pos_optical_axis[2]);
				// DiskIntersect still has a bug
				//drawLine(pos_optical_axis, apertureNormal+pos_optical_axis, Vec3Df(1.0,1.0,1.0));
				// Deliberate technical debt lmao
				if (!intersectWithSphere(20000.0, lensCenter, rayOrigin, rayDest, intersectPos, normal)) {
					break;
				}
			//	if ((intersectPos - lensCenter).getLength() > currentEl.aperture) {
				//	break;
				//}

			}
			else {
				// Otherwise; we interact with the spherical lens
				// Determine the intersection and surface-normal of our ray and the lens-element
				Vec3Df lensCenter = Vec3Df(pos_optical_axis[0] + currentEl.radius, pos_optical_axis[1], pos_optical_axis[2]);


				cout << "Optical center! - " << lensCenter << endl;
				if (!intersectWithSphere(currentEl.radius, lensCenter, rayOrigin, rayDest, intersectPos, normal) )
				{
					cout << "   > MISSED! - " << endl;
					break;
				}
			}

			cout << "   > Intersects! -  <" << intersectPos << ">" << endl;
			/*glPointSize(8.0);
			glColor3f(1.0, 1.0, 0.6);
			glBegin(GL_POINTS);
			glVertex3f(intersectPos[0], intersectPos[1], intersectPos[2]);
			glEnd();*/

			drawLine(rayOrigin, intersectPos, Vec3Df(1.0,0.0,1.0));					// Draw line blue
			
			drawLine(intersectPos, intersectPos+normal*4, Vec3Df(1.0, 0.0,0.0));		// Draw normal red
			float sqLength = (intersectPos - pos_optical_axis).getLength();		// Get length from lenscenter to intersect
			float sqAperture = currentEl.aperture / 2.0;						

			rayOrigin = intersectPos;	// Set ray origin on the new Intersection

			// Test whether the ray passes the aperture or not
			if (sqLength > sqAperture) { 
				cout << "   > Outside aperture! - " << endl; 
				cout << "   >sqLen: " << sqLength << endl;
				cout << "   >sqApt: " << sqAperture << endl;
				break;
			}
			

			//Refraction with lens element
			if (currentEl.radius != 0.0) {
				Vec3Df newDest;
				float refractIn = currentEl.nd;

				// This is tricky; we need to test whether the next lens is joined with the 
				// other. If so; our rays will be parallel to the axis when the light travels between
				// the one end to the other. If not; we need to refract on every surface we hit.
				float refractOut;

				// If the refractory index of the next lens-element is 0; avoid division by zero
				if (cursor > 0 && camera->currentLens.lensElements[cursor - 1].nd != 0) {
					refractOut = camera->currentLens.lensElements[cursor - 1].nd;
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
			cursor--;
		}
	}

	bool DebugScreen::diskIntersect(const Vec3Df & pointA, const Vec3Df & pointB, Vec3Df & planeNormal, Vec3Df & planePos, float radius, Vec3Df & pointOnPlane)
	{
		float dot_normalLine = Vec3Df::dotProduct(pointB, planeNormal);
		if (dot_normalLine > 1e-6) {
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

	bool DebugScreen::refractMyRay(float n1, float n2, const Vec3Df & normal, const Vec3Df rayIncident, Vec3Df & transmissiveRay) {
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
	bool DebugScreen::intersectWithSphere(float radius, Vec3Df spherePos, const Vec3Df & origin, const Vec3Df & dest, Vec3Df & intersectPos, Vec3Df & normal) 
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
		
		glColor3f(1.0, 0.0,0.0);
		glBegin(GL_LINES);
		glVertex3f(intersectPos[0], intersectPos[1], intersectPos[2]);
		glVertex3f(intersectPos[0] + normal[0]*4, intersectPos[1] + normal[1]*4, intersectPos[2] + normal[2]*4);
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

