#ifndef _HANDY_TRACKER_H
#define _HANDY_TRACKER_H

#include <vector>
class ColorRegion;
class AdaboostClassifier;

class HandyTracker
{
public:
  HandyTracker();
  ~HandyTracker();

  bool AnalyzeRegion(ColorRegion* region);

  bool SetOpenClassifier(AdaboostClassifier* classifier);
  bool SetClosedClassifier(AdaboostClassifier* classifier);

  enum HandState
  {
    ST_UNKNOWN, ST_OPEN, ST_CLOSED, ST_CONFLICT
  };

  HandState GetLastState();
private:
  std::vector<ColorRegion*> mRegionHistory;
  std::vector<HandState> mStateHistory;

  AdaboostClassifier* mOpenClassifier;
  AdaboostClassifier* mClosedClassifier;
};

#endif