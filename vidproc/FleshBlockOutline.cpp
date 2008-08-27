#include "VideoDecoder.h"
#include "FleshDetector.h"
#include "TimingAnalyzer.h"
#include <qapplication.h>

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  VideoDecoder decoder;
  FleshDetector fleshDetector;
  Image* inputImage;
  Image* fleshImage;
  Image* outlineImage;
  Image* confidenceImage;
  int frameNumber = 0;
  QString vidFilename;
  char outputFilename[1024];

  if ( argc < 4 )
  {
    printf("Usage: fleshblockoutline <classifier file> <video file> <output directory>\n");
    return 1;
  }

  if ( !fleshDetector.Load(argv[1]) )
  {
    fprintf(stderr, "Error loading flesh detector %s\n", argv[1]);
    return 1;
  }

  vidFilename = argv[2];
  decoder.SetFilename(vidFilename);
  if ( !decoder.Load() )
  {
    fprintf(stderr, "Error loading video %s\n", argv[2]);
    return 1;
  }

  while ( decoder.UpdateFrame() )
  {
    inputImage = decoder.GetMyFrame();

    TimingAnalyzer_Start(0);
    if ( fleshDetector.Process(inputImage, &outlineImage, &fleshImage, &confidenceImage) )
    {
      TimingAnalyzer_Stop(0);

      sprintf(outputFilename, "%s/flesh%05d.ppm", argv[3], frameNumber);
      fleshImage->Save(outputFilename);
      sprintf(outputFilename, "%s/frame%05d.ppm", argv[3], frameNumber);
      outlineImage->Save(outputFilename);
      sprintf(outputFilename, "%s/confidence%05d.ppm", argv[3], frameNumber);
      confidenceImage->Save(outputFilename);
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
  printf("BayesianClassifier Classify Time Min: %d\tMax: %d\tMean: %d\n",
         TimingAnalyzer_Min(4), TimingAnalyzer_Max(4), TimingAnalyzer_Mean(4));

  return 0;
}
