#include "Hand.h"

Hand::Hand()
{
  mLeft = -1;
  mRight = -1;
  mTop = -1;
  mBottom = -1;
}

Hand::~Hand()
{
}

void Hand::GetBounds(int &left, int &right, int &top, int &bottom)
{
  left = mLeft;
  right = mRight;
  top = mTop;
  bottom = mBottom;
}

bool Hand::SetBounds(int left, int right, int top, int bottom)
{
  if ( (left < 0) || (right < left) || (top < 0) || (bottom < top) )
    return false;

  mLeft = left;
  mRight = right;
  mTop = top;
  mBottom = bottom;

  return true;
}
