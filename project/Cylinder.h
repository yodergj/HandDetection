/* Gabe Yoder
   CS660
   Course Project
*/
#ifndef CYLINDER_H
#define CYLINDER_H

#define CIRCLE_SIDES 32

typedef struct
{
  double x,y,z;
} Point;

typedef struct
{
  int points[3];
} Triangle;

class Cylinder
{
  public:
    Cylinder();
    ~Cylinder();
    void SetSize(double height, double radius);
    Point *points;
    Triangle *faces;
    int numFaces;
};

#endif
