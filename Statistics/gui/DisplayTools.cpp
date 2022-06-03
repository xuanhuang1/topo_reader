#include "DisplayTools.h"

void colorLookup(const int id, CurDataType &red, CurDataType &green, CurDataType &blue) 
{
  if(id%12 == 0) { red = 205./256; green = 131./256.; blue = 131./256.; } 
  if(id%12 == 1) { red = 241./256.; green = 187./256.; blue = 115./256.; }
	if(id%12 == 2) { red = 232./256.; green = 150./256.; blue = 248./256.; }
	if(id%12 == 3) { red = 90./256.; green = 84./256.; blue = 134./256.; }
	if(id%12 == 4) { red = 185./256.; green = 153./256.; blue = 195./256.; }
	if(id%12 == 5) { red = 141./256.; green = 181./256.; blue = 185./256.; }
	if(id%12 == 6) { red = 133./256.; green = 194./256.; blue = 215./256.; } 
	if(id%12 == 7) { red = 150./256.; green = 150./256.; blue = 198./256.; }
	if(id%12 == 8) { red = 18./256.; green = 108./256.; blue = 61./256.; }
	if(id%12 == 9) { red = 117./256.; green = 231./256.; blue = 171./256.; }
	if(id%12 == 10) { red = 246./256.; green = 240./256.; blue = 154./256.; }
	if(id%12 == 11) { red = 192./256.; green = 244./256.; blue = 166./256.; }
}


void material(CurDataType r, CurDataType g, CurDataType b, CurDataType a, CurDataType spec, CurDataType gloss) 
{
  GLfloat array[4] ;
	array[0] = GLfloat( r ) ;
	array[1] = GLfloat ( g ) ;
	array[2] = GLfloat ( b ) ;
	array[3] = a ;
	glMaterialfv ( GL_FRONT_AND_BACK , GL_AMBIENT , array ) ;
	glMaterialfv ( GL_FRONT_AND_BACK, GL_DIFFUSE, array ) ;

	array[0] = GLfloat ( spec ) ;
	array[1] = GLfloat ( spec ) ;
	array[2] = GLfloat ( spec ) ;
	array[3] = 1.0;
	glMaterialfv ( GL_FRONT_AND_BACK, GL_SPECULAR, array ) ;

	array[0] = 0.0 ;
	array[1] = 0.0 ;
	array[2] = 0.0 ;
	array[3] = 1.0;
	glMaterialfv ( GL_FRONT_AND_BACK, GL_EMISSION, array ) ;
  glMaterialf ( GL_FRONT_AND_BACK, GL_SHININESS, GLfloat ( gloss ) ) ;
}

