#ifndef HANDYMOUSE_H
#define HANDYMOUSE_H

#include <QtWidgets/QMainWindow>
#include <QGraphicsPixmapItem>
#include "ui_handymouse.h"
#include "VideoDecoder.h"

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

private:
  Ui::HandyMouseClass ui;
  VideoDecoder mVideoDecoder;
  QGraphicsScene* mScene;
  QGraphicsPixmapItem* mPixmapItem;
  std::vector<QPixmap> mPixmaps;
  size_t mFrameNumber;
};

#endif // HANDYMOUSE_H
