#include <QFileDialog>
#include <QImageWriter>
#include "handymouse.h"
#include "HandyTracker.h"
#include "ColorRegion.h"
#include "AdaboostClassifier.h"

HandyMouse::HandyMouse(QWidget *parent)
	: QMainWindow(parent)
{
  mFrameNumber = 0;
  mPixmapItem = 0;
  mEllipseItem = 0;
  mRectItem = 0;
	ui.setupUi(this);
  mScene = new QGraphicsScene(this);
  ui.imageView->setScene(mScene);

  QBrush brush(QColor(0,255,0), Qt::DiagCrossPattern);
  mScene->setBackgroundBrush(brush);

  mTracker = new HandyTracker;
  mVideoDecoder = 0;
}

HandyMouse::~HandyMouse()
{
  delete mTracker;
  delete mVideoDecoder;
}

void HandyMouse::BuildImageFilter()
{
  QString filter;
  QList<QByteArray> formats = QImageWriter::supportedImageFormats();
  foreach (QString format, formats)
  {
    filter += QString("%1 files (*.%2);;").arg(format.toUpper()).arg(format);
  }
  if (filter.endsWith(";;"))
    filter.chop(2);

  mImageFilter = filter;
}

void HandyMouse::on_actionExport_Frame_triggered()
{
  if ( mPixmaps.empty() )
    return;

  if ( mImageFilter.isEmpty() )
    BuildImageFilter();

  QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), "", mImageFilter);

  if ( filename.isEmpty() )
    return;

  mPixmaps[mFrameNumber].toImage().save(filename);
}

void HandyMouse::on_actionExport_Hand_triggered()
{
  if ( mPixmaps.empty() )
    return;

  ColorRegion* region = mTracker->GetRegion(mFrameNumber);
  if ( !region )
    return;

  if ( mImageFilter.isEmpty() )
    BuildImageFilter();

  QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), "", mImageFilter);

  if ( filename.isEmpty() )
    return;

  QImage srcImage = mPixmaps[mFrameNumber].toImage();
  QImage outImage(region->GetWidth(), region->GetHeight(), srcImage.format());
  int x, y;
  int minX = region->GetMinX();
  int maxX = region->GetMaxX();
  int minY = region->GetMinY();
  int maxY = region->GetMaxY();

  for (y = minY; y <= maxY; y++)
  {
    for (x = minX; x <= maxX; x++)
    {
      QRgb color;
      if ( region->ContainsPixel(x, y) )
        color = srcImage.pixel(x, y);
      else
        color = qRgb(0, 0, 0);
      outImage.setPixel(x - minX, y - minY, color);
    }
  }
  outImage.save(filename);
}

void HandyMouse::on_actionLoad_triggered()
{
  QString filename = QFileDialog::getOpenFileName(this,
    tr("Open Video"),
    "",
    tr("Video Files (*.asf *.wma *.wmv *.divx *.f4v *.flv *.mkv *.mk3d *.mka *.mks *.mcf *.mp4 *.mpg *.mpeg *.ogg *.ogv *.mov *.qt *.webm)"));

  if ( filename.isEmpty() )
    return;

  if ( mVideoDecoder )
    delete mVideoDecoder;
  mVideoDecoder = new VideoDecoder;

  if ( mPixmapItem )
    mPixmapItem->setPixmap( QPixmap() );
  if ( !mPixmaps.empty() )
    mPixmaps.clear();

  if ( mTracker )
    mTracker->ResetHistory();

  std::string filenameStr = filename.toStdString(); 
  mVideoDecoder->SetFilename(filenameStr);
  if ( !mVideoDecoder->Load() )
  {
    fprintf(stderr, "Failed loading video <%s>\n", filenameStr.c_str());
    return;
  }

  if ( !mVideoDecoder->UpdateFrame() )
  {
    fprintf(stderr, "Failed updating frame\n");
    return;
  }

  Image* img = mVideoDecoder->GetFrame();
  // img will only be null if no frames are in the video
  if ( !img )
    return;

  mFrameNumber = 0;
  ProcessFrame(img);

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
  DisplayResults();
}

void HandyMouse::on_actionLoad_Open_Classifier_triggered()
{
  if ( !mTracker )
  {
    printf("Error: Tracker not initialized\n");
    return;
  }

  QString filename = QFileDialog::getOpenFileName(this,
     tr("Load Open Classifier"),
     "",
     tr("Classifier Files (*.cfg)"));

  if ( filename.isEmpty() )
    return;

  std::string filenameStr = filename.toStdString(); 

  AdaboostClassifier* classifier = new AdaboostClassifier;
  if ( !classifier->Load(filenameStr.c_str()) )
  {
    printf("Failed loading open classifier\n");
    delete classifier;
    return;
  }

  if ( !mTracker->SetOpenClassifier(classifier) )
  {
    printf("Failed setting open classifier\n");
    delete classifier;
  }
}

