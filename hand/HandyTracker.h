#ifndef _HANDY_TRACKER_H
#define _HANDY_TRACKER_H

#include <vector>
#include "Matrix.h"
class ColorRegion;
class AdaboostClassifier;

class HandyTracker
{
public:
  HandyTracker();
  ~HandyTracker();

  bool AnalyzeRegion(ColorRegion* region);
  bool GenerateFeatureData(ColorRegion* region, Matrix& featureData);

  static int mNumFeatures;

  bool SetOpenClassifier(AdaboostClassifier* classifier);
  bool SetClosedClassifier(AdaboostClassifier* classifier);

  enum HandState
  {
    ST_UNKNOWN, ST_OPEN, ST_CLOSED, ST_CONFLICT
  };

  HandState GetLastState();

  HandState GetState(int frameNumber);
  ColorRegion* GetRegion(int frameNumber);
  Matrix* GetFeatureData(int frameNumber);
  void ResetHistory();
private:
  std::vector<ColorRegion*> mRegionHistory;
  std::vector<Matrix> mFeatureHistory;
  std::vector<HandState> mStateHistory;

  AdaboostClassifier* mOpenClassifier;
  AdaboostClassifier* mClosedClassifier;
};

#endif