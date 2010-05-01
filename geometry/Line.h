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
    Line& operator=(const Line& ref);
    ~Line();
    bool operator==(const Line& ref) const;
    double GetAngle() const;
    double GetInnerAngle(const Line& ref) const;
  private:
    bool mIsVertical;
    double mSlope;
    double mYIntercept;
    double mXIntercept;
};

#endif
