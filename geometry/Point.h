#ifndef _POINT_H
#define _POINT_H

#include <float.h>
#include <math.h>
#include <utility>
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
    bool operator<(const Point& ref) const;

    int GetTaxicabDistance(const Point& ref);

    int x;
    int y;
};

typedef std::pair<double, Point> DistPointPair;

class DistPointPairCompare
{
public:
  bool operator()(const DistPointPair& l, const DistPointPair& r)
  {
    if ( fabs(l.first - r.first) > FLT_MIN )
      return (l.first < r.first);
    return (l.second < r.second);
  }
};

#endif
