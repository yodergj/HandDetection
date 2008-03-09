#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include "Window.h"

/* Delay times for desired frame rates */
#define FPS10 100
#define FPS15 67
#define FPS30 33
#define FPS60 17

#define DEBUG_UPDATE 1

#define HAND_HEIGHT_ESTIMATE 300

Window::Window() : QMainWindow()
{
  QMenuBar *menubar = new QMenuBar(this);
  QMenu *filemenu = new QMenu();
  QMenu *runmenu = new QMenu();
  mTimer = new QTimer(this);
  connect(mTimer,SIGNAL(timeout()),this,SLOT(UpdateVideos()));

  menubar->insertItem("&File",filemenu);
  menubar->insertItem("&Run",runmenu);

  filemenu->insertItem("Load Video",this,SLOT(selectVideo()));
  filemenu->insertItem("Quit",qApp,SLOT(quit()));

  runmenu->insertItem("Start",this,SLOT(Play()));
  runmenu->insertItem("Stop",this,SLOT(Stop()));
  runmenu->insertItem("Reset",this,SLOT(Reset()));

  mHandXDelta = 0;
  mHandYDelta = 0;
  mHandZDelta = 0;

  setCentralWidget(vbox);
}

Window::~Window()
{
}

void Window::Play()
{
  Reset();
  mTimer->start(FPS15);
}

void Window::Stop()
{
  mTimer->stop();
}

void Window::Reset()
{
}

void Window::UpdateVideos()
{
  ProcessImages();
}

void Window::selectVideo()
{
  QString frontFilename = QFileDialog::getOpenFileName("","Videos (*.avi)",this,"open file dialog","Select front video" );
  QString sideFilename = QFileDialog::getOpenFileName("","Videos (*.avi)",this,"open file dialog","Select side video" );

  if ( !frontFilename.isNull() && !sideFilename.isNull() )
  {
  }
}

void Window::ProcessImages()
{
  QImage *frontImage;
  QImage *sideImage;
  QImage frontIQImage;
  QImage sideIQImage;
  QImage differenceImage;
  int i;
  int area;
  int numClusters;
  int width, height;
  int biggestCluster, biggestClusterArea;
  QPtrList<QRect> blocklist;
  QPoint frontPos;
  QPoint sidePos;
  int frontXDelta = 0;
  int frontYDelta = 0;
  int sideXDelta = 0;
  int sideYDelta = 0;

  blocklist.setAutoDelete(true);

  width = frontImage->width();
  height = frontImage->height();
  differenceImage.create(width,height,32);

  if ( !mLastFrontIQFrame.isNull() )
  {
#if 0
    SubtractImage(&frontIQImage, &mLastFrontIQFrame, &differenceImage);
    numClusters = GetDifferenceBlocks(&differenceImage, &blocklist);
#endif
    if ( numClusters > 0 )
    {
      biggestCluster = 0;
      biggestClusterArea = blocklist.at(0)->width() * blocklist.at(0)->height();
      for (i=1; i<numClusters; i++)
      {
        area = blocklist.at(i)->width() * blocklist.at(i)->height();
        if ( area > biggestClusterArea )
        {
          biggestClusterArea = area;
          biggestCluster = i;
        }
      }
      frontPos = blocklist.at(biggestCluster)->center();
      if ( !mLastFrontPos.isNull() )
      {
        frontXDelta = frontPos.x() - mLastFrontPos.x();
        frontYDelta = frontPos.y() - mLastFrontPos.y();
      }
      mLastFrontPos = frontPos;
    }

    blocklist.clear();

#if 0
    SubtractImage(&sideIQImage, &mLastSideIQFrame, &differenceImage);
    numClusters = GetDifferenceBlocks(&differenceImage, &blocklist);
#endif
    if ( numClusters > 0 )
    {
      biggestCluster = 0;
      biggestClusterArea = blocklist.at(0)->width() * blocklist.at(0)->height();
      for (i=1; i<numClusters; i++)
      {
        area = blocklist.at(i)->width() * blocklist.at(i)->height();
        if ( area > biggestClusterArea )
        {
          biggestClusterArea = area;
          biggestCluster = i;
        }
      }
      sidePos = blocklist.at(biggestCluster)->center();
      if ( !mLastSidePos.isNull() )
      {
        sideXDelta = sidePos.x() - mLastSidePos.x();
        sideYDelta = sidePos.y() - mLastSidePos.y();
      }
      mLastSidePos = sidePos;
    }
  }
  mHandXDelta = frontXDelta * mScaleFactor;
  /* Y Delta really should be based on both front and side,
     but for now we just use front since it will hopefully be more
     reliable than the side */
  mHandYDelta = frontYDelta * mScaleFactor;
  mHandZDelta = sideXDelta * mScaleFactor;

  mLastFrontIQFrame = frontIQImage.copy();
  mLastSideIQFrame = sideIQImage.copy();
}

int main(int argc, char *argv[])
{
  QApplication app(argc,argv);
  Window gui;
  gui.show();
  app.setMainWidget(&gui);
  return app.exec();
}
