#ifndef DISPLAYTOOLS_H
#define DISPLAYTOOLS_H

#include <stdlib.h>
#include <iostream>

#ifdef MAC_OS
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else

#ifdef WIN32
#include <Windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#endif

#define CurDataType float

void colorLookup(const int id, CurDataType &red, CurDataType &green, CurDataType &blue);
void material(CurDataType r, CurDataType g, CurDataType b, CurDataType a, CurDataType spec, CurDataType gloss);

#endif

