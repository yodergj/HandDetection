#include <QFileDialog>
#include "handymouse.h"

HandyMouse::HandyMouse(QWidget *parent)
	: QMainWindow(parent)
{
  mFrameNumber = 0;
  mPixmapItem = 0;
	ui.setupUi(this);
  mScene = new QGraphicsScene(this);
  ui.imageView->setScene(mScene);

  QBrush brush(QColor(0,255,0), Qt::DiagCrossPattern);
  mScene->setBackgroundBrush(brush);
}

HandyMouse::~HandyMouse()
{

}

void HandyMouse::on_actionLoad_triggered()
{
  QString filename = QFileDialog::getOpenFileName(this,
     tr("Open Video"),
     "",
     tr("Video Files (*.asf *.wma *.wmv *.divx *.f4v *.flv *.mkv *.mk3d *.mka *.mks *.mcf *.mp4 *.mpg *.mpeg *.ogg *.ogv *.mov *.qt *.webm)"));

  if ( filename.isEmpty() )
    return;

  if ( mPixmapItem )
    mPixmapItem->setPixmap( QPixmap() );
  if ( !mPixmaps.empty() )
    mPixmaps.clear();

  std::string filenameStr = filename.toStdString(); 
  mVideoDecoder.SetFilename(filenameStr);
  if ( !mVideoDecoder.Load() )
  {
    printf("Crap happens\n");
    return;
  }

  if ( !mVideoDecoder.UpdateFrame() )
  {
    printf("This sucks\n");
    return;
  }

  Image* img = mVideoDecoder.GetFrame();
  // img will only be null if no frames are in the video
  if ( !img )
    return;
  QImage qimg(img->GetRGBBuffer(), img->GetWidth(), img->GetHeight(), img->GetWidth() * 3, QImage::Format_RGB888);

  if ( qimg.isNull() )
  {
    printf("More crap happens\n");
    return;
  }

  QPixmap pixmap;
  if ( !pixmap.convertFromImage(qimg) )
  {
    printf("Yet more crap happens\n");
    return;
  }

  mPixmaps.push_back(pixmap);
  if ( mPixmapItem )
    mPixmapItem->setPixmap(mPixmaps[0]);
  else
  {
    mPixmapItem = mScene->addPixmap(mPixmaps[0]);
    mPixmapItem->setPos(0.0, 0.0);
  }
  mFrameNumber = 0;
}

void HandyMouse::on_prevButton_clicked()
{
  if ( mFrameNumber == 0 )
    return;

  mFrameNumber--;
  mPixmapItem->setPixmap(mPixmaps[mFrameNumber]);
}

void HandyMouse::on_nextButton_clicked()
{
  if ( mPixmaps.empty() )
    return;

  if ( mFrameNumber == mPixmaps.size() - 1 )
  {
    mVideoDecoder.UpdateFrame();
    Image* img = mVideoDecoder.GetFrame();
    // img will be null when we hit the end of the stream
    if ( !img )
      return;
    QImage qimg(img->GetRGBBuffer(), img->GetWidth(), img->GetHeight(), img->GetWidth() * 3, QImage::Format_RGB888);

    if ( qimg.isNull() )
    {
      printf("More crap happens\n");
      return;
    }

    QPixmap pixmap;
    if ( !pixmap.convertFromImage(qimg) )
    {
      printf("Yet more crap happens\n");
      return;
    }

    mPixmaps.push_back(pixmap);
  }

  mFrameNumber++;
  mPixmapItem->setPixmap(mPixmaps[mFrameNumber]);
}