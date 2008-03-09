/* Gabe Yoder
   CS660
   Course Project
*/
#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <qwidget.h>
#include <qimage.h>
#include <qtimer.h>
extern "C" {
#include <ffmpeg/avformat.h>
}

class VideoWidget : public QWidget
{
Q_OBJECT
public:
  VideoWidget(QWidget *parent=0, const char *name=0, WFlags f=0);
  ~VideoWidget();
  bool SetStartFrame(int frame);
  void SetFilename(QString filename);
  bool Load();
  QImage* GetFrame();
public slots:
  void Play();
  void Stop();
  void Reset();
  void UpdateFrame();
protected:
  void paintEvent(QPaintEvent *event);
private:
  bool GetNextFrame();
  QTimer* mTimer;
  AVFormatContext *mFormatContext;
  AVCodecContext *mCodecContext;
  AVCodec *mCodec;
  AVFrame *mFrame;
  AVFrame *mFrameRGB;
  AVPacket mPacket;
  int mVideoStream;
  uint8_t *mBuffer;
  QImage mImage;
  int mStartFrame;
  QString mFilename;
};

#endif
