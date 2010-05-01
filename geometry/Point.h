#ifndef _POINT_H
#define _POINT_H

class Point
{
  public:
    Point(int cx = 0, int cy = 0);
    Point(const Point& ref);
    Point& operator=(const Point& ref);
    ~Point();
    bool operator==(const Point& ref) const;

    int x;
    int y;
};

#endif
