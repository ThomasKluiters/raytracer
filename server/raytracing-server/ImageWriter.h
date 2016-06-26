#ifndef IMAGE_JFDJKDFSLJFDFKSDFDJFDFJSDKSFJSDLF
#define IMAGE_JFDJKDFSLJFDFKSDFDJFDFJSDKSFJSDLF

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <windows.h>
#define _CRT_SECURE_NO_DEPRECATE
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <vector>

//Image class
//This class can be used to write your final result to an image.
//You can open the image using a PPM viewer.


class Image
{
public:
    Image(int width, int height)
    : _width(width)
    , _height(height)
    {
        _image.resize(3 * _width*_height);
    }
    
    void setValue(int i, float rgbValue) {
        _image[i] = rgbValue;
    }
    
    std::vector<float> _image;
    int _width;
    int _height;
    
    bool writeImage(const char * filename);
};

bool Image::writeImage(const char * filename)
{
    FILE* file;
#pragma warning (disable : 4996)
    file = fopen(filename, "wb");
    if (!file)
    {
        printf("dump file problem... file\n");
        return false;
    }
    
    fprintf(file, "P6\n%i %i\n255\n", _width, _height);
    
    std::vector<unsigned char> imageC(_image.size());
    
    for (unsigned int i = 0; i < _image.size(); ++i)
        imageC[i] = (unsigned char)(_image[i]);
    
    int t = fwrite(&(imageC[0]), _width * _height * 3, 1, file);
    if (t != 1)
    {
        printf("Dump file problem... fwrite\n");
        return false;
    }
    
    fclose(file);
    return true;
}

#endif