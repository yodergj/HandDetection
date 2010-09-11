#include <math.h>
#include "Line.h"
#include "DoublePoint.h"
#include "Point.h"

Line::Line(double x)
{
  mIsVertical = true;
  mSlope = 0;
  mYIntercept = 0;
  mXIntercept = x;
}

Line::Line(double slope, double yIntercept)
{
  mIsVertical = false;
  mSlope = slope;
  mYIntercept = yIntercept;
  mXIntercept = -mYIntercept / mSlope;
}

Line::Line(const Line& ref)
{
  operator=(ref);
}

Line::Line(const Point& a, const Point& b)
{
  if ( a.x == b.x )
  {
    mIsVertical = true;
    mSlope = 0;
    mYIntercept = 0;
    mXIntercept = a.x;
  }
  else
  {
    mIsVertical = false;
    mSlope = (b.y - a.y) / (double)(b.x - a.x);
    mYIntercept = -mSlope * a.x + a.y;
    mXIntercept = -mYIntercept / mSlope;
  }
}

Line::Line(const DoublePoint& a, const DoublePoint& b)
{
  if ( a.x == b.x )
  {
    mIsVertical = true;
    mSlope = 0;
    mYIntercept = 0;
    mXIntercept = a.x;
  }
  else
  {
    mIsVertical = false;
    mSlope = (b.y - a.y) / (b.x - a.x);
    mYIntercept = -mSlope * a.x + a.y;
    mXIntercept = -mYIntercept / mSlope;
  }
}

Line& Line::operator=(const Line& ref)
{
  mIsVertical = ref.mIsVertical;
  mSlope = ref.mSlope;
  mYIntercept = ref.mYIntercept;
  mXIntercept = ref.mXIntercept;
  return *this;
}

Line::~Line()
{
}

bool Line::operator==(const Line& ref) const
{
  if ( mIsVertical )
    return (mXIntercept == ref.mXIntercept);
  return (mSlope == ref.mSlope) && (mYIntercept == ref.mYIntercept);
}

double Line::GetAngleRad() const
{
  if ( mIsVertical )
    return M_PI / 2;

  if ( mSlope == 0 )
    return 0;

  return atan( 1 / mSlope );
}

double Line::GetAngleDeg() const
{
  double angle = GetAngleRad();
  return angle * 180 / M_PI;
}

double Line::GetInnerAngleRad(const Line& ref) const
{
  double diff;

  diff = fabs( GetAngleRad() - ref.GetAngleRad() );
  while ( diff > M_PI )
    diff -= M_PI;

  if ( diff > M_PI / 2 )
    diff = M_PI / 2 - diff;

  return diff;
}

double Line::GetInnerAngleDeg(const Line& ref) const
{
  double diff;

  diff = fabs( GetAngleDeg() - ref.GetAngleDeg() );
  while ( diff > 180 )
    diff -= 180;

  if ( diff > 90 )
    diff = 180 - diff;

  return diff;
}
