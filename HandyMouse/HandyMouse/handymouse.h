#ifndef HANDYMOUSE_H
#define HANDYMOUSE_H

#include <QtWidgets/QMainWindow>
#include <QGraphicsPixmapItem>
#include "ui_handymouse.h"
#include "VideoDecoder.h"
class HandyTracker;

class HandyMouse : public QMainWindow
{
  Q_OBJECT

public:
  HandyMouse(QWidget *parent = 0);
  ~HandyMouse();

public slots:
  void on_prevButton_clicked();
  void on_nextButton_clicked();
  void on_actionLoad_triggered();
  void on_actionLoad_Open_Classifier_triggered();
  void on_actionLoad_Closed_Classifier_triggered();
  void on_actionExport_Frame_triggered();
  void on_actionExport_Hand_triggered();

private:
  void ProcessFrame(Image* img);
  void DisplayResults();
  Ui::HandyMouseClass ui;
  VideoDecoder* mVideoDecoder;
  QGraphicsScene* mScene;
  QGraphicsPixmapItem* mPixmapItem;
  QGraphicsEllipseItem* mEllipseItem;
  QGraphicsRectItem* mRectItem;
  std::vector<QPixmap> mPixmaps;
  size_t mFrameNumber;

  HandyTracker* mTracker;

  QString mImageFilter;
  void BuildImageFilter();
};

#endif // HANDYMOUSE_H
