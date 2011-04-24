#include <string.h>
#include "Hand.h"

static unsigned char white[] = {255, 255, 255};
static unsigned char black[] = {0, 0, 0};
static unsigned char red[] = {255, 0, 0};
static unsigned char green[] = {0, 255, 0};
//static unsigned char blue[] = {0, 0, 255};
//static unsigned char magenta[] = {255, 0, 255};
static unsigned char yellow[] = {255, 255, 0};
//static unsigned char cyan[] = {0, 255, 255};

Hand::Hand()
{
  mLeft = -1;
  mRight = -1;
  mTop = -1;
  mBottom = -1;
  memset(mPostureColors, 0, NUM_HAND_COLORS * sizeof(const char *));
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

string Hand::GetPostureString()
{
  return mPostureStr;
}

void Hand::SetPostureString(const string& postureStr)
{
  mPostureStr = postureStr;

  if ( postureStr == POSTURE_CLOSED_STR )
  {
    mPostureColors[0] = white;
    mPostureColors[1] = white;
    mPostureColors[2] = white;
    mPostureColors[3] = white;
  }
  else if ( postureStr == POSTURE_POINT_STR )
  {
    mPostureColors[0] = black;
    mPostureColors[1] = white;
    mPostureColors[2] = white;
    mPostureColors[3] = white;
  }
  else if ( postureStr == POSTURE_TWO_STR )
  {
    mPostureColors[0] = white;
    mPostureColors[1] = black;
    mPostureColors[2] = white;
    mPostureColors[3] = white;
  }
  else if ( postureStr == POSTURE_THREE_STR )
  {
    mPostureColors[0] = black;
    mPostureColors[1] = black;
    mPostureColors[2] = white;
    mPostureColors[3] = white;
  }
  else if ( postureStr == POSTURE_FOUR_STR )
  {
    mPostureColors[0] = white;
    mPostureColors[1] = white;
    mPostureColors[2] = black;
    mPostureColors[3] = white;
  }
  else if ( postureStr == POSTURE_OPEN_STR )
  {
    mPostureColors[0] = black;
    mPostureColors[1] = white;
    mPostureColors[2] = black;
    mPostureColors[3] = white;
  }
  else if ( postureStr == POSTURE_FIST_STR )
  {
    mPostureColors[0] = green;
    mPostureColors[1] = green;
    mPostureColors[2] = green;
    mPostureColors[3] = green;
  }
  else if ( postureStr == POSTURE_THUMB_STR )
  {
    mPostureColors[0] = black;
    mPostureColors[1] = green;
    mPostureColors[2] = green;
    mPostureColors[3] = green;
  }
  else if ( postureStr == POSTURE_PINKY_STR )
  {
    mPostureColors[0] = green;
    mPostureColors[1] = green;
    mPostureColors[2] = green;
    mPostureColors[3] = black;
  }
  else if ( postureStr == POSTURE_VULCAN_STR )
  {
    mPostureColors[0] = yellow;
    mPostureColors[1] = white;
    mPostureColors[2] = white;
    mPostureColors[3] = yellow;
  }
  else if ( postureStr == POSTURE_HANGLOOSE_STR )
  {
    mPostureColors[0] = black;
    mPostureColors[1] = green;
    mPostureColors[2] = green;
    mPostureColors[3] = yellow;
  }
  else
  {
    // Unknown posture string
    mPostureColors[0] = red;
    mPostureColors[1] = red;
    mPostureColors[2] = red;
    mPostureColors[3] = red;
  }
}

const unsigned char* Hand::GetPostureColor(int colorNum)
{
  if ( (colorNum < 0) || (colorNum >= NUM_HAND_COLORS) )
    return 0;

  return mPostureColors[colorNum];
}