#ifndef _HANDY_TRACKER_H
#define _HANDY_TRACKER_H

#include <vector>
#include "Matrix.h"
class ColorRegion;
class AdaboostClassifier;

#define ASPECT_RATIO_INDEX 25
#define GRID_DIM_SIZE 5

class HandyTracker
{
public:
  HandyTracker();
  ~HandyTracker();

  bool AnalyzeRegion(ColorRegion* region);
  bool AnalyzeRegionForInitialization(ColorRegion* region);
  bool GenerateFeatureData(ColorRegion* region, Matrix& featureData, int heightRestriction = 0);

  static int mNumFeatures;

  bool SetOpenClassifier(AdaboostClassifier* classifier);
  bool SetClosedClassifier(AdaboostClassifier* classifier);

  enum HandState
  {
    ST_UNKNOWN, ST_OPEN, ST_CLOSED, ST_CONFLICT, ST_REJECT
  };

  HandState GetLastState();

  HandState GetState(int frameNumber);
  ColorRegion* GetRegion(int frameNumber);
  Matrix* GetOpenFeatureData(int frameNumber);
  Matrix* GetClosedFeatureData(int frameNumber);
  void ResetHistory();
  void PurgeRegion(int frameNumber);
private:
  HandState CalculateStateResult(int openResult, int closedResult);
  std::vector<ColorRegion*> mRegionHistory;
  std::vector<Matrix> mOpenFeatureHistory;
  std::vector<Matrix> mClosedFeatureHistory;
  std::vector<HandState> mStateHistory;

  AdaboostClassifier* mOpenClassifier;
  AdaboostClassifier* mClosedClassifier;
};

#endif