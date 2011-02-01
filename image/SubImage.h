#ifndef SUB_IMAGE_H
#define SUB_IMAGE_H

#include "Image.h"
#include "Point.h"

class SubImage: public Image
{
  public:
    SubImage();
    SubImage(const SubImage& ref);
    SubImage(Image* parent, int left, int right, int top, int bottom);
    virtual ~SubImage();
    SubImage& operator=(const SubImage& ref);
    DoublePoint GetParentCoords(const DoublePoint& pt);
    Point GetParentCoords(const Point& pt);
    Point GetParentCoords(int x, int y);
    DoublePoint GetTopLevelCoords(const DoublePoint& pt);
    Point GetTopLevelCoords(const Point& pt);
    Point GetTopLevelCoords(int x, int y);
    bool CreateFromParent(Image* parent, int left, int right, int top, int bottom);
  private:
    Image* mParent;
    Point mOffset;
};

#endif