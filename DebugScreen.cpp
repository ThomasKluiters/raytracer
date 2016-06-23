
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

