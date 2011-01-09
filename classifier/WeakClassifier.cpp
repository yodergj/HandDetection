#include <stdio.h>
#include <string.h>
#include <map>
using std::map;
#include "WeakClassifier.h"
#include "ThresholdClassifier.h"
#include "RangeClassifier.h"

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
  xmlDocPtr document;
  xmlNodePtr rootNode;
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

  document = xmlNewDoc(NULL);
  rootNode = Save(document);
  if ( rootNode )
  {
    xmlDocSetRootElement(document, rootNode);
    xmlDocFormatDump(file, document, 1);
  }
  else
    retCode = false;

  fclose(file);
  xmlFreeDoc(document);

  return retCode;
}

xmlNodePtr WeakClassifier::Save(xmlDocPtr document)
{
  xmlNodePtr classifierNode = NULL;
  if ( document )
    classifierNode = xmlNewDocNode(document, NULL, (const xmlChar *)WEAK_CLASSIFIER_STR, NULL);
  else
    classifierNode = xmlNewNode(NULL, (const xmlChar *)WEAK_CLASSIFIER_STR);

  if ( classifierNode )
    SetStringValue(classifierNode, FEATURE_STR, mFeatureString);
  else
    fprintf(stderr, "WeakClassifier::Save - Failed getting xml node\n");

  return classifierNode;
}

WeakClassifier* WeakClassifier::Load(const char* filename)
{
  WeakClassifier* result = NULL;
  xmlDocPtr document;
  xmlNodePtr node;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "WeakClassifier::Load - Bad filename\n");
    return NULL;
  }

  document = xmlParseFile(filename);

  if ( !document )
  {
    fprintf(stderr, "WeakClassifier::Load - Failed parsing %s\n", filename);
    return NULL;
  }

  node = xmlDocGetRootElement(document);
  if ( !node )
  {
    xmlFreeDoc(document);
    fprintf(stderr, "WeakClassifier::Load - No root node in %s\n", filename);
    return NULL;
  }

  result = Load(node);

  xmlFreeDoc(document);

  return result;
}

WeakClassifier* WeakClassifier::Load(xmlNodePtr classifierNode)
{
  WeakClassifier* result = NULL;

  if ( !classifierNode )
    return NULL;

  if ( !classifierNode->name ||
       strcmp((const char *)classifierNode->name, WEAK_CLASSIFIER_STR) )
  {
    fprintf(stderr, "WeakClassifier::Load - Invalid node name %s\n",
            classifierNode->name ? (const char *)classifierNode->name : "NULL");
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

bool WeakClassifier::LoadClassifier(xmlNodePtr classifierNode)
{
  mFeatureString = GetStringValue(classifierNode, FEATURE_STR);

  return true;
}