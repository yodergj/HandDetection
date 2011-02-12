#ifndef _POINT_H
#define _POINT_H

class DoublePoint;

class Point
{
  public:
    Point(int cx = 0, int cy = 0);
    Point(const Point& ref);
    Point(const DoublePoint& ref);
    Point& operator=(const Point& ref);
    Point& operator=(const DoublePoint& ref);
    ~Point();
    bool operator==(const Point& ref) const;
    bool operator!=(const Point& ref) const;

    int x;
    int y;
};

#endif
