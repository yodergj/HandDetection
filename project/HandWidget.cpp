/* Gabe Yoder
   CS660
   Course Project
*/

#include <qapplication.h>
#include "HandWidget.h"

#define SHOW_MODEL_STATS 0
#define USE_LIGHTING 0
#define MODEL_SCALE_FACTOR 400000

HandWidget::HandWidget(QWidget *parent, const char *name, const QGLWidget *shareWidget, WFlags f) : QGLWidget(parent, name, shareWidget, f)
{
  mHand = NULL;
  mModel = NULL;
  mXRot = 0;
  mYRot = 0;
  mXTran = 100;
  mYTran = 150;
  mZTran = 400;
}

HandWidget::~HandWidget()
{
}

bool HandWidget::SetView(ViewDirType direction)
{
  switch (direction)
  {
    case VIEW_FRONT:
      mYRot = 0;
      mXTran = 100;
      mYTran = 150;
      mZTran = 400;
      break;
    case VIEW_SIDE:
      mYRot = 90;
      mXTran = 50;
      mYTran = 175;
      mZTran = 400;
      break;
    default:
      fprintf(stderr,"Invalid view direction\n");
      return false;
  }
  return true;
}

void HandWidget::SetHand(Hand *hand)
{
  mHand = hand;
}

void HandWidget::SetHand(WRLModel *hand)
{
  mModel = hand;
}

void HandWidget::initializeGL()
{
  glClearColor(0.0, 0.0, 0.0, 0.0);

#if !USE_LIGHTING
  glShadeModel(GL_FLAT);
  glEnable(GL_DEPTH_TEST);
#else
  GLfloat spec0[] = { (GLfloat).7, (GLfloat).7, (GLfloat).7, 1.0 };
  GLfloat amb0[] = { (GLfloat).1, (GLfloat).1, (GLfloat).1, 1.0 };
  GLfloat pos0[] = { 1, 1, 1, 0 };

  glEnable(GL_NORMALIZE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glShadeModel(GL_SMOOTH);
  glLightfv(GL_LIGHT0, GL_SPECULAR, spec0);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, spec0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, amb0);
  glLightfv(GL_LIGHT0, GL_POSITION, pos0);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
#endif
}

void HandWidget::resizeGL(int width, int height)
{
  double aspect = ((double)width)/height;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  /* Viewing angle is approximately 60 degrees, when width == height */
  glFrustum((GLfloat)-.577*aspect,(GLfloat).577*aspect,(GLfloat)-.577,(GLfloat).577,1,2000);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glScalef(1.0, -1.0, 1.0);
  glTranslatef(0.0, -1.0, 0.0);

  glViewport(0,0,(GLint)width,(GLint)height);
}

void HandWidget::paintHand()
{
  int i, j, k;
  Cylinder *bone;
  AnglePoint *location;
  int pointIndex;
  Point *point;

  for (i=0; i < NUM_BONES; i++)
  {
#if 0
    if ( i == 0 )
      glColor3f(1,0,0);
    else if ( i == 1 )
      glColor3f(0,1,0);
    else if ( i == 2 )
      glColor3f(0,0,1);
    else
      glColor3f(1,1,1);
#endif
    glPushMatrix();

    bone = &(mHand->bones[i]);
    location = &(mHand->locations[i]);

    glTranslatef(location->point.x, location->point.y, location->point.z);
    glRotatef(location->XRotation,1,0,0);
    glRotatef(location->ZRotation,0,0,1);
    glBegin(GL_TRIANGLES);
    for (j=0; j < bone->numFaces; j++)
    {
      for (k=0; k < 3; k++)
      {
        pointIndex = bone->faces[j].points[k];
        point = &(bone->points[pointIndex]);
        glVertex3f(point->x, point->y, point->z);
      }
    }
    glEnd();
    glPopMatrix();
  }
}

void HandWidget::paintModel()
{
  int i, j;
  int pointIndex;
#if SHOW_MODEL_STATS
  double minX = 999999;
  double maxX = -999999;
  double minY = 999999;
  double maxY = -999999;
  double minZ = 999999;
  double maxZ = -999999;
#endif

  glBegin(GL_TRIANGLES);
  for (i=0; i<mModel->numFaces; i++)
  {
    for (j=0; j<3; j++)
    {
      pointIndex = mModel->faces[i].points[j];
      glVertex3f(mModel->points[pointIndex].x * MODEL_SCALE_FACTOR,
                 mModel->points[pointIndex].y * MODEL_SCALE_FACTOR,
                 mModel->points[pointIndex].z * MODEL_SCALE_FACTOR);
#if SHOW_MODEL_STATS
      if ( mModel->points[pointIndex].x < minX )
        minX = mModel->points[pointIndex].x;
      if ( mModel->points[pointIndex].x > maxX )
        maxX = mModel->points[pointIndex].x;
      if ( mModel->points[pointIndex].y < minY )
        minY = mModel->points[pointIndex].y;
      if ( mModel->points[pointIndex].y > maxY )
        maxY = mModel->points[pointIndex].y;
      if ( mModel->points[pointIndex].z < minZ )
        minZ = mModel->points[pointIndex].z;
      if ( mModel->points[pointIndex].z > maxZ )
        maxZ = mModel->points[pointIndex].z;
#endif
    }
  }
  glEnd();
#if SHOW_MODEL_STATS
  printf("X Range %f - %f\n", minX, maxX);
  printf("Y Range %f - %f\n", minY, maxY);
  printf("Z Range %f - %f\n", minZ, maxZ);
#endif
}

void HandWidget::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(-mXTran,-mYTran,-mZTran);
  glRotatef(mXRot,1,0,0);
  glRotatef(-mYRot,0,1,0);

  glColor3f(1,1,1);

#if USE_LIGHTING
  GLfloat pos0[] = { mXTran, mYTran, mZTran, 1.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, pos0);
#endif

  if ( mModel )
    paintModel();
  else if ( mHand )
    paintHand();

  glFlush();
  swapBuffers();
}




#if 0
int main(int argc, char *argv[])
{
  QApplication app(argc,argv);
  HandWidget gui;
  Hand hand;
  gui.SetHand(&hand);
  gui.show();
  app.setMainWidget(&gui);
  return app.exec();
}
#endif
