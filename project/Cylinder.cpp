/* Gabe Yoder
   CS660
   Course Project
*/

#include "Cylinder.h"
#include <math.h>
#include <stdlib.h>

Cylinder::Cylinder()
{
  points = NULL;
  faces = NULL;
  numFaces = 0;
}

Cylinder::~Cylinder()
{
  if ( points )
    free(points);
  if ( faces )
    free(faces);
}

void Cylinder::SetSize(double height, double radius)
{
  int i;

  numFaces = 4 * CIRCLE_SIDES;
  points = (Point *)realloc(points,(2*CIRCLE_SIDES+2)*sizeof(Point));
  faces = (Triangle *)realloc(faces,numFaces*sizeof(Triangle));

  for (i=0; i < CIRCLE_SIDES; i++)
  {
    points[i].x = radius * cos(2*M_PI*i/CIRCLE_SIDES);
    points[i].y = 0;
    points[i].z = radius * sin(2*M_PI*i/CIRCLE_SIDES);
#if 0
    points[i+CIRCLE_SIDES].x = 2 * points[i].x;
#else
    points[i+CIRCLE_SIDES].x = points[i].x;
#endif
    points[i+CIRCLE_SIDES].y = -height;
    points[i+CIRCLE_SIDES].z = points[i].z;

    faces[i].points[0] = i;
    faces[i].points[1] = (i+1)%CIRCLE_SIDES;
    faces[i].points[2] = 2*CIRCLE_SIDES;

    faces[i+CIRCLE_SIDES].points[0] = i;
    faces[i+CIRCLE_SIDES].points[1] = (i+1)%CIRCLE_SIDES;
    faces[i+CIRCLE_SIDES].points[2] = i+CIRCLE_SIDES;

    faces[i+2*CIRCLE_SIDES].points[0] = i + CIRCLE_SIDES;
    faces[i+2*CIRCLE_SIDES].points[1] = (i+1)%CIRCLE_SIDES + CIRCLE_SIDES;
    faces[i+2*CIRCLE_SIDES].points[2] = 2*CIRCLE_SIDES + 1;

    faces[i+3*CIRCLE_SIDES].points[0] = i + CIRCLE_SIDES;
    faces[i+3*CIRCLE_SIDES].points[1] = (i+1)%CIRCLE_SIDES + CIRCLE_SIDES;
    faces[i+3*CIRCLE_SIDES].points[2] = (i+1)%CIRCLE_SIDES;
  }
  points[2*CIRCLE_SIDES].x = 0;
  points[2*CIRCLE_SIDES].y = 0;
  points[2*CIRCLE_SIDES].z = 0;

  points[2*CIRCLE_SIDES+1].x = 0;
  points[2*CIRCLE_SIDES+1].y = -radius;
  points[2*CIRCLE_SIDES+1].z = 0;
}
