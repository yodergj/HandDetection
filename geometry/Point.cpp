#include "Point.h"

Point::Point(int cx, int cy)
{
  x = cx;
  y = cy;
}

Point::Point(const Point& ref)
{
  operator=(ref);
}

Point& Point::operator=(const Point& ref)
{
  x = ref.x;
  y = ref.y;
  return *this;
}

Point::~Point()
{
}

bool Point::operator==(const Point& ref) const
{
  return (x == ref.x) && (y == ref.y);
}
