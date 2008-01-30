#include "BayesianClassifier.h"
#include <qapplication.h>
#include <qimage.h>

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  QColor color;
  int j;
  int u, v;
  int width, height;
  int r, g, b;
  double y, i, q;
  BayesianClassifier classifier;
  Matrix input;
  int classIndex = 0;
  FILE *file;

  if ( argc < 4 )
    return 0;

  input.SetSize(2, 1);
  classifier.Create(2, 2);

  for (j=1; j<argc; j++)
  {
    if ( !strcmp(argv[j], "-x") )
    {
      classIndex = 1;
      continue;
    }
    if ( inputImage.load(argv[j]) )
    {
      if ( classIndex == 0 )
        printf("Processing hand image %s\n", argv[j]);
      else
        printf("Processing other image %s\n", argv[j]);
      width = inputImage.width();
      height = inputImage.height();
      for (v = 0; v < height; v++)
      {
        for (u = 0; u < width; u++)
        {
          color = inputImage.pixel(u,v);
          color.getRgb(&r, &g, &b);
          y =   r * .299 + g *  .587 + b *  .114;
          i = ((r * .596 + g * -.275 + b * -.321)/.596 + 255)/2;
          q = ((r * .212 + g * -.523 + b *  .311)/.523 + 255)/2;

          if ( (classIndex == 0) && (y <= 50) )
            continue;

          input.SetValue(0, 0, i / 255);
          input.SetValue(1, 0, q / 255);
          classifier.AddTrainingData(input, classIndex);
        }
      }
    }
  }

  printf("Starting training\n");
  classifier.Train();
  file = fopen("hand.cfg", "w");
  classifier.Save(file);
  fclose(file);

  return 0;
}
