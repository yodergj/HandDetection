#ifndef HAND_H
#define HAND_H

#include <string>
using std::string;

#define NUM_HAND_COLORS 4

#define POSTURE_CLOSED_STR "closed"
#define POSTURE_FIST_STR "fist"
#define POSTURE_FOUR_STR "four"
#define POSTURE_HANGLOOSE_STR "hangloose"
#define POSTURE_OPEN_STR "open"
#define POSTURE_PINKY_STR "pinky"
#define POSTURE_POINT_STR "point"
#define POSTURE_THREE_STR "three"
#define POSTURE_THUMB_STR "thumb"
#define POSTURE_TWO_STR "two"
#define POSTURE_VULCAN_STR "vulcan"

class Hand
{
  public:
    Hand();
    ~Hand();
    void GetBounds(int &left, int &right, int &top, int &bottom);
    bool SetBounds(int left, int right, int top, int bottom);
    string GetPostureString();
    void SetPostureString(const string& postureStr);
    const unsigned char* GetPostureColor(int colorNum);
  private:
    int mLeft;
    int mRight;
    int mTop;
    int mBottom;
    string mPostureStr;
    const unsigned char* mPostureColors[NUM_HAND_COLORS];
};

#endif
