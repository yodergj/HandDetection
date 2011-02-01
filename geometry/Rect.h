#ifndef _RECT_H
#define _RECT_H

#include "DoublePoint.h"

class Rect
{
  public:
    Rect();
    Rect(const Point& a, const Point& b, const Point& c, const Point& d);
    Rect(const DoublePoint& a, const DoublePoint& b, const DoublePoint& c, const DoublePoint& d);
    virtual Rect& operator=(const Rect& ref);
    virtual ~Rect();
    virtual bool operator==(const Rect& ref) const;
    DoublePoint GetPoint(int index) const;
    double GetWidth() const;
    double GetHeight() const;
    double GetArea() const;
    double GetAspectRatio() const;
    void Scale(int xScale, int yScale);
    void Translate(int xOffset, int yOffset);
  protected:
    DoublePoint mPoints[4];
    double mWidth;
    double mHeight;
};

#endif
