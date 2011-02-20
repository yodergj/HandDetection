#ifndef HAND_H
#define HAND_H

#include <string>
using std::string;

class Hand
{
  public:
    Hand();
    ~Hand();
    void GetBounds(int &left, int &right, int &top, int &bottom);
    bool SetBounds(int left, int right, int top, int bottom);
    string GetPostureString();
    void SetPostureString(const string& postureStr);
  private:
    int mLeft;
    int mRight;
    int mTop;
    int mBottom;
    string mPostureStr;
};

#endif
