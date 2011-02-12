#include <math.h>
#include "LineSegment.h"

LineSegment::LineSegment() : Line()
{
}

LineSegment::LineSegment(const Point& a, const Point& b) : Line(a, b)
{
  mPoints[0] = a;
  mPoints[1] = b;
}

LineSegment::LineSegment(const DoublePoint& a, const DoublePoint& b) : Line(a, b)
{
  mPoints[0] = a;
  mPoints[1] = b;
}

LineSegment::LineSegment(const LineSegment& ref) : Line(ref)
{
  mPoints[0] = ref.mPoints[0];
  mPoints[1] = ref.mPoints[1];
}

LineSegment& LineSegment::operator=(const LineSegment& ref)
{
  Line::operator=(ref);
  mPoints[0] = ref.mPoints[0];
  mPoints[1] = ref.mPoints[1];
  return *this;
}

LineSegment::~LineSegment()
{
}

bool LineSegment::operator==(const LineSegment& ref) const
{
  // Point order doesn't matter for this class
  return ( (mPoints[0] == ref.mPoints[0] && mPoints[1] == ref.mPoints[1]) ||
           (mPoints[0] == ref.mPoints[1] && mPoints[1] == ref.mPoints[0]) );
}

DoublePoint LineSegment::GetFirstPoint() const
{
  return mPoints[0];
}

DoublePoint LineSegment::GetSecondPoint() const
{
  return mPoints[1];
}

DoublePoint LineSegment::GetLeftPoint() const
{
  if ( mPoints[0].x < mPoints[1].x )
    return mPoints[0];
  return mPoints[1];
}

DoublePoint LineSegment::GetRightPoint() const
{
  if ( mPoints[0].x > mPoints[1].x )
    return mPoints[0];
  return mPoints[1];
}

DoublePoint LineSegment::GetTopPoint() const
{
  if ( mPoints[0].y < mPoints[1].y )
    return mPoints[0];
  return mPoints[1];
}

DoublePoint LineSegment::GetBottomPoint() const
{
  if ( mPoints[0].y > mPoints[1].y )
    return mPoints[0];
  return mPoints[1];
}

double LineSegment::GetLength() const
{
  return sqrt( (mPoints[0].x - mPoints[1].x) * (mPoints[0].x - mPoints[1].x) +
               (mPoints[0].y - mPoints[1].y) * (mPoints[0].y - mPoints[1].y) );
}

void LineSegment::Scale(int xScale, int yScale)
{
  Line::Scale(xScale, yScale);
  mPoints[0].x *= xScale;
  mPoints[0].y *= yScale;
  mPoints[1].x *= xScale;
  mPoints[1].y *= yScale;
}

void LineSegment::Translate(int xOffset, int yOffset)
{
  Line::Translate(xOffset, yOffset);
  mPoints[0].x += xOffset;
  mPoints[0].y += yOffset;
  mPoints[1].x += xOffset;
  mPoints[1].y += yOffset;
}

void LineSegment::RotateAroundEnd(double radians, bool firstEnd)
{
  double cosVal = cos(radians);
  double sinVal = sin(radians);
    double newX, newY;
  int pt, origin;

  if ( firstEnd )
  {
    origin = 0;
    pt = 1;
  }
  else
  {
    origin = 1;
    pt = 0;
  }
  mPoints[pt].x -= mPoints[origin].x;
  mPoints[pt].y -= mPoints[origin].y;
  newX = mPoints[pt].x * cosVal - mPoints[pt].y * sinVal;
  newY = mPoints[pt].y * cosVal + mPoints[pt].x * sinVal;
  mPoints[pt].x = newX + mPoints[origin].x;
  mPoints[pt].y = newY + mPoints[origin].y;
}

DoublePoint LineSegment::GetVector() const
{
  return DoublePoint( mPoints[1].x - mPoints[0].x, mPoints[1].y - mPoints[0].y );
}

double LineSegment::GetAngleToLineRad(const LineSegment& ref) const
{
  double angle, refAngle, diff;

  angle = atan2(mPoints[1].y - mPoints[0].y, mPoints[1].x - mPoints[0].x);
  refAngle = atan2(ref.mPoints[1].y - ref.mPoints[0].y, ref.mPoints[1].x - ref.mPoints[0].x);
  diff = refAngle - angle;

  if ( diff > M_PI )
    diff -= 2 * M_PI;

  if ( diff < -M_PI )
    diff += 2 * M_PI;

  return diff;
}

double LineSegment::GetAngleToLineDeg(const LineSegment& ref) const
{
  double angle = GetAngleToLineRad(ref);
  return angle * 180 / M_PI;
}