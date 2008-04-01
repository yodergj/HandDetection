#include <qapplication.h>
#include <qimage.h>
#include <avifile.h>
#include <avm_fourcc.h>

#define FRAME_RATE 10

void InsertFrame(QImage* image, avm::IVideoWriteStream* outVidStr)
{
  int x, y;
  int width, height;
  unsigned char* data;
  unsigned char* outputLine;
  QRgb* scanline;

  width = image->width();
  height = image->height();
  printf("width %d height %d\n", width, height);
  data = (unsigned char*)malloc(3* width * height);

  for (y = 0; y < height; y++)
  {
    scanline = (QRgb*)image->scanLine(y);
    outputLine = data + y * width * 3;
    for (x = 0; x < width; x++)
    {
      outputLine[3 * x] = qBlue(scanline[x]);
      outputLine[3 * x + 1] = qGreen(scanline[x]);
      outputLine[3 * x + 2] = qRed(scanline[x]);
    }
  }

  avm::CImage frame(data, width, height);
  outVidStr->AddFrame(&frame);

  free(data);
}

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  int i;
  int width = 0;
  int height = 0;
  avm::IWriteFile* outFile = avm::CreateWriteFile("out.avi");
  BITMAPINFOHEADER bi;
  //fourcc_t codec = fccMP42;
  fourcc_t codec = fccDIV3;
  //fourcc_t codec = fccIV32;
  //fourcc_t codec = fccCVID;
  avm::IVideoWriteStream* vidStr;

  if ( argc < 2 )
    return 0;

  for (i = 1; i < argc; i++)
  {
    if ( !inputImage.load(argv[i]) )
    {
      fprintf(stderr, "Error loading %s\n", argv[i]);
      return 1;
    }

    if ( i == 1 )
    {
      width = inputImage.width();
      height = inputImage.height();

      memset(&bi, 0, sizeof(BITMAPINFOHEADER));
      bi.biSize = sizeof(BITMAPINFOHEADER);
      bi.biWidth = width;
      bi.biHeight = height;
      bi.biSizeImage = width * height * 3;
      bi.biPlanes = 1;
      bi.biBitCount = 24;

      vidStr = outFile->AddVideoStream(codec, &bi, 1000000 / FRAME_RATE);
      vidStr->Start();
    }

    InsertFrame(&inputImage, vidStr);
  }
  vidStr->Stop();

  return 0;
}
