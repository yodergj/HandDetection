/* Gabe Yoder
   CS660
   Course Project
*/

#include "Hand.h"
#include <math.h>

Hand::Hand()
{
#if 0
  PlaceBone(THUMB_TIP,    30, 5, -40, 80, 0, 0, 0);
  PlaceBone(THUMB_MIDDLE, 30, 5, -40, 40, 0, 0, 0);
  PlaceBone(THUMB_BOTTOM, 30, 5, -40,  0, 0, 0, 0);

  PlaceBone(INDEX_TIP,    30, 5, -20, 120, 0, 0, 0);
  PlaceBone(INDEX_MIDDLE, 30, 5, -20,  80, 0, 0, 0);
  PlaceBone(INDEX_BOTTOM, 30, 5, -20,  40, 0, 0, 0);
  PlaceBone(INDEX_HAND,   30, 5, -20,   0, 0, 0, 0);

  PlaceBone(MIDDLE_TIP,    30, 5, 0, 120, 0, 0, 0);
  PlaceBone(MIDDLE_MIDDLE, 30, 5, 0,  80, 0, 0, 0);
  PlaceBone(MIDDLE_BOTTOM, 30, 5, 0,  40, 0, 0, 0);
  PlaceBone(MIDDLE_HAND,   30, 5, 0,   0, 0, 0, 0);

  PlaceBone(RING_TIP,    30, 5, 20, 120, 0, 0, 0);
  PlaceBone(RING_MIDDLE, 30, 5, 20,  80, 0, 0, 0);
  PlaceBone(RING_BOTTOM, 30, 5, 20,  40, 0, 0, 0);
  PlaceBone(RING_HAND,   30, 5, 20,   0, 0, 0, 0);

  PlaceBone(PINKY_TIP,    30, 5, 40, 120, 0, 0, 0);
  PlaceBone(PINKY_MIDDLE, 30, 5, 40,  80, 0, 0, 0);
  PlaceBone(PINKY_BOTTOM, 30, 5, 40,  40, 0, 0, 0);
  PlaceBone(PINKY_HAND,   30, 5, 40,   0, 0, 0, 0);
#else
  PlaceBone(THUMB_TIP,      50, 14, 220, 130, 0, 0, -30);
  PlaceBone(THUMB_MIDDLE,   60, 14, 190, 100, 0, 0, -30);
  PlaceBone(THUMB_BOTTOM,   80, 14, 160,  60, 0, 0, -30);

  PlaceBone(INDEX_TIP,      40, 14, 120, 290, 0, 0,   0);
  PlaceBone(INDEX_MIDDLE,   50, 14, 120, 250, 0, 0,   0);
  PlaceBone(INDEX_BOTTOM,   80, 14, 120, 200, 0, 0,   0);
  PlaceBone(INDEX_HAND,    120, 14, 120, 120, 0, 0, -10);

  PlaceBone(MIDDLE_TIP,     45, 14,  80, 315, 0, 0,   0);
  PlaceBone(MIDDLE_MIDDLE,  60, 14,  80, 270, 0, 0,   0);
  PlaceBone(MIDDLE_BOTTOM,  90, 14,  80, 210, 0, 0,   0);
  PlaceBone(MIDDLE_HAND,   120, 14,  80, 120, 0, 0,   0);

  PlaceBone(RING_TIP,       40, 14,  40, 290, 0, 0,   0);
  PlaceBone(RING_MIDDLE,    50, 14,  40, 250, 0, 0,   0);
  PlaceBone(RING_BOTTOM,    80, 14,  40, 200, 0, 0,   0);
  PlaceBone(RING_HAND,     120, 14,  40, 120, 0, 0,   0);

  PlaceBone(PINKY_TIP,      40, 14,   0, 255, 0, 0,   0);
  PlaceBone(PINKY_MIDDLE,   40, 14,   0, 215, 0, 0,   0);
  PlaceBone(PINKY_BOTTOM,   75, 14,   0, 175, 0, 0,   0);
  PlaceBone(PINKY_HAND,    100, 14,   0, 100, 0, 0,  10);

  xSpan = 250;
  ySpan = 290;
  zSpan = 28;
#endif
}

Hand::~Hand()
{
}

void Hand::PlaceBone(BoneIndexType bone, double length, double radius,
                     double x, double y, double z,
                     double xRot, double zRot)
{
  bones[bone].SetSize(length,radius);
  locations[bone].point.x = x;
  locations[bone].point.y = y;
  locations[bone].point.z = z;
  locations[bone].XRotation = xRot;
  locations[bone].ZRotation = zRot;
}
