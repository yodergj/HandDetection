#include <stdio.h>
#include <opencv/highgui.h>
#include "Image.h"

int main(int argc, char* argv[])
{
  IplImage* frame;
  Image image;

  if ( argc != 2 )
  {
    printf("Usage: %s <image>\n", argv[0]);
    return 0;
  }

#if 1
  frame = cvLoadImage(argv[1]);
  image.CopyIplImage(frame);
  image.Save("foo.png");

  cvReleaseImage(&frame);
#else
  image.Load(argv[1]);
  cvSaveImage("foo.png", image.GetIplImage());
#endif

  return 0;
}