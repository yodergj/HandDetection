#ifndef _DOUBLE_POINT_H
#define _DOUBLE_POINT_H

class Point;

class DoublePoint
{
  public:
    DoublePoint(double cx = 0, double cy = 0);
    DoublePoint(const DoublePoint& ref);
    DoublePoint(const Point& ref);
    DoublePoint& operator=(const DoublePoint& ref);
    DoublePoint& operator=(const Point& ref);
    ~DoublePoint();
    bool operator==(const DoublePoint& ref) const;

    double x;
    double y;
};

#endif
