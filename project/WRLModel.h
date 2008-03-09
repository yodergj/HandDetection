/* Gabe Yoder
   CS660
   Course Project
*/
#ifndef WRL_MODEL_H
#define WRL_MODEL_H

typedef struct
{
  double x, y, z;
} WRLPoint;

typedef struct
{
  int points[3];
} WRLTriangle;

class WRLModel
{
  public:
    WRLModel();
    ~WRLModel();
    bool Load(char *filename);
    WRLPoint *points;
    int numPoints;
    WRLTriangle *faces;
    int numFaces;
    double xSpan, ySpan, zSpan;
};

#endif
