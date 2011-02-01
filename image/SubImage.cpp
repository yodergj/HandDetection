#include "SubImage.h"
#include "DoublePoint.h"

SubImage::SubImage()
{
  mParent = 0;
}

SubImage::SubImage(const SubImage& ref)
{
  *this = ref;
}

SubImage::~SubImage()
{
}

SubImage::SubImage(Image* parent, int left, int right, int top, int bottom)
{
  CreateFromParent(parent, left, right, top, bottom);
}

SubImage& SubImage::operator=(const SubImage& ref)
{
  if ( &ref != this )
  {
    Image::operator=(ref);
    mParent = ref.mParent;
    mOffset = ref.mOffset;
  }

  return *this;
}

DoublePoint SubImage::GetParentCoords(const DoublePoint& pt)
{
  return DoublePoint(pt.x + mOffset.x, pt.y + mOffset.y);
}

Point SubImage::GetParentCoords(const Point& pt)
{
  return Point(pt.x + mOffset.x, pt.y + mOffset.y);
}

Point SubImage::GetParentCoords(int x, int y)
{
  return GetParentCoords( Point(x, y) );
}

DoublePoint SubImage::GetTopLevelCoords(const DoublePoint& pt)
{
  DoublePoint coords(pt.x + mOffset.x, pt.y + mOffset.y);
  SubImage* parent = dynamic_cast<SubImage*>(mParent);
  while ( parent )
  {
    coords.x += parent->mOffset.x;
    coords.y += parent->mOffset.y;
    parent = dynamic_cast<SubImage*>(parent->mParent);
  }
  return coords;
}

Point SubImage::GetTopLevelCoords(const Point& pt)
{
  Point coords(pt.x + mOffset.x, pt.y + mOffset.y);
  SubImage* parent = dynamic_cast<SubImage*>(mParent);
  while ( parent )
  {
    coords.x += parent->mOffset.x;
    coords.y += parent->mOffset.y;
    parent = dynamic_cast<SubImage*>(parent->mParent);
  }
  return coords;
}

Point SubImage::GetTopLevelCoords(int x, int y)
{
  return GetTopLevelCoords( Point(x, y) );
}

bool SubImage::CreateFromParent(Image* parent, int left, int right, int top, int bottom)
{
  if ( !parent || (left > right) || (top > bottom) || (right < 0) || (bottom < 0) ||
       (right >= parent->GetWidth()) || (bottom >= parent->GetHeight()) )
    return false;

  int width, height, srcLineWidth, destLineWidth;
  unsigned char* destLine;
  unsigned char* srcLine;

  mParent = parent;
  mOffset.x = left;
  mOffset.y = top;

  MarkBufferAsUpdated();
  width = right - left + 1;
  height = bottom - top + 1;
  SetSize(width, height);
  srcLineWidth = parent->GetWidth() * 3;
  destLineWidth = mWidth * 3;
  srcLine = parent->GetRGBBuffer() + top * srcLineWidth + left * 3;
  destLine = mBuffer;
  for (int i = 0; i < height; i++, srcLine += srcLineWidth, destLine += destLineWidth)
    memcpy(destLine, srcLine, destLineWidth);

  return true;
}