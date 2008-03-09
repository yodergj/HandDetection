/* Gabe Yoder
   CS660
   Course Project
*/
#ifndef HAND_H
#define HAND_H

#include "Cylinder.h"

typedef enum
{
  THUMB_TIP,
  THUMB_MIDDLE,
  THUMB_BOTTOM,

  INDEX_TIP,
  INDEX_MIDDLE,
  INDEX_BOTTOM,
  INDEX_HAND,

  MIDDLE_TIP,
  MIDDLE_MIDDLE,
  MIDDLE_BOTTOM,
  MIDDLE_HAND,

  RING_TIP,
  RING_MIDDLE,
  RING_BOTTOM,
  RING_HAND,

  PINKY_TIP,
  PINKY_MIDDLE,
  PINKY_BOTTOM,
  PINKY_HAND,

  NUM_BONES
} BoneIndexType;

typedef struct
{
  Point point;
  double XRotation;
  double ZRotation;
} AnglePoint;

class Hand
{
  public:
    Hand();
    ~Hand();
    void PlaceBone(BoneIndexType bone, double length, double radius, double x, double y, double z, double xRot, double zRot);
    Cylinder bones[NUM_BONES];
    AnglePoint locations[NUM_BONES];
    double xSpan, ySpan, zSpan;
};

#endif
