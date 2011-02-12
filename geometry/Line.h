#ifndef _LINE_H
#define _LINE_H

class DoublePoint;
class Point;

class Line
{
  public:
    Line(double x = 0);
    Line(double slope, double yIntercept);
    Line(double slope, const Point& pt);
    Line(double slope, const DoublePoint& pt);
    Line(const Line& ref);
    Line(const Point& a, const Point& b);
    Line(const DoublePoint& a, const DoublePoint& b);
    virtual Line& operator=(const Line& ref);
    virtual ~Line();
    virtual bool operator==(const Line& ref) const;
    double GetAngleRad() const;
    double GetAngleDeg() const;
    double GetInnerAngleRad(const Line& ref) const;
    double GetInnerAngleDeg(const Line& ref) const;
    virtual void Scale(int xScale, int yScale);
    virtual void Translate(int xOffset, int yOffset);
    Line GetOrthogonalLine(const Point& pt) const;
    Line GetOrthogonalLine(const DoublePoint& pt) const;
    bool PointAboveLine(const Point& pt) const;
    bool PointAboveLine(const DoublePoint& pt) const;
    bool PointOnLine(const Point& pt) const;
    bool PointOnLine(const DoublePoint& pt) const;
    bool PointBelowLine(const Point& pt) const;
    bool PointBelowLine(const DoublePoint& pt) const;
  protected:
    bool mIsVertical;
    double mSlope;
    double mYIntercept;
    double mXIntercept;
};

#endif
