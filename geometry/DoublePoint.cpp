#include "DoublePoint.h"
#include "Point.h"

DoublePoint::DoublePoint(double cx, double cy)
{
  x = cx;
  y = cy;
}

DoublePoint::DoublePoint(const DoublePoint& ref)
{
  operator=(ref);
}

DoublePoint::DoublePoint(const Point& ref)
{
  operator=(ref);
}

DoublePoint& DoublePoint::operator=(const DoublePoint& ref)
{
  x = ref.x;
  y = ref.y;
  return *this;
}

DoublePoint& DoublePoint::operator=(const Point& ref)
{
  x = ref.x;
  y = ref.y;
  return *this;
}

DoublePoint::~DoublePoint()
{
}

bool Point::operator==(const Point& ref) const
{
  return (x == ref.x) && (y == ref.y);
}
