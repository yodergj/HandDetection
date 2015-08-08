#include <string.h>
#include <string>
#include <QtCore/QCoreApplication>
#include "AdaboostClassifier.h"
#include "Image.h"
#include "VideoDecoder.h"
#include "ColorRegion.h"
#include "HandyTracker.h"

#define MIN_REGION_WIDTH  100
#define MIN_REGION_HEIGHT 100

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
  string className, listFilename, inputDataFilename;
  Image image;
  VideoDecoder* videoDecoder = new VideoDecoder;
  AdaboostClassifier handClassifier;
  HandyTracker tracker;
  char filename[512];
  char datafilename[512];
  char buf[1024];
  FILE *file;
  int revNumber = 0;
  std::vector<double> aspectRatios;

  if ( (argc < 4) || (argc > 6) )
  {
    printf("Usage: %s <class name> <num weak classifiers> [-d <data file>] [list file]\n", argv[0]);
    return 0;
  }

  className = argv[1];

  numWeakClassifiers = atoi(argv[2]);
  if ( numWeakClassifiers < 1 )
  {
    fprintf(stderr, "Invalid number of weak classifiers %d\n", numWeakClassifiers );
    return 1;
  }

  std::vector<std::string> fileVec;
  if ( !strcmp(argv[3], "-d") )
  {
    if ( argc == 4 )
    {
      printf("Usage: %s <class name> <num weak classifiers> [-d <data file>] [list file]\n", argv[0]);
      return 0;
    }
    inputDataFilename = argv[4];
    if ( argc == 6 )
      listFilename = argv[5];
  }
  else
    listFilename = argv[3];

  if ( !listFilename.empty() )
  {
    FILE* listFp = fopen(listFilename.c_str(), "r");
    if ( !listFp )
    {
      fprintf(stderr, "Failed opening list file <%s>\n", listFilename.c_str());
      return 1;
    }

    while ( fgets(buf, 1024, listFp) && !feof(listFp) )
    {
      std::string str = buf;
      if ( str.empty() )
        continue;

      while ( (str[str.size() - 1] == '\n') ||
        (str[str.size() - 1] == '\r') )
      {
        str = str.substr(0, str.size() - 1);
      }

      if ( !str.empty() )
        fileVec.push_back(str);
    }
    fclose(listFp);
  }

  if ( !handClassifier.Create(HandyTracker::mNumFeatures, 2, numWeakClassifiers) )
  {
    fprintf(stderr, "Failed creating classifier\n");
    return 1;
  }
  
  std::string featureStr;
  for (i = 0; i < HandyTracker::mNumFeatures; i++)
    featureStr += (char)('A' + i);
  handClassifier.SetFeatureString(featureStr);

  if ( !inputDataFilename.empty() )
  {
    if ( !handClassifier.LoadTrainingData(inputDataFilename.c_str()) )
    {
      fprintf(stderr, "Failed loading data file %s\n", inputDataFilename.c_str());
      return 1;
    }
  }

  for (i = 0; i < (int)fileVec.size(); i++)
  {
    if ( !strcmp(fileVec[i].c_str(), "-x") )
    {
      classIndex = 1;
      continue;
    }
    if ( image.Load(fileVec[i]) )
    {
      if ( classIndex == 0 )
        printf("Processing hand class image %s\n", fileVec[i].c_str());
      else
        printf("Processing other hand image %s\n", fileVec[i].c_str());
      width = image.GetWidth();
      height = image.GetHeight();
      Point startPt(width / 2, height / 2);
      ColorRegion region;
      if ( region.Grow(image, startPt) && (region.GetWidth() >= MIN_REGION_WIDTH) && (region.GetHeight() >= MIN_REGION_HEIGHT) )
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
      videoDecoder->SetFilename(fileVec[i]);
      if ( videoDecoder->Load() )
      {
        if ( classIndex == 0 )
          printf("Processing hand class video %s\n", fileVec[i].c_str());
        else
          printf("Processing other hand video %s\n", fileVec[i].c_str());

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

          if ( (region->GetWidth() < MIN_REGION_WIDTH) || (region->GetHeight() < MIN_REGION_HEIGHT) )
          {
            fprintf(stderr, "Rejecting color region for minimum size\n");
            delete region;
            region = oldRegion;
            oldRegion = 0;
          }
          else
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
        fprintf(stderr, "Failed opening image/video %s\n", fileVec[i].c_str());

      delete videoDecoder;
      videoDecoder = new VideoDecoder;
    }
  }

  do
  {
    sprintf(filename, "adahand-%s-%d.rev%d.cfg", className.c_str(), numWeakClassifiers, revNumber);
    sprintf(datafilename, "adahand-%s-%d.rev%d.data", className.c_str(), numWeakClassifiers, revNumber);
    file = fopen(filename, "r");
    if ( file )
      fclose(file);
    revNumber++;
  } while ( file );

  handClassifier.SaveTrainingData(datafilename);

  printf("Starting training\n");
  if ( !handClassifier.Train() )
  {
    printf("Failed training the hand classifier\n");
    return 1;
  }

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
