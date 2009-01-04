#ifndef HAND_H
#define HAND_H

class Hand
{
  public:
    Hand();
    ~Hand();
    void GetBounds(int &left, int &right, int &top, int &bottom);
    bool SetBounds(int left, int right, int top, int bottom);
  private:
    int mLeft;
    int mRight;
    int mTop;
    int mBottom;
};

#endif
