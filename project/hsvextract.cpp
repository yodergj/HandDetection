#include <stdio.h>
#include <qapplication.h>
#include <qimage.h>

int main(int argc, char *argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  QImage hueImage, saturationImage, valueImage;
  QImage mixImage;
  QString basename;
  QColor color;
  int i;
  int x,y;
  int width, height;
  int h, s, v;

  if ( argc < 2 )
    return 0;

  for (i=1; i<argc; i++)
  {
    basename = argv[i];
    basename.truncate( basename.findRev('.') );
    if ( inputImage.load(argv[i]) )
    {
      width = inputImage.width();
      height = inputImage.height();
      hueImage.create(width,height,32);
      saturationImage.create(width,height,32);
      valueImage.create(width,height,32);
      mixImage.create(width,height,32);
      for (y=0; y<height; y++)
      {
        for (x=0; x<width; x++)
        {
          color = inputImage.pixel(x,y);
          color.getHsv(&h, &s, &v);
          if ( h > -1 )
            hueImage.setPixel(x, y, qRgb(h,h,h));
          else
            hueImage.setPixel(x, y, qRgb(255,0,0));
          saturationImage.setPixel(x, y, qRgb(s,s,s));
          valueImage.setPixel(x, y, qRgb(v,v,v));
          if ( h > -1 )
            mixImage.setPixel(x, y, qRgb(h,0,s));
          else
            mixImage.setPixel(x, y, qRgb(0,0,s));
        }
      }
      hueImage.save(basename + "_h.png","PNG");
      saturationImage.save(basename + "_s.png","PNG");
      valueImage.save(basename + "_v.png","PNG");
      mixImage.save(basename + "_m.png","PNG");
    }
  }

  return 0;
}
