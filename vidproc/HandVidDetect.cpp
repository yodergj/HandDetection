#include <string>
#include "VideoDecoder.h"
#include "FleshDetector.h"
#include "HandDetector.h"
#include "TimingAnalyzer.h"

using std::string;

int main(int argc, char* argv[])
{
  VideoDecoder decoder;
  FleshDetector fleshDetector;
  HandDetector handDetector;
  Image* inputImage;
  Image* fleshImage;
  Image* confidenceImage;
  Image outlineImage;
  int frameNumber = 0;
  string vidFilename;
  char outputFilename[1024];
  char* dirName;
  int i, j;
  int numFleshRegions, numHands, xScale, yScale;
  int left, right, top, bottom;
  vector<ConnectedRegion*>* fleshRegionVector;
  vector<Hand*> hands;
  unsigned char boxColor[] = {255, 255, 255};

  if ( argc != 5 )
  {
    printf("Usage: %s <flesh classifier file> <hand classifier file> <video file> <output directory>\n", argv[0]);
    return 1;
  }

  if ( !fleshDetector.Load(argv[1]) )
  {
    fprintf(stderr, "Error loading flesh detector %s\n", argv[1]);
    return 1;
  }

  if ( !handDetector.Load(argv[2]) )
  {
    fprintf(stderr, "Error loading hand detector %s\n", argv[2]);
    return 1;
  }

  vidFilename = argv[3];
  decoder.SetFilename(vidFilename);
  if ( !decoder.Load() )
  {
    fprintf(stderr, "Error loading video %s\n", argv[3]);
    return 1;
  }

  dirName = argv[4];

  while ( decoder.UpdateFrame() )
  {
    hands.clear();
    inputImage = decoder.GetFrame();

    outlineImage = *inputImage;
    TimingAnalyzer_Start(0);
    if ( fleshDetector.Process(inputImage, NULL, &fleshImage, &confidenceImage) )
    {
      TimingAnalyzer_Stop(0);
      fleshRegionVector = fleshDetector.GetFleshRegions(inputImage, xScale, yScale);
      if ( fleshRegionVector )
      {
        numFleshRegions = fleshRegionVector->size();
        for (i = 0; i < numFleshRegions; i++)
        {
          if ( !(*fleshRegionVector)[i]->GetBounds(left, right, top, bottom) )
          {
            fprintf(stderr, "Error getting flesh block %d bounds\n", i);
            return 1;
          }
          left *= xScale;
          right = (right + 1) * xScale - 1;
          top *= yScale;
          bottom = (bottom + 1) * yScale - 1;
          if ( (right - left + 1 < 20) || (bottom - top + 1 < 20) )
            continue;

          if ( !handDetector.Process(inputImage, left, right, top, bottom, hands) )
          {
            fprintf(stderr, "Error detecting hand in flesh block %d\n", i);
            return 1;
          }
          numHands = hands.size();
          for (j = 0; j < numHands; j++)
          {
            hands[j]->GetBounds(left, right, top, bottom);
            outlineImage.DrawBox(boxColor, 3, left, top, right, bottom);
          }
        }
      }

      sprintf(outputFilename, "%s/flesh%05d.ppm", dirName, frameNumber);
      fleshImage->Save(outputFilename);
      sprintf(outputFilename, "%s/confidence%05d.ppm", dirName, frameNumber);
      confidenceImage->Save(outputFilename);

      sprintf(outputFilename, "%s/frame%05d.ppm", dirName, frameNumber);
      outlineImage.Save(outputFilename);
    }

    frameNumber++;
  }
  printf("FleshDetector Process Time Min: %d\tMax: %d\tMean: %d\n",
         TimingAnalyzer_Min(0), TimingAnalyzer_Max(0), TimingAnalyzer_Mean(0));
  printf("FleshDetector GetFleshImage Time Min: %d\tMax: %d\tMean: %d\n",
         TimingAnalyzer_Min(1), TimingAnalyzer_Max(1), TimingAnalyzer_Mean(1));
  printf("FleshDetector GetOutlineImage Time Min: %d\tMax: %d\tMean: %d\n",
         TimingAnalyzer_Min(2), TimingAnalyzer_Max(2), TimingAnalyzer_Mean(2));
  printf("FleshDetector GetFleshConfidenceImage Time Min: %d\tMax: %d\tMean: %d\n",
         TimingAnalyzer_Min(3), TimingAnalyzer_Max(3), TimingAnalyzer_Mean(3));
  printf("FleshDetector CalcConfidence Time Min: %d\tMax: %d\tMean: %d\n",
         TimingAnalyzer_Min(4), TimingAnalyzer_Max(4), TimingAnalyzer_Mean(4));

  return 0;
}