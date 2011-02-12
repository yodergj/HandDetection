#include "Point.h"
#include "DoublePoint.h"

Point::Point(int cx, int cy)
{
  x = cx;
  y = cy;
}

Point::Point(const Point& ref)
{
  operator=(ref);
}

Point::Point(const DoublePoint& ref)
{
  operator=(ref);
}

Point& Point::operator=(const Point& ref)
{
  x = ref.x;
  y = ref.y;
  return *this;
}

Point& Point::operator=(const DoublePoint& ref)
{
  if ( ref.x < 0 )
    x = (int)(ref.x - .5);
  else
    x = (int)(ref.x + .5);

  if ( ref.y < 0 )
    y = (int)(ref.y - .5);
  else
    y = (int)(ref.y + .5);

  return *this;
}

Point::~Point()
{
}

bool Point::operator==(const Point& ref) const
{
  return (x == ref.x) && (y == ref.y);
}

bool Point::operator!=(const Point& ref) const
{
  return (x != ref.x) || (y != ref.y);
}