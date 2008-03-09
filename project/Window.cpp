/* Gabe Yoder
   CS660
   Course Project
*/

#include <qapp.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qfiledialog.h>
#include "Window.h"
#include "ImageRoutines.h"

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
  QPopupMenu *filemenu = new QPopupMenu();
  QPopupMenu *runmenu = new QPopupMenu();
  QVBox* vbox = new QVBox(this);
  QHBox* topHbox = new QHBox(vbox);
  QHBox* bottomHbox = new QHBox(vbox);
  mVideoView = new VideoWidget(topHbox);
  mModelView = new HandWidget(topHbox);
  mSideVideoView = new VideoWidget(bottomHbox);
  mSideModelView = new HandWidget(bottomHbox);
  mHandData = new Hand();
  mModelData = new WRLModel();
  mTimer = new QTimer(this);
  connect(mTimer,SIGNAL(timeout()),this,SLOT(UpdateVideos()));

  menubar->insertItem("&File",filemenu);
  menubar->insertItem("&Run",runmenu);

  filemenu->insertItem("Load Video",this,SLOT(selectVideo()));
  filemenu->insertItem("Quit",qApp,SLOT(quit()));

  runmenu->insertItem("Start",this,SLOT(Play()));
  runmenu->insertItem("Stop",this,SLOT(Stop()));
  runmenu->insertItem("Reset",this,SLOT(Reset()));

  mSideModelView->SetView(VIEW_SIDE);
  if ( mModelData->Load("leftHand_bones.WRL") )
  {
    mModelView->SetHand(mModelData);
    mSideModelView->SetHand(mModelData);
    mScaleFactor = mModelData->ySpan / HAND_HEIGHT_ESTIMATE;
  }
  else
  {
    mModelView->SetHand(mHandData);
    mSideModelView->SetHand(mHandData);
    mScaleFactor = mHandData->ySpan / HAND_HEIGHT_ESTIMATE;
  }

  mHandXDelta = 0;
  mHandYDelta = 0;
  mHandZDelta = 0;

  mVideoView->SetFilename("/home/gabe/hand_video/20070503_211137/3.avi");
  mVideoView->Load();
  mVideoView->Reset();
  mSideVideoView->SetFilename("/home/gabe/hand_video/20070503_211137/2.avi");
  mSideVideoView->Load();
  mSideVideoView->Reset();
  mSideVideoView->SetStartFrame(15);

  setCentralWidget(vbox);
}

Window::~Window()
{
  delete mHandData;
}

void Window::Play()
{
#if 0
  mVideoView->Play();
  mSideVideoView->Play();
#else
  Reset();
  mTimer->start(FPS15);
#endif
}

void Window::Stop()
{
#if 0
  mVideoView->Stop();
  mSideVideoView->Stop();
#else
  mTimer->stop();
#endif
}

void Window::Reset()
{
  mVideoView->Reset();
  mSideVideoView->Reset();
}

void Window::UpdateVideos()
{
  mVideoView->UpdateFrame();
  mSideVideoView->UpdateFrame();
  ProcessImages();
  UpdateHandModel();
}

void Window::selectVideo()
{
  QString frontFilename = QFileDialog::getOpenFileName("","Videos (*.avi)",this,"open file dialog","Select front video" );
  QString sideFilename = QFileDialog::getOpenFileName("","Videos (*.avi)",this,"open file dialog","Select side video" );

  if ( !frontFilename.isNull() && !sideFilename.isNull() )
  {
    mVideoView->SetFilename(frontFilename);
    mVideoView->Load();
    mVideoView->Reset();
    mSideVideoView->SetFilename(sideFilename);
    mSideVideoView->Load();
    mSideVideoView->Reset();
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
#if DEBUG_UPDATE
  static int updateNum = 1;
  printf("Update number %d\n",updateNum);
  updateNum++;
#endif

  blocklist.setAutoDelete(true);
  frontImage = mVideoView->GetFrame();
  sideImage = mSideVideoView->GetFrame();
  frontIQImage = GetIQImage(frontImage);
  sideIQImage = GetIQImage(sideImage);

  width = frontImage->width();
  height = frontImage->height();
  differenceImage.create(width,height,32);

  if ( !mLastFrontIQFrame.isNull() )
  {
    SubtractImage(&frontIQImage, &mLastFrontIQFrame, &differenceImage);
    numClusters = GetDifferenceBlocks(&differenceImage, &blocklist);
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
#if DEBUG_UPDATE
      printf("front (%d, %d) %d %d %d %d (%d, %d)\n",
          frontPos.x(), frontPos.y(),
          blocklist.at(biggestCluster)->left(),
          blocklist.at(biggestCluster)->right(),
          blocklist.at(biggestCluster)->top(),
          blocklist.at(biggestCluster)->bottom(),
          frontXDelta, frontYDelta);
#endif
      mLastFrontPos = frontPos;
    }

    blocklist.clear();

    SubtractImage(&sideIQImage, &mLastSideIQFrame, &differenceImage);
    numClusters = GetDifferenceBlocks(&differenceImage, &blocklist);
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
#if DEBUG_UPDATE
      printf("side (%d, %d) %d %d %d %d (%d, %d)\n",
          sidePos.x(), sidePos.y(),
          blocklist.at(biggestCluster)->left(),
          blocklist.at(biggestCluster)->right(),
          blocklist.at(biggestCluster)->top(),
          blocklist.at(biggestCluster)->bottom(),
          sideXDelta, sideYDelta);
#endif
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

void Window::UpdateHandModel()
{
  int i;

  /* Only work with the WRL model for now */
  if ( !mModelData )
    return;

  if ( mHandXDelta || mHandYDelta || mHandZDelta )
  {
    for (i=0; i<mModelData->numPoints; i++)
    {
      mModelData->points[i].x += mHandXDelta;
      mModelData->points[i].y += mHandYDelta;
      mModelData->points[i].z += mHandZDelta;
    }

    mModelView->repaint();
    mSideModelView->repaint();
  }
}


int main(int argc, char *argv[])
{
  QApplication app(argc,argv);
  Window gui;
  gui.show();
  app.setMainWidget(&gui);
  return app.exec();
}
