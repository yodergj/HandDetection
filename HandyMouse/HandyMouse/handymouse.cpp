#include <QFileDialog>
#include <QImageWriter>
#include "handymouse.h"
#include "HandyTracker.h"
#include "ColorRegion.h"
#include "AdaboostClassifier.h"

const int HandyMouse::mMaxPixmapBacklog = 10;

HandyMouse::HandyMouse(QWidget *parent)
	: QMainWindow(parent)
{
  mTrackingInitialized = false;
  mFrameNumber = 0;
  mPixmapItem = 0;
  mEllipseItem = 0;
  mRectItem = 0;
  mXItem = 0;
  mWristLineItem = 0;
  mWristLineItem2 = 0;
	ui.setupUi(this);
  mScene = new QGraphicsScene(this);
  ui.imageView->setScene(mScene);

  QBrush brush(QColor(0,255,0), Qt::DiagCrossPattern);
  mScene->setBackgroundBrush(brush);

  mTracker = new HandyTracker;
  mVideoDecoder = 0;
  mStreamFullyProcessed = false;
}

HandyMouse::~HandyMouse()
{
  delete mTracker;
  delete mVideoDecoder;
  ClearPixmaps();
}

void HandyMouse::ClearPixmaps()
{
  size_t numPixmaps = mPixmaps.size();
  for (size_t i = 0; i < numPixmaps; i++)
    delete mPixmaps[i];
  mPixmaps.clear();
}

void HandyMouse::BuildImageFilter()
{
  QString filter = "PNG files (*.png);;";
  QList<QByteArray> formats = QImageWriter::supportedImageFormats();
  foreach (QString format, formats)
  {
    if ( format.toUpper() != QString("PNG") )
      filter += QString("%1 files (*.%2);;").arg(format.toUpper()).arg(format);
  }
  if (filter.endsWith(";;"))
    filter.chop(2);

  mImageFilter = filter;
}

void HandyMouse::on_actionExport_Frame_triggered()
{
  if ( mPixmaps.empty() || !mPixmaps[mFrameNumber] )
    return;

  if ( mImageFilter.isEmpty() )
    BuildImageFilter();

  QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), "", mImageFilter);

  if ( filename.isEmpty() )
    return;

  mPixmaps[mFrameNumber]->toImage().save(filename);
}

void HandyMouse::on_actionExport_Hand_triggered()
{
  if ( mPixmaps.empty() || !mPixmaps[mFrameNumber] )
    return;

  ColorRegion* region = mTracker->GetRegion(mFrameNumber);
  if ( !region )
    return;

  if ( mImageFilter.isEmpty() )
    BuildImageFilter();

  QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), "", mImageFilter);

  if ( filename.isEmpty() )
    return;

  QImage srcImage = mPixmaps[mFrameNumber]->toImage();
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
    tr("Video Files (*.asf *.wma *.wmv *.divx *.f4v *.flv *.mkv *.mk3d *.mka *.mks *.mcf *.mp4 *.mpg *.mpeg *.mts *.ogg *.ogv *.mov *.qt *.webm)"));

  if ( filename.isEmpty() )
    return;

  if ( mVideoDecoder )
    delete mVideoDecoder;
  mVideoDecoder = new VideoDecoder;
  mStreamFullyProcessed = false;

  if ( mPixmapItem )
    mPixmapItem->setPixmap( QPixmap() );
  ClearPixmaps();

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

  mTrackingInitialized = false;
  mFrameNumber = 0;
  ProcessFrame(img);

  QImage qimg(img->GetRGBBuffer(), img->GetWidth(), img->GetHeight(), img->GetWidth() * 3, QImage::Format_RGB888);

  if ( qimg.isNull() )
  {
    fprintf(stderr, "QImage is null\n");
    return;
  }

  QPixmap* pixmap = new QPixmap;
  if ( !pixmap->convertFromImage(qimg, Qt::NoFormatConversion) )
  {
    fprintf(stderr, "Failed getting pixmap from image\n");
    delete pixmap;
    return;
  }

  mPixmaps.push_back(pixmap);
  if ( mPixmapItem )
    mPixmapItem->setPixmap(*mPixmaps[0]);
  else
  {
    mPixmapItem = mScene->addPixmap(*mPixmaps[0]);
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
  if ( mPixmaps[mFrameNumber] )
    mPixmapItem->setPixmap(*mPixmaps[mFrameNumber]);
  else
  {
    // TODO
  }
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
    {
      mStreamFullyProcessed = true;
      mFrameNumber--;
      return;
    }

    ProcessFrame(img);

    QImage qimg(img->GetRGBBuffer(), img->GetWidth(), img->GetHeight(), img->GetWidth() * 3, QImage::Format_RGB888);

    if ( qimg.isNull() )
    {
      fprintf(stderr, "QImage is null\n");
      mStreamFullyProcessed = true;
      mFrameNumber--;
      return;
    }

    QPixmap* pixmap = new QPixmap;
    if ( !pixmap->convertFromImage(qimg, Qt::NoFormatConversion) )
    {
      fprintf(stderr, "Failed getting pixmap from image\n");
      delete pixmap;
      mStreamFullyProcessed = true;
      mFrameNumber--;
      return;
    }

    mPixmaps.push_back(pixmap);
  }

  if ( mFrameNumber >= mMaxPixmapBacklog )
  {
    delete mPixmaps[mFrameNumber - mMaxPixmapBacklog];
    mPixmaps[mFrameNumber - mMaxPixmapBacklog] = 0;
    mTracker->PurgeRegion(mFrameNumber - mMaxPixmapBacklog);
  }

  if ( mPixmaps[mFrameNumber] )
    mPixmapItem->setPixmap(*mPixmaps[mFrameNumber]);

  DisplayResults();
}

void HandyMouse::on_actionTo_Region_Size_Jump_triggered()
{
  if ( mPixmaps.empty() || !mVideoDecoder )
    return;

  ColorRegion* prevRegion = 0;
  ColorRegion* region = mTracker->GetRegion(mFrameNumber);
  bool sizeJumpOccurred = false;
  while ( !sizeJumpOccurred )
  {
    prevRegion = region;
    on_nextButton_clicked();

    if ( mStreamFullyProcessed )
      break;

    region = mTracker->GetRegion(mFrameNumber);
    if ( ( abs( region->GetWidth() - prevRegion->GetWidth() ) > 20 ) ||
         ( abs( region->GetHeight() - prevRegion->GetHeight() ) > 20 ) )
      sizeJumpOccurred = true;
  }
}

void HandyMouse::ProcessFrame(Image* img)
{
  if ( !img )
    return;

  ColorRegion* region = 0;
  if ( mTrackingInitialized )
  {
    ColorRegion* oldRegion = mTracker->GetRegion(mFrameNumber - 1);
    if ( !oldRegion )
    {
      printf("Failed getting old region\n");
      return;
    }
    region = new ColorRegion;
    region->TrackFromOldRegion(*img, *oldRegion);

    if ( !mTracker->AnalyzeRegion(region) )
    {
      printf("Failed analyzing region");
      delete region;
      return;
    }
    region->FreeIntegralBuffer();
  }
  else
  {
    Point imgCenter(img->GetWidth() / 2, img->GetHeight() / 2);
    region = new ColorRegion;
    region->Grow(*img, imgCenter);

    if ( !mTracker->AnalyzeRegionForInitialization(region) )
    {
      printf("Failed analyzing region for initialization");
      delete region;
      return;
    }
    region->FreeIntegralBuffer();

    HandyTracker::HandState handClass = mTracker->GetState(mFrameNumber);
    if ( handClass == HandyTracker::ST_OPEN )
      mTrackingInitialized = true;
  }
}

void HandyMouse::DisplayResults()
{
  QString debugText;
  QString frameStr = QString("%1").arg(mFrameNumber);
  ui.frameNumLabel->setText(frameStr);

  HandyTracker::HandState handClass = mTracker->GetState(mFrameNumber);
  ColorRegion* region = mTracker->GetRegion(mFrameNumber);
  Matrix* openFeatureData = mTracker->GetOpenFeatureData(mFrameNumber);
  Matrix* closedFeatureData = mTracker->GetClosedFeatureData(mFrameNumber);

  debugText += "Class: ";
  if ( handClass == HandyTracker::ST_OPEN )
    debugText += "Open\n";
  else if ( handClass == HandyTracker::ST_CLOSED )
    debugText += "Closed\n";
  else if ( handClass == HandyTracker::ST_CONFLICT )
    debugText += "Conflicted\n";
  else if ( handClass == HandyTracker::ST_REJECT )
    debugText += "Rejected\n";
  else
    debugText += "Unknown\n";

  if ( openFeatureData )
  {
    debugText += "Open Data:\n";
    int numFeatures = openFeatureData->GetRows();
    for (int i = 0; i < numFeatures; i++)
    {
      if ( i > 0 )
      {
        if ( i % GRID_DIM_SIZE )
          debugText += "\t";
        else
          debugText += "\n";
      }
      debugText += QString("%1").arg(openFeatureData->GetValue(i, 0), -10);
    }
    debugText += "\n";
  }

  if ( closedFeatureData )
  {
    debugText += "Closed Data:\n";
    int numFeatures = closedFeatureData->GetRows();
    for (int i = 0; i < numFeatures; i++)
    {
      if ( i > 0 )
      {
        if ( i % GRID_DIM_SIZE )
          debugText += "\t";
        else
          debugText += "\n";
      }
      debugText += QString("%1").arg(closedFeatureData->GetValue(i, 0), -10);
    }
    debugText += "\n";
  }

  if ( region )
  {
    Point centroid = region->GetCentroid();
    qreal ellipseX = centroid.x;
    qreal ellipseY = centroid.y;
    qreal ellipseW = 5.0;
    qreal ellipseH = 5.0;
    if ( mEllipseItem )
      mEllipseItem->setRect(ellipseX, ellipseY, ellipseW, ellipseH);
    else
      mEllipseItem = mScene->addEllipse(ellipseX, ellipseY, ellipseW, ellipseH, QPen( QColor(0,0,0) ));

    QRectF regionRect(region->GetMinX(), region->GetMinY(), region->GetWidth(), region->GetHeight());
    if ( mRectItem )
      mRectItem->setRect(regionRect);
    else
      mRectItem = mScene->addRect(regionRect);

    if ( mTrackingInitialized )
    {
      delete mXItem;
      mXItem = 0;

      int refHeight = region->GetReferenceHeight();
      if ( refHeight == 0 )
      {
        delete mWristLineItem;
        mWristLineItem = 0;
        delete mWristLineItem2;
        mWristLineItem2 = 0;
      }
      else
      {
        if ( mWristLineItem )
          mWristLineItem->setLine(region->GetMinX(), region->GetMinY() + refHeight, region->GetMaxX(), region->GetMinY() + refHeight);
        else
          mWristLineItem = mScene->addLine(region->GetMinX(), region->GetMinY() + refHeight, region->GetMaxX(), region->GetMinY() + refHeight);

        if ( mWristLineItem2 )
          mWristLineItem2->setLine(region->GetMinX() + region->GetFeatureWidth(), region->GetMinY(), region->GetMinX() + region->GetFeatureWidth(), region->GetMinY() + refHeight);
        else
          mWristLineItem2 = mScene->addLine(region->GetMinX() + region->GetFeatureWidth(), region->GetMinY(), region->GetMinX() + region->GetFeatureWidth(), region->GetMinY() + refHeight);
      }
    }
    else
    {
      delete mWristLineItem;
      mWristLineItem = 0;
      delete mWristLineItem2;
      mWristLineItem2 = 0;

      QPolygonF poly;
      poly << regionRect.bottomLeft() << regionRect.topRight() << regionRect.topLeft() << regionRect.bottomRight();
      if ( mXItem )
        mXItem->setPolygon(poly);
      else
        mXItem = mScene->addPolygon(poly);
    }

    QPen coloredPen;
    if ( handClass == HandyTracker::ST_OPEN )
      coloredPen = QPen( QColor(0, 255, 0) );
    else if ( handClass == HandyTracker::ST_CLOSED )
      coloredPen = QPen( QColor(0, 0, 255) );
    else if ( handClass == HandyTracker::ST_CONFLICT )
      coloredPen = QPen( QColor(0, 255, 255) );
    else if ( handClass == HandyTracker::ST_REJECT )
      coloredPen = QPen( QColor(255, 0, 0) );
    else
      coloredPen = QPen( QColor(255, 255, 255) );

    mRectItem->setPen(coloredPen);
    if ( mXItem )
      mXItem->setPen(coloredPen);
    if ( mWristLineItem )
      mWristLineItem->setPen(coloredPen);
    if ( mWristLineItem2 )
      mWristLineItem2->setPen(coloredPen);
  }

  ui.debugLabel->setText(debugText);
}