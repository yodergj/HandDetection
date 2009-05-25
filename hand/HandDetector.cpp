#include "HandDetector.h"
#include "XMLUtils.h"
#include <stdio.h>
#include <string.h>

#define MAX_FEATURES 512
#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

#define HAND_DETECTOR_STR "HandDetector"
#define X_RES_STR "XResolution"
#define Y_RES_STR "YResolution"
#define FEATURE_STR "Features"

HandDetector::HandDetector()
{
}

HandDetector::~HandDetector()
{
}

bool HandDetector::Load(const char* filename)
{
  bool retCode = true;
  xmlDocPtr document;
  xmlNodePtr node;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "HandDetector::Load - Bad filename\n");
    return false;
  }

  document = xmlParseFile(filename);

  if ( !document )
  {
    fprintf(stderr, "HandDetector::Load - Failed parsing %s\n", filename);
    return false;
  }

  node = xmlDocGetRootElement(document);
  if ( !node )
  {
    xmlFreeDoc(document);
    fprintf(stderr, "HandDetector::Load - No root node in %s\n", filename);
    return false;
  }

  mXResolution = GetIntValue(node, X_RES_STR, 1);
  mYResolution = GetIntValue(node, Y_RES_STR, 1);
  mFeatureList = GetStringValue(node, FEATURE_STR);

  node = node->children;
  while ( node && retCode )
  {
    if ( !strcmp((char *)node->name, BAYESIAN_CLASSIFIER_STR) )
      retCode = mClassifier.Load(node);
    else if ( strcmp((char *)node->name, "text") )
      retCode = false;
    node = node->next;
  }

  xmlFreeDoc(document);

  return retCode;
}

bool HandDetector::Save(const char* filename)
{
  FILE* filePtr;
  bool retCode;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  xmlNodePtr classifierNode;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "HandDetector::Save - Bad filename\n");
    return false;
  }

  filePtr = fopen(filename, "w");
  if ( !filePtr )
  {
    fprintf(stderr, "HandDetector::Save - Failed opening %s\n", filename);
    return false;
  }

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document, NULL, (const xmlChar *)HAND_DETECTOR_STR, NULL);
  xmlDocSetRootElement(document, rootNode);

  SetIntValue(rootNode, X_RES_STR, mXResolution);
  SetIntValue(rootNode, Y_RES_STR, mYResolution);
  SetStringValue(rootNode, FEATURE_STR, mFeatureList);

  classifierNode = xmlNewNode(NULL, (const xmlChar*)BAYESIAN_CLASSIFIER_STR);
  xmlAddChild(rootNode, classifierNode);
  retCode = mClassifier.Save(classifierNode);

  xmlDocFormatDump(filePtr, document, 1);
  fclose(filePtr);
  xmlFreeDoc(document);

  return retCode;
}

bool HandDetector::Process(Image* imagePtr, int left, int right, int top, int bottom,
                           vector<Hand*> &results)
{
  Matrix featureVector;
  int classIndex;
  double confidence;
  bool done = false;
  Hand* hand;

  if ( !imagePtr || (right < left) || (bottom < top) )
  {
    fprintf(stderr, "HandDetector::Process - Invalid parameter\n");
    return false;
  }

  while ( !done )
  {
    // TODO Pick out multiple sizes and or positions of rectangles within the given area

    if ( !FillFeatureVector(imagePtr, left, right, top, bottom, featureVector) )
    {
      fprintf(stderr, "HandDetector::Process - FillFeatureVector failed\n");
      return false;
    }

    mClassifier.Classify(featureVector, classIndex, confidence);

    if ( classIndex == 0 )
    {
      printf("Hand Confidence %f\n", confidence);
      hand = new Hand;
      hand->SetBounds(left, right, top, bottom);
      results.push_back(hand);
    }

    done = true;
  }

  return true;
}

bool HandDetector::Create(string featureList, int xResolution, int yResolution,
                          int numHandGaussians, int numNonHandGaussians)
{
  int classComponents[2];
  int numFeatures;

  if ( (featureList == "") || (xResolution < 1) || (yResolution < 1) ||
       (numHandGaussians < 1) || (numNonHandGaussians < 1) )
  {
    fprintf(stderr, "HandDetector::Create - Invalid parameter\n");
    return false;
  }

  mFeatureList = featureList;
  mXResolution = xResolution;
  mYResolution = yResolution;

  numFeatures = featureList.size();
  classComponents[0] = numHandGaussians;
  classComponents[1] = numNonHandGaussians;
  if ( !mClassifier.Create(xResolution * yResolution * numFeatures, 2, classComponents) )
  {
    fprintf(stderr, "HandDetector::Create - Failed creating classifier\n");
    return false;
  }

  return true;
}

bool HandDetector::AddTrainingSample(Image* imagePtr, int left, int right, int top, int bottom, bool isHand)
{
  Matrix featureVector;
  int classIndex;

  if ( !FillFeatureVector(imagePtr, left, right, top, bottom, featureVector) )
  {
    fprintf(stderr, "HandDetector::AddTrainingSample - Failed filling feature vector\n");
    return false;
  }

  if ( isHand )
    classIndex = 0;
  else
    classIndex = 1;

  return mClassifier.AddTrainingData(featureVector, classIndex);
}

bool HandDetector::Train()
{
  return mClassifier.Train();
}

bool HandDetector::FillFeatureVector(Image* imagePtr, int left, int right, int top, int bottom,
                                     Matrix& featureVector)
{
  int i, j, k, numFeatures, fillPos, imageWidth;
  double width, height;
  int sampleTop, sampleBottom, sampleLeft, sampleRight, samplePixels;
  double* integralBuffer;
  double* integralPixel;
  Matrix sampleVector;

  if ( !imagePtr || (right < left) || (bottom < top) || (mFeatureList == "") )
  {
    fprintf(stderr, "HandDetector::FillFeatureVector - Invalid parameter\n");
    return false;
  }

  numFeatures = mFeatureList.size();
  if ( !featureVector.SetSize(numFeatures * mXResolution * mYResolution, 1) )
  {
    fprintf(stderr, "HandDetector::FillFeatureVector - Failed setting feature vector size\n");
    return false;
  }

  if ( !sampleVector.SetSize(numFeatures, 1) )
  {
    fprintf(stderr, "HandDetector::FillFeatureVector - Failed setting sample vector size\n");
    return false;
  }

  width = right - left + 1;
  height = bottom - top + 1;
  imageWidth = imagePtr->GetWidth();

  fillPos = 0;
  integralBuffer = imagePtr->GetCustomIntegralBuffer(mFeatureList);
  if ( !integralBuffer )
  {
    fprintf(stderr, "HandDetector::FillFeatureVector - Failed getting integral buffer\n");
    return false;
  }

  for (i = 0; i < mXResolution; i++)
  {
    sampleLeft = left + (int)(i * width / mXResolution);
    sampleRight = MAX(left + (int)((i + 1) * width / mXResolution) - 1, sampleLeft);
    for (j = 0; j < mYResolution; j++)
    {
      sampleTop = top + (int)(j * height / mYResolution);
      sampleBottom = MAX(top + (int)((j + 1) * height / mYResolution) - 1, sampleTop);
      samplePixels = (sampleRight - sampleLeft + 1) * (sampleBottom - sampleTop + 1);

      integralPixel = integralBuffer + numFeatures * (sampleBottom * imageWidth + sampleRight);
      sampleVector.Set(integralPixel);

      if ( sampleTop > 0 )
      {
        integralPixel = integralBuffer + numFeatures * ( (sampleTop - 1) * imageWidth + sampleRight);
        sampleVector -= integralPixel;

        if ( sampleLeft > 0 )
        {
          integralPixel = integralBuffer + numFeatures * ( (sampleTop - 1) * imageWidth + sampleLeft - 1);
          sampleVector += integralPixel;
        }
      }

      if ( sampleLeft > 0 )
      {
        integralPixel = integralBuffer + numFeatures * (sampleBottom * imageWidth + sampleLeft - 1);
        sampleVector -= integralPixel;
      }

      sampleVector *= 1.0 / samplePixels;
      for (k = 0; k < numFeatures; k++, fillPos++)
        featureVector.SetValue(fillPos, 0, sampleVector.GetValue(k, 0));
    }
  }

  return true;
}
