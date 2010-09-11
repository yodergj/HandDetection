#ifndef DUMMY_FLESH_DETECTOR_H
#define DUMMY_FLESH_DETECTOR_H

#include "FleshDetector.h"

class DummyFleshDetector : public FleshDetector
{
  public:
    DummyFleshDetector();
    ~DummyFleshDetector();
    virtual bool Load(const char* filename);
  protected:
    virtual bool CalcConfidence(Image* imagePtr, int xScale, int yScale);
};

#endif
