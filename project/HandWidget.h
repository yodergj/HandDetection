/* Gabe Yoder
   CS660
   Course Project
*/
#ifndef HAND_WIDGET_H
#define HAND_WIDGET_H

#include <qgl.h>
#include "Hand.h"
#include "WRLModel.h"

typedef enum
{
  VIEW_FRONT, VIEW_SIDE
} ViewDirType;

class HandWidget : public QGLWidget
{
Q_OBJECT
public:
  HandWidget(QWidget *parent=0, const char *name=0, const QGLWidget *shareWidget=0, WFlags f=0);
  ~HandWidget();
  void SetHand(Hand *hand);
  void SetHand(WRLModel *hand);
  bool SetView(ViewDirType direction);
protected:
  virtual void initializeGL();
  virtual void resizeGL(int width, int height);
  virtual void paintGL();
private:
  void paintHand();
  void paintModel();
  GLfloat mXTran;
  GLfloat mYTran;
  GLfloat mZTran;
  int mXRot;
  int mYRot;
  Hand *mHand;
  WRLModel *mModel;
};

#endif
