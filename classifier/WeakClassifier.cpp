#include <stdio.h>
#include <string.h>
#include <map>
using std::map;
#include "WeakClassifier.h"
#include "ThresholdClassifier.h"
#include "RangeClassifier.h"
#include <xercesc\framework\MemBufFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#define FEATURE_STR "Features"

WeakClassifier::WeakClassifier()
{
}

WeakClassifier::WeakClassifier(const WeakClassifier& ref)
{
  operator=(ref);
}

WeakClassifier::~WeakClassifier()
{
}

WeakClassifier& WeakClassifier::operator=(const WeakClassifier& ref)
{
  mFeatureString = ref.mFeatureString;
  return *this;
}

int WeakClassifier::Classify(double value)
{
  return 0;
}

bool WeakClassifier::Train(const vector<double>& samples, const vector<double>& weights,
                           const vector<int>& classes, double* trainingError)
{
  fprintf(stderr, "WeakClassifier::Train - Training undefined for base class\n");
  return false;
}

string WeakClassifier::GetFeatureString() const
{
  return mFeatureString;
}

bool WeakClassifier::SetFeatureString(const string& featureString)
{
  if ( (int)featureString.size() != 1 )
  {
    fprintf(stderr, "WeakClassifier::SetFeatureString - Feature string \"%s\" doesn't have length 1\n", featureString.c_str());
    return false;
  }

  mFeatureString = featureString;

  return true;
}

bool WeakClassifier::SetFeatureString(const char featureLetter)
{
  mFeatureString = featureLetter;
  return true;
}

bool WeakClassifier::Print(FILE* file)
{
  if ( !file )
  {
    fprintf(stderr, "WeakClassifier::Print - Invalid parameter\n");
    return false;
  }

  fprintf(file, "WeakClassifier\n");
  fprintf(file, "Feature %s\n", mFeatureString.c_str());

  return true;
}

bool WeakClassifier::Save(const char* filename)
{
  FILE* file;
  bool retCode = true;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "WeakClassifier::Save - Invalid parameter\n");
    return false;
  }

  file = fopen(filename, "w");
  if ( !file )
  {
    fprintf(stderr, "WeakClassifier::Save - Failed opening %s\n", filename);
    return false;
  }

  InitXerces();
  xercesc::DOMImplementation* impl = xercesc::DOMImplementationRegistry::getDOMImplementation(XMLSTR("Core"));
  xercesc::DOMLSOutput* output = impl->createLSOutput();
  xercesc::DOMLSSerializer* serializer = impl->createLSSerializer();
  xercesc::MemBufFormatTarget* formatTarget = new xercesc::MemBufFormatTarget();
  output->setByteStream(formatTarget);
  xercesc::DOMDocument* doc = impl->createDocument(0, 0, 0);
  //xercesc::DOMElement* rootElem = doc->getDocumentElement();
  //doc->renameNode(rootElem, 0, X(ADABOOST_CLASSIFIER_STR));

  Save(doc, true);

  serializer->write(doc, output);
  const unsigned char* data = formatTarget->getRawBuffer();
  unsigned int len = formatTarget->getLen();
  fwrite(data, 1, len, file);

  doc->release();
  delete formatTarget;
  fclose(file);

  return retCode;
}

xercesc::DOMElement* WeakClassifier::Save(xercesc::DOMDocument* document, bool toRootElem)
{
  if ( !document )
    return 0;

  xercesc::DOMElement* classifierNode = 0;
  if ( toRootElem )
  {
    classifierNode = document->getDocumentElement();
    document->renameNode(classifierNode, 0, XMLSTR(WEAK_CLASSIFIER_STR));
  }
  else
    classifierNode = document->createElement(XMLSTR(WEAK_CLASSIFIER_STR));

  if ( classifierNode )
    SetStringValue(classifierNode, FEATURE_STR, mFeatureString.c_str());
  else
    fprintf(stderr, "WeakClassifier::Save - Failed getting xml node\n");

  return classifierNode;
}

WeakClassifier* WeakClassifier::Load(const char* filename)
{
  WeakClassifier* result = NULL;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "WeakClassifier::Load - Bad filename\n");
    return NULL;
  }

  InitXerces();

  xercesc::XercesDOMParser* domParser = new xercesc::XercesDOMParser();
  xercesc::DOMDocument* doc = 0;

  domParser->parse(XMLSTR(filename));
  doc = domParser->getDocument();
  xercesc::DOMElement* rootElem = doc->getDocumentElement();
  
  result = Load(rootElem);

  delete domParser;

  return result;
}

WeakClassifier* WeakClassifier::Load(xercesc::DOMElement* classifierNode)
{
  WeakClassifier* result = NULL;

  if ( !classifierNode )
    return NULL;

  std::string nodeName = CHAR(classifierNode->getNodeName());
  if ( strcmp(nodeName.c_str(), WEAK_CLASSIFIER_STR) )
  {
    fprintf(stderr, "WeakClassifier::Load - Invalid node name <%s>\n", nodeName.c_str());
    return NULL;
  }

  string type = GetStringValue(classifierNode, CLASSIFIER_TYPE_STR);
  if ( type == RANGE_CLASSIFIER_STR )
    result = new RangeClassifier;
  else if ( type == THRESHOLD_CLASSIFIER_STR )
    result = new ThresholdClassifier;
  else
    fprintf(stderr, "WeakClassifier::Load - Invalid classifier type %s\n", type.c_str());

  if ( result && !result->LoadClassifier( classifierNode ) )
  {
    delete result;
    result = NULL;
  }

  return result;
}

bool WeakClassifier::LoadClassifier(xercesc::DOMElement* classifierNode)
{
  mFeatureString = GetStringValue(classifierNode, FEATURE_STR);

  return true;
}
