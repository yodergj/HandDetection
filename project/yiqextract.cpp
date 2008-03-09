#include <stdio.h>
#include <qapplication.h>
#include <qimage.h>

int main(int argc, char *argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  QImage yImage, iImage, qImage;
  QImage iqImage;
  QString basename;
  QColor color;
  int j;
  int u, v;
  int width, height;
  int r, g, b;
  double y, i, q;

  if ( argc < 2 )
    return 0;

  for (j=1; j<argc; j++)
  {
    basename = argv[j];
    basename.truncate( basename.findRev('.') );
    if ( inputImage.load(argv[j]) )
    {
      width = inputImage.width();
      height = inputImage.height();
      yImage.create(width,height,32);
      iImage.create(width,height,32);
      qImage.create(width,height,32);
      iqImage.create(width,height,32);
      for (v=0; v<height; v++)
      {
        for (u=0; u<width; u++)
        {
          color = inputImage.pixel(u,v);
          color.getRgb(&r, &g, &b);
          y =   r * .299 + g *  .587 + b *  .114;
          i = ((r * .596 + g * -.275 + b * -.321)/.596 + 255)/2;
          q = ((r * .212 + g * -.523 + b *  .311)/.523 + 255)/2;

          yImage.setPixel(u, v, qRgb((int)y,(int)y,(int)y));
          iImage.setPixel(u, v, qRgb((int)i,(int)i,(int)i));
          qImage.setPixel(u, v, qRgb((int)q,(int)q,(int)q));
          iqImage.setPixel(u, v, qRgb((int)i,0,(int)q));
        }
      }
      yImage.save(basename + "_y.png","PNG");
      iImage.save(basename + "_i.png","PNG");
      qImage.save(basename + "_q.png","PNG");
      iqImage.save(basename + "_iq.png","PNG");
    }
  }

  return 0;
}
