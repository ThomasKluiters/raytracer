# This file should be viewed as a -*- mode: Makefile -*-

#########################################
# The goal part: what to make
#########################################
TEMPLATE = app
CONFIG   = console glut debug_and_release

INCLUDEPATH += .


HEADERS=\
 	argumentParser.h\
	trackball.h\


SOURCES=\
 main.cpp\

#LIBS+= -lglut



