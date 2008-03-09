/* Gabe Yoder
   CS660
   Course Project
*/

#include "VideoWidget.h"
#include <qapplication.h>
#include <qpainter.h>

/* Delay times for desired frame rates */
#define FPS10 100
#define FPS15 67
#define FPS30 33
#define FPS60 17

static bool AVRegistered = false;

VideoWidget::VideoWidget(QWidget *parent, const char *name, WFlags f) : QWidget(parent,name,f)
{
  mTimer = new QTimer(this);
  connect(mTimer,SIGNAL(timeout()),this,SLOT(UpdateFrame()));

  mFormatContext = NULL;
  mCodecContext = NULL;
  mCodec = NULL;
  mFrame = NULL;
  mFrameRGB = NULL;
  mPacket.data = NULL;
  mVideoStream = -1;
  mBuffer = NULL;
  mStartFrame = 0;
  mFilename = "";

  if ( !AVRegistered )
  {
    av_register_all();
    AVRegistered = true;
  }
}

VideoWidget::~VideoWidget()
{
  if ( mFrame )
    av_free(mFrame);
  if ( mFrameRGB )
    av_free(mFrameRGB);
  if ( mCodecContext )
    avcodec_close(mCodecContext);
  if ( mFormatContext )
    av_close_input_file(mFormatContext);
  if ( mBuffer )
    free(mBuffer);
}

QImage* VideoWidget::GetFrame()
{
  return &mImage;
}

bool VideoWidget::SetStartFrame(int frame)
{
  if ( frame < 0 )
    return false;

  mStartFrame = frame;
  return true;
}

void VideoWidget::SetFilename(QString filename)
{
  mFilename = filename;
}

void VideoWidget::Play()
{
  Reset();
  mTimer->start(FPS15);
}

void VideoWidget::Stop()
{
  mTimer->stop();
}

void VideoWidget::Reset()
{
  int i;

  if ( mFilename != "" )
  {
    Load();
    for (i=0; i <= mStartFrame; i++)
      UpdateFrame();
  }
  repaint();
}

bool VideoWidget::GetNextFrame()
{
  int frameFinished;

  while ( av_read_frame(mFormatContext, &mPacket) >= 0 )
  {
    if ( mPacket.stream_index == mVideoStream )
    {
      avcodec_decode_video(mCodecContext, mFrame, &frameFinished, mPacket.data, mPacket.size);

      if ( frameFinished )
      {
        img_convert((AVPicture *)mFrameRGB, PIX_FMT_RGB24, (AVPicture *)mFrame, mCodecContext->pix_fmt, mCodecContext->width, mCodecContext->height);

        return true;
      }
    }
    av_free_packet(&mPacket);
  }

  return false;
}

void VideoWidget::UpdateFrame()
{
  int *pixel;
  int srcWidth, srcHeight;
  int x,y;
  uint8_t *srcData = NULL;

  if ( !GetNextFrame() )
  {
    Stop();
    return;
  }

  if ( !mImage.isNull() )
  {
    srcWidth = mCodecContext->width;
    srcHeight = mCodecContext->height;
    pixel = (int *)mImage.bits();

    for (y=0; y<srcHeight; y++)
    {
      srcData = mFrameRGB->data[0] + y * mFrameRGB->linesize[0];
      for (x=0; x<srcWidth; x++)
      {
        *pixel = srcData[2] | (srcData[1] << 8) | (srcData[0] << 16);
        pixel++;
        srcData += 3;
      }
    }
  }

  repaint(0,0,width(),height(),false);
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.drawImage(0,0,mImage);
}

bool VideoWidget::Load()
{
  unsigned int i;
  int numBytes;
  uint8_t *tmp;

  if ( av_open_input_file(&mFormatContext, mFilename.ascii(), NULL, 0, NULL) != 0 )
    return false;

  if ( av_find_stream_info(mFormatContext) < 0 )
    return false;

  /* Some debug info */
  dump_format(mFormatContext, 0, mFilename.ascii(), false);

  for (i=0; i < mFormatContext->nb_streams; i++)
  {
    if ( mFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO )
    {
      mVideoStream = i;
      break;
    }
  }

  if ( mVideoStream == -1 )
    return false;

  mCodecContext = mFormatContext->streams[mVideoStream]->codec;
  mCodec = avcodec_find_decoder(mCodecContext->codec_id);
  if ( !mCodec )
    return false;

  if ( avcodec_open(mCodecContext, mCodec) < 0 )
    return false;

#if 0
  /* Correct wrong frame rate returned by some codecs */
  if ( (mCodecContext->frame_rate > 1000) && (mCodecContext->frame_rate_base == 1) )
    mCodecContext->frame_rate_base = 1000;
  /* frame_rate isn't a field anymore.  Use 1/time_base (of type AVRational) */
#endif

  mFrame = avcodec_alloc_frame();
  if ( !mFrame )
    return false;

  mFrameRGB = avcodec_alloc_frame();
  if ( !mFrameRGB )
    return false;

  /* Determine required buffer size and allocate buffer */
  numBytes = avpicture_get_size(PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height);
  tmp = (uint8_t *)realloc(mBuffer,numBytes*sizeof(uint8_t));
  if ( !tmp )
    return false;
  mBuffer = tmp;

  avpicture_fill((AVPicture *)mFrameRGB, mBuffer, PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height);

  /* Have to use 32-bit depth since 24-bit isn't supported anymore */
  if ( !mImage.create(mCodecContext->width, mCodecContext->height, 32) )
    return false;

  return true;
}

#if 0
int main(int argc, char *argv[])
{
  QApplication app(argc,argv);
  VideoWidget gui;
  if ( !gui.Load("/home/gabe/hand_video/20070323_141051/2.avi") )
    fprintf(stderr,"Error loading avi file\n");
  gui.show();
  app.setMainWidget(&gui);
  gui.Play();
  return app.exec();
}
#endif
