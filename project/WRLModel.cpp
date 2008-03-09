/* Gabe Yoder
   CS660
   Course Project
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "WRLModel.h"

WRLModel::WRLModel()
{
  points = NULL;
  numPoints = 0;
  faces = NULL;
  numFaces = 0;
  xSpan = 0;
  ySpan = 0;
  zSpan = 0;
}

WRLModel::~WRLModel()
{
  if ( points )
    free(points);
  if ( faces )
    free(faces);
}

bool WRLModel::Load(char *filename)
{
  int i;
  int len;
  FILE *file;
  char buf[64];
  bool inCoords = false;
  bool inIndex = false;
  double x, y, z;
  int pointsAllocated = 0;
  WRLPoint* tmpPoint;
  WRLTriangle* tmpTriangle;
  double minX, minY, minZ, maxX, maxY, maxZ;

  minX = 9999999999.9;
  maxX = -9999999999.9;
  minY = 9999999999.9;
  maxY = -9999999999.9;
  minZ = 9999999999.9;
  maxZ = -9999999999.9;

  if ( !filename )
    return false;

  file = fopen(filename,"r");
  if ( !file )
    return false;
  
  while ( fscanf(file, "%64s", buf) != EOF )
  {
    if ( inCoords )
    {
      x = strtod(buf, NULL);
      fscanf(file, "%64s", buf);
      y = strtod(buf, NULL);
      fscanf(file, "%64s", buf);
      z = strtod(buf, NULL);
      len = strlen(buf);
      if ( buf[len-1] == ']' )
        inCoords = false;
      pointsAllocated++;
      tmpPoint = (WRLPoint *)realloc(points,pointsAllocated * sizeof(WRLPoint));
      if ( !tmpPoint )
      {
        fprintf(stderr,"Error allocating points\n");
        fclose(file);
        return false;
      }
      numPoints++;
      points = tmpPoint;
      points[pointsAllocated-1].x = x;
      points[pointsAllocated-1].y = y;
      points[pointsAllocated-1].z = z;
      if ( x < minX )
        minX = x;
      if ( x > maxX )
        maxX = x;
      if ( y < minY )
        minY = y;
      if ( y > maxY )
        maxY = y;
      if ( z < minZ )
        minZ = z;
      if ( z > maxZ )
        maxZ = z;
    }
    else if ( inIndex )
    {
      tmpTriangle = (WRLTriangle *)realloc(faces,(numFaces+1)*sizeof(WRLTriangle));
      if ( !tmpTriangle )
      {
        fprintf(stderr,"Error allocating faces\n");
        fclose(file);
        return false;
      }
      faces = tmpTriangle;
      for (i=0; i<3; i++)
      {
        faces[numFaces].points[i] = atoi(buf);
        fscanf(file, "%64s", buf);
      }
      numFaces++;
      if ( atoi(buf) != -1 )
      {
        fprintf(stderr,"Error non-triangulsr face detected\n");
        fclose(file);
        return false;
      }
      len = strlen(buf);
      if ( buf[len-1] == ']' )
        inIndex = false;
    }
    else
    {
      if ( !strncmp(buf,"coord",64) )
      {
        /* Skip ahead to the actual coordinates */
        for (i=0; i<6; i++)
          fscanf(file, "%64s", buf);
        inCoords = true;
      }
      else if ( !strncmp(buf,"coordIndex",64) )
      {
        /* Skip ahead to the actual index values */
        fscanf(file, "%64s", buf);
        inIndex = true;
      }
    }
  }

  xSpan = maxX - minX;
  ySpan = maxY - minY;
  zSpan = maxZ - minZ;

  fclose(file);
  return true;
}
