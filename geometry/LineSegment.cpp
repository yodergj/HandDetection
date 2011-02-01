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