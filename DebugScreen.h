#ifndef DEBUGSCREEN_H
#define DEBUGSCREEN_H
#include <string.h>
#include <map>
#include <chrono>
#include "Vec3D.h"

class DebugScreen
{

public:
	DebugScreen(std::string scenedata, unsigned int * x_res, unsigned int * y_res, void * font);

	void putString(std::string description, std::string value);
	void putBool(std::string description, bool value);
	void putInt(std::string description, int &value);
	void putVector(std::string description, Vec3Df *vector);
	void putChrono(std::string description, std::chrono::duration<double> value);
	void putFloat(std::string description, float value);
	void drawDebugInfo();
	void updateValues();

	std::string sceneData;
	void toggleOverlay();
	void traceUpdate(float value);
	void traceEnd();


	void drawLineHR(int offset_y);
	void drawLineVR(int offset_x);

	std::string rayTraceProgress(float progressPercentage);
	float renderProgress;

private:
	std::map <std::string, int> integers_to_track;
	std::map <std::string, bool> booleans_to_track;
	std::map <std::string, std::string>  strings_to_track;
	std::map <std::string, float> floats_to_track;
	std::map <std::string, std::chrono::duration<double> > durations;
	std::map <std::string, Vec3Df*> vectors_to_display;
	std::map <std::string, float> floats_to_display;

	void * Debug_font;
	unsigned int * x_resolution;
	unsigned int * y_resolution;
	bool show_overlay;
	Vec3Df color_text;

	// Dynamic settings
	int CURRENT_Y_OFFSET;		// Indicates the current distance from the top of the window.

	// Manual settings.
	const int CHAR_OFFSET = 18;						 // Offset in pixels between each line drawn
	const int BORDER_OFFSET = 20;					 // Offset for the distance from edge of window, to start drawing.
	int X_OFFSET;									 // Initial offset for x-axis drawing.
	const std::string RESOLUTION_ID = "Resolution";

	std::string toString(bool val);
	std::string updateResolution();
	std::string toString(std::string description, int val);
	std::string toString(std::string description, unsigned int val);
	std::string format(std::string descr, std::string val);
	std::string format(std::string descr, int val);
	std::string format(std::string descr, bool val);
	std::string format(std::string descr, float val);
	std::string format(std::string descr, double val);
	std::string format(std::string descr, Vec3Df val);
	std::string format(std::string descr, std::chrono::duration<double> duration);

	void printDurations(int x_offset);
	void printStrings(int x_offset);
	void printFloats(int x_offset);
	void printVectors(int x_offset);

	void printToFrame(std::string toPrint);
	void printLines(int x_offset);			
	void resetOffsetY();				// Clears the vertical offset back to the border-value.
	void pushGL();
	void popGL();
};

#endif
