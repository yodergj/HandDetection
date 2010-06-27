#ifndef _LINE_H
#define _LINE_H

class DoublePoint;
class Point;

class Line
{
  public:
    Line(double x = 0);
    Line(double slope, double yIntercept);
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
  protected:
    bool mIsVertical;
    double mSlope;
    double mYIntercept;
    double mXIntercept;
};

#endif
