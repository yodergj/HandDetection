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
    DoublePoint GetFirstPoint() const;
    DoublePoint GetSecondPoint() const;
    DoublePoint GetLeftPoint() const;
    DoublePoint GetRightPoint() const;
    DoublePoint GetTopPoint() const;
    DoublePoint GetBottomPoint() const;
    double GetLength() const;
    virtual void Scale(int xScale, int yScale);
    virtual void Translate(int xOffset, int yOffset);
    void RotateAroundEnd(double radians, bool firstEnd = true);
    DoublePoint GetVector() const;
    double GetAngleToLineRad(const LineSegment& ref) const;
    double GetAngleToLineDeg(const LineSegment& ref) const;
  protected:
    DoublePoint mPoints[2];
};

#endif