void HandyMouse::on_actionLoad_Closed_Classifier_triggered()
{
  if ( !mTracker )
  {
    printf("Error: Tracker not initialized\n");
    return;
  }

  QString filename = QFileDialog::getOpenFileName(this,
     tr("Load Closed Classifier"),
     "",
     tr("Classifier Files (*.cfg)"));

  if ( filename.isEmpty() )
    return;

  std::string filenameStr = filename.toStdString(); 

  AdaboostClassifier* classifier = new AdaboostClassifier;
  if ( !classifier->Load(filenameStr.c_str()) )
  {
    printf("Failed loading closed classifier\n");
    delete classifier;
    return;
  }

  if ( !mTracker->SetClosedClassifier(classifier) )
  {
    printf("Failed setting closed classifier\n");
    delete classifier;
  }
}

void HandyMouse::on_prevButton_clicked()
{
  if ( mFrameNumber == 0 )
    return;

  mFrameNumber--;
  mPixmapItem->setPixmap(mPixmaps[mFrameNumber]);
  DisplayResults();
}

void HandyMouse::on_nextButton_clicked()
{
  if ( mPixmaps.empty() || !mVideoDecoder )
    return;

  mFrameNumber++;
  if ( mFrameNumber == mPixmaps.size() )
  {
    mVideoDecoder->UpdateFrame();
    Image* img = mVideoDecoder->GetFrame();
    // img will be null when we hit the end of the stream
    if ( !img )
      return;

    ProcessFrame(img);

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

  mPixmapItem->setPixmap(mPixmaps[mFrameNumber]);
  DisplayResults();
}

void HandyMouse::ProcessFrame(Image* img)
{
  if ( !img )
    return;

  // TODO Add the initialization stage
  ColorRegion* region = 0;
  if ( mFrameNumber == 0 )
  {
    Point imgCenter(img->GetWidth() / 2, img->GetHeight() / 2);
    region = new ColorRegion;
    region->Grow(*img, imgCenter);
  }
  else
  {
    ColorRegion* oldRegion = mTracker->GetRegion(mFrameNumber - 1);
    if ( !oldRegion )
    {
      printf("Failed getting old region\n");
      return;
    }
    region = new ColorRegion;
    region->TrackFromOldRegion(*img, *oldRegion);
  }

  if ( !mTracker->AnalyzeRegion(region) )
  {
    printf("Failed analyzing region");
    delete region;
    return;
  }
}

void HandyMouse::DisplayResults()
{
  HandyTracker::HandState handClass = mTracker->GetState(mFrameNumber);
  ColorRegion* region = mTracker->GetRegion(mFrameNumber);
  Matrix* featureData = mTracker->GetFeatureData(mFrameNumber);

  printf("Class: ");
  if ( handClass == HandyTracker::ST_OPEN )
    printf("Open\n");
  else if ( handClass == HandyTracker::ST_CLOSED )
    printf("Closed\n");
  else if ( handClass == HandyTracker::ST_CONFLICT )
    printf("Conflicted\n");
  else
    printf("Unknown\n");

  if ( featureData )
  {
    int numFeatures = featureData->GetRows();
    for (int i = 0; i < numFeatures; i++)
    {
      if ( i > 0 )
      {
        if ( i % 4 )
          printf("\t");
        else
          printf("\n");
      }
      printf("%lf", featureData->GetValue(i, 0));
    }
    printf("\n");
  }

  if ( region )
  {
    Point centroid = region->GetCentroid();
    qreal ellipseX = centroid.x;
    qreal ellipseY = centroid.y;
    if ( mEllipseItem )
      mEllipseItem->setPos(ellipseX, ellipseY);
    else
      mEllipseItem = mScene->addEllipse(ellipseX, ellipseY, 1.0, 1.0, QPen( QColor(0,0,0) ));

    QRectF regionRect(region->GetMinX(), region->GetMinY(), region->GetWidth(), region->GetHeight());
    if ( mRectItem )
      mRectItem->setRect(regionRect);
    else
      mRectItem = mScene->addRect(regionRect);

    if ( handClass == HandyTracker::ST_OPEN )
      mRectItem->setPen( QPen( QColor(0, 255, 0) ) );
    else if ( handClass == HandyTracker::ST_CLOSED )
      mRectItem->setPen( QPen( QColor(0, 0, 255) ) );
    else if ( handClass == HandyTracker::ST_CONFLICT )
      mRectItem->setPen( QPen( QColor(255, 0, 0) ) );
    else
      mRectItem->setPen( QPen( QColor(255, 255, 255) ) );
  }

  // TODO Finish results display
}