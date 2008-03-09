/* Gabe Yoder
   CS660
   Course Project
*/

#ifndef WINDOW_H
#define WINDOW_H

#include <qmainwindow.h>
#include <qtimer.h>
#include <qimage.h>
#include "HandWidget.h"
#include "VideoWidget.h"

class Window : public QMainWindow
{
  Q_OBJECT

  public:
    Window();
    ~Window();
  public slots:
    void selectVideo();
    void Play();
    void Stop();
    void Reset();
    void UpdateVideos();
  private:
    void ProcessImages();
    void UpdateHandModel();
    QTimer *mTimer;
    HandWidget *mModelView;
    VideoWidget *mVideoView;
    HandWidget *mSideModelView;
    VideoWidget *mSideVideoView;
    Hand *mHandData;
    WRLModel *mModelData;
    QImage mLastFrontIQFrame;
    QImage mLastSideIQFrame;
    QPoint mLastFrontPos;
    QPoint mLastSidePos;
    double mHandXDelta;
    double mHandYDelta;
    double mHandZDelta;
    double mScaleFactor;
};

#endif
