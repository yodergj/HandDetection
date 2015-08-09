#include <string.h>
#include <string>
#include <QtCore/QCoreApplication>
#include "AdaboostClassifier.h"
#include "Image.h"
#include "VideoDecoder.h"
#include "ColorRegion.h"
#include "HandyTracker.h"
#include "VideoEncoder.h"

void ProcessFrame(Image* img, HandyTracker* tracker, bool& trackingInitialized, int frameNumber)
{
  if ( !img || !tracker )
    return;

  ColorRegion* region = 0;
  if ( trackingInitialized )
  {
    ColorRegion* oldRegion = tracker->GetRegion(frameNumber - 1);
    if ( !oldRegion )
    {
      fprintf(stderr, "Failed getting old region\n");
      return;
    }
    region = new ColorRegion;
    region->TrackFromOldRegion(*img, *oldRegion);

    if ( !tracker->AnalyzeRegion(region) )
    {
      fprintf(stderr, "Failed analyzing region");
      delete region;
      return;
    }
    region->FreeIntegralBuffer();
  }
  else
  {
    Point imgCenter(img->GetWidth() / 2, img->GetHeight() / 2);
    region = new ColorRegion;
    region->Grow(*img, imgCenter);

    if ( !tracker->AnalyzeRegionForInitialization(region) )
    {
      fprintf(stderr, "Failed analyzing region for initialization");
      delete region;
      return;
    }
    region->FreeIntegralBuffer();

    HandyTracker::HandState handClass = tracker->GetState(frameNumber);
    if ( handClass == HandyTracker::ST_OPEN )
      trackingInitialized = true;
  }
}

void DrawResults(Image* img, HandyTracker* tracker, int frameNumber)
{
  if ( !img || !tracker )
    return;

  HandyTracker::HandState handClass = tracker->GetState(frameNumber);
  ColorRegion* region = tracker->GetRegion(frameNumber);

  if ( !region )
    return;

  unsigned char color[3];
  if ( handClass == HandyTracker::ST_OPEN )
  {
    color[0] = 0;
    color[1] = 255;
    color[2] = 0;
  }
  else if ( handClass == HandyTracker::ST_CLOSED )
  {
    color[0] = 0;
    color[1] = 0;
    color[2] = 255;
  }
  else if ( handClass == HandyTracker::ST_CONFLICT )
  {
    color[0] = 0;
    color[1] = 255;
    color[2] = 255;
  }
  else if ( handClass == HandyTracker::ST_REJECT )
  {
    color[0] = 255;
    color[1] = 0;
    color[2] = 0;
  }
  else
  {
    color[0] = 255;
    color[1] = 255;
    color[2] = 255;
  }

  img->DrawBox(color, 2, region->GetMinX(), region->GetMinY(), region->GetMaxX(), region->GetMaxY());

  int refHeight = region->GetReferenceHeight();
  if ( refHeight > 0 )
  {
    int refY = region->GetMinY() + refHeight - 1;
    int refX = region->GetMinX() + region->GetFeatureWidth() - 1;
    img->DrawLine(color, 1, region->GetMinX(), refY, refX, refY);
    img->DrawLine(color, 1, refX, region->GetMinY(), refX, refY);
  }
}

int main(int argc, char *argv[])
{
#if 0
  QCoreApplication a(argc, argv);
  return a.exec();
#endif
  VideoDecoder* videoDecoder = new VideoDecoder;
  VideoEncoder* videoEncoder = 0;
  AdaboostClassifier* openClassifier = new AdaboostClassifier;
  AdaboostClassifier* closedClassifier = new AdaboostClassifier;
  HandyTracker tracker;

  if ( argc != 5 )
  {
    printf("Usage: %s <open classifier> <closed classifier> <input video> <output video>\n", argv[0]);
    return 0;
  }

  if ( !openClassifier->Load(argv[1]) )
  {
    fprintf(stderr, "Failed loading open classifier\n", argv[1]);
    return 1;
  }

  if ( !tracker.SetOpenClassifier(openClassifier) )
  {
    fprintf(stderr, "Failed setting open classifier\n");
    return 1;
  }

  if ( !closedClassifier->Load(argv[2]) )
  {
    fprintf(stderr, "Failed loading closed classifier\n", argv[2]);
    return 1;
  }

  if ( !tracker.SetClosedClassifier(closedClassifier) )
  {
    fprintf(stderr, "Failed setting closed classifier\n");
    return 1;
  }

  videoDecoder->SetFilename(argv[3]);
  if ( !videoDecoder->Load() )
  {
    fprintf(stderr, "Failed loading video <%s>\n", argv[3]);
    return 1;
  }

  if ( !videoDecoder->UpdateFrame() )
  {
    fprintf(stderr, "Failed updating frame\n");
    return 1;
  }

  int frameNumber = 0;
  bool trackingInitialized = false;
  Image* img = videoDecoder->GetFrame();
  while ( img )
  {
    if ( !videoEncoder )
    {
      videoEncoder = new VideoEncoder;
      if ( !videoEncoder->Open(argv[4], img->GetWidth(), img->GetHeight(), 25) )
      {
        fprintf(stderr, "Failed opening output video <%s>\n", argv[4]);
        return 1;
      }
    }

    ProcessFrame(img, &tracker, trackingInitialized, frameNumber);
    if ( trackingInitialized )
      DrawResults(img, &tracker, frameNumber);

    videoEncoder->AddFrame(img);

    if ( frameNumber > 1 )
      tracker.PurgeRegion(frameNumber - 2);
    frameNumber++;

    videoDecoder->UpdateFrame();
    img = videoDecoder->GetFrame();
  }
  videoEncoder->Close();

  return 0;
}
