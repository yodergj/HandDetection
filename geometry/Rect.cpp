#include <math.h>
#include "Rect.h"
#include "Point.h"

Rect::Rect()
{
}

Rect::Rect(const Point& a, const Point& b, const Point& c, const Point& d)
{
  mPoints[0] = a;
  mPoints[1] = b;
  mPoints[2] = c;
  mPoints[3] = d;
  mWidth = sqrt( (a.x - d.x) * (a.x - d.x) + (a.y - d.y) * (a.y - d.y) );
  mHeight = sqrt( (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) );
}

Rect::Rect(const DoublePoint& a, const DoublePoint& b, const DoublePoint& c, const DoublePoint& d)
{
  mPoints[0] = a;
  mPoints[1] = b;
  mPoints[2] = c;
  mPoints[3] = d;
  mWidth = sqrt( (a.x - d.x) * (a.x - d.x) + (a.y - d.y) * (a.y - d.y) );
  mHeight = sqrt( (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) );
}

Rect& Rect::operator=(const Rect& ref)
{
  mPoints[0] = ref.mPoints[0];
  mPoints[1] = ref.mPoints[1];
  mPoints[2] = ref.mPoints[2];
  mPoints[3] = ref.mPoints[3];
  mWidth = ref.mWidth;
  mHeight = ref.mHeight;
  return *this;
}

Rect::~Rect()
{
}

bool Rect::operator==(const Rect& ref) const
{
  return (mPoints[0] == ref.mPoints[0]) && (mPoints[1] == ref.mPoints[1]) && (mPoints[2] == ref.mPoints[2]) && (mPoints[3] == ref.mPoints[3]);
}

DoublePoint Rect::GetPoint(int index) const
{
  if ( (index < 0) || (index >= 4) )
    return DoublePoint();
  return mPoints[index];
}

double Rect::GetWidth() const
{
  return mWidth;
}

double Rect::GetHeight() const
{
  return mHeight;
}

double Rect::GetArea() const
{
  return mWidth * mHeight;
}

double Rect::GetAspectRatio() const
{
  return mWidth / mHeight;
}

void Rect::Scale(int xScale, int yScale)
{
  for (int i = 0; i < 4; i++)
  {
    mPoints[i].x *= xScale;
    mPoints[i].y *= yScale;
  }
  mWidth = sqrt( (mPoints[0].x - mPoints[3].x) * (mPoints[0].x - mPoints[3].x) +
                 (mPoints[0].y - mPoints[3].y) * (mPoints[0].y - mPoints[3].y) );
  mHeight = sqrt( (mPoints[0].x - mPoints[1].x) * (mPoints[0].x - mPoints[1].x) +
                  (mPoints[0].y - mPoints[1].y) * (mPoints[0].y - mPoints[1].y) );
}

void Rect::Translate(int xOffset, int yOffset)
{
  for (int i = 0; i < 4; i++)
  {
    mPoints[i].x += xOffset;
    mPoints[i].y += yOffset;
  }
}