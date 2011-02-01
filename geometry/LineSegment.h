#ifndef _LINE_SEGMENT_H
#define _LINE_SEGMENT_H

#include "Line.h"
#include "DoublePoint.h"

class LineSegment : public Line
{
  public:
    LineSegment();
    LineSegment(const Point& a, const Point& b);
    LineSegment(const DoublePoint& a, const DoublePoint& b);
    LineSegment(const LineSegment& ref);
    virtual LineSegment& operator=(const LineSegment& ref);
    virtual ~LineSegment();
    virtual bool operator==(const LineSegment& ref) const;
    DoublePoint GetLeftPoint() const;
    DoublePoint GetRightPoint() const;
    DoublePoint GetTopPoint() const;
    DoublePoint GetBottomPoint() const;
    double GetLength() const;
    virtual void Scale(int xScale, int yScale);
    virtual void Translate(int xOffset, int yOffset);
  protected:
    DoublePoint mPoints[2];
};

#endif
