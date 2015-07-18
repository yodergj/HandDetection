#include <string.h>
#include <string>
#include <QtCore/QCoreApplication>
#include "AdaboostClassifier.h"
#include "Image.h"
#include "VideoDecoder.h"
#include "ColorRegion.h"
#include "HandyTracker.h"

int main(int argc, char *argv[])
{
#if 0
  QCoreApplication a(argc, argv);
  return a.exec();
#endif

  int i, j, k;
  int width, height;
  int numWeakClassifiers;
  int classIndex = 0;
  string className;
  Image image;
  VideoDecoder* videoDecoder = new VideoDecoder;
  AdaboostClassifier handClassifier;
  HandyTracker tracker;
  char filename[256];
  FILE *file;
  int revNumber = 0;
  std::vector<double> aspectRatios;

  if ( argc < 6 )
  {
    printf("Usage: %s <class name> <num weak classifiers> <hand class Image> [...] -x <hand non-class image> [...]\n", argv[0]);
    return 0;
  }

  className = argv[1];

  numWeakClassifiers = atoi(argv[2]);
  if ( numWeakClassifiers < 1 )
  {
    printf("Invalid number of weak classifiers %d\n", numWeakClassifiers );
    return 1;
  }

  if ( !handClassifier.Create(HandyTracker::mNumFeatures, 2, numWeakClassifiers) )
  {
    printf("Failed creating classifier\n");
    return 1;
  }
  
  std::string featureStr("abcdefghijklmnopq");
  handClassifier.SetFeatureString(featureStr);

  for (i = 3; i < argc; i++)
  {
    if ( !strcmp(argv[i], "-x") )
    {
      classIndex = 1;
      continue;
    }
    if ( image.Load(argv[i]) )
    {
      if ( classIndex == 0 )
        printf("Processing hand class image %s\n", argv[i]);
      else
        printf("Processing other hand image %s\n", argv[i]);
      width = image.GetWidth();
      height = image.GetHeight();
      Point startPt(width / 2, height / 2);
      ColorRegion region;
      if ( region.Grow(image, startPt) )
      {
        Matrix featureData;
        if ( tracker.GenerateFeatureData(&region, featureData) )
          handClassifier.AddTrainingData(featureData, classIndex);
        else
          fprintf(stderr, "Failed generating feature data\n");
      }
      else
        fprintf(stderr, "Failed getting color region\n");
    }
    else
    {
      videoDecoder->SetFilename(argv[i]);
      if ( videoDecoder->Load() )
      {
        if ( classIndex == 0 )
          printf("Processing hand class video %s\n", argv[i]);
        else
          printf("Processing other hand video %s\n", argv[i]);

        int frameNumber = 0;
        ColorRegion* oldRegion = 0;
        ColorRegion* region = 0;
        videoDecoder->UpdateFrame();
        Image* img = videoDecoder->GetFrame();
        // img will be null when we hit the end of the stream
        while ( img )
        {
          printf("Processing frame %d\n", ++frameNumber);
          width = img->GetWidth();
          height = img->GetHeight();

          delete oldRegion;
          oldRegion = region;
          region = new ColorRegion;

          if ( oldRegion )
          {
            if ( !region->TrackFromOldRegion(*img, *oldRegion) )
              fprintf(stderr, "Failed tracking color region from old region\n");
          }
          else
          {
            Point startPt(width / 2, height / 2);
            if ( !region->Grow(*img, startPt) )
              fprintf(stderr, "Failed growing color region\n");
          }

          if ( !region->Empty() )
          {
            Matrix featureData;
            if ( tracker.GenerateFeatureData(region, featureData) )
            {
              handClassifier.AddTrainingData(featureData, classIndex);
              if ( classIndex == 0 )
                aspectRatios.push_back( featureData.GetValue(16, 0) );
            }
            else
              fprintf(stderr, "Failed generating feature data\n");
          }

          videoDecoder->UpdateFrame();
          img = videoDecoder->GetFrame();
        }
        delete oldRegion;
        delete region;
      }
      else
        fprintf(stderr, "Failed opening image/video %s\n", argv[i]);

      delete videoDecoder;
      videoDecoder = new VideoDecoder;
    }
  }

  printf("Starting training\n");
  if ( !handClassifier.Train() )
  {
    printf("Failed training the hand classifier\n");
    return 1;
  }

  do
  {
    sprintf(filename, "adahand-%s-%d.rev%d.cfg", className.c_str(), numWeakClassifiers, revNumber);
    file = fopen(filename, "r");
    if ( file )
      fclose(file);
    revNumber++;
  } while ( file );

  handClassifier.Save(filename);

  double aspectMin = FLT_MAX;
  double aspectMax = 0;
  double aspectTotal = 0;
  for (i = 0; i < (int)aspectRatios.size(); i++)
  {
    aspectTotal += aspectRatios[i];
    if ( aspectRatios[i] < aspectMin )
      aspectMin = aspectRatios[i];
    if ( aspectRatios[i] > aspectMax )
      aspectMax = aspectRatios[i];
  }
  double aspectMean = aspectTotal / aspectRatios.size();

  sprintf(filename, "%s-aspectRatio.data", className.c_str());
  file = fopen(filename, "w");
  fprintf(file, "Min %f\nMean %f\nMax %f\n", aspectMin, aspectMean, aspectMax);
  fclose(file);

  return 0;
}
