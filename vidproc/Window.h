#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QImage>

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
    QTimer *mTimer;
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
