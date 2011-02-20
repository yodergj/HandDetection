#include <string.h>
#include "CompositeClassifier.h"
#include "Matrix.h"
#include "AdaboostClassifier.h"

#define FEATURE_STR "Features"
#define CLASS_NAME_STR "ClassName"

CompositeClassifier::CompositeClassifier()
{
}

CompositeClassifier::~CompositeClassifier()
{
  Clear();
}

void CompositeClassifier::Clear()
{
  vector<AdaboostClassifier*>::iterator itr;
  for (itr = mClassifiers.begin(); itr != mClassifiers.end(); itr++)
    delete *itr;
  mClassifiers.clear();
  mClassNames.clear();
}

int CompositeClassifier::Classify(const Matrix& data)
{
  int i, numClassifiers;
  int resultClass = -1;

  numClassifiers = mClassifiers.size();
  for (i = 0; i < numClassifiers; i++)
    if ( mClassifiers[i]->Classify(data) == 0 )
    {
      printf("Matched Class %d %s\n", i, mClassNames[i].c_str());
      if ( resultClass == -1 )
        resultClass = i;
      else
        printf("Result Confusion\n");
    }

  return resultClass;
}

bool CompositeClassifier::AddClassifier(AdaboostClassifier* classifier, const string& className)
{
  int i, numFeatures, numClassifiers;

  if ( !classifier )
    return false;

  if ( mClassifiers.empty() )
    mFeatureString = classifier->GetFeatureString();
  else
  {
    string features = classifier->GetFeatureString();
    numFeatures = features.size();
    for (i = 0; i < numFeatures; i++)
      if ( mFeatureString.find(features[i]) == string::npos )
        mFeatureString.push_back(features[i]);
  }
  mClassifiers.push_back(classifier);
  mClassNames.push_back(className);

  numClassifiers = mClassifiers.size();
  for (i = 0; i < numClassifiers; i++)
    mClassifiers[i]->SetFeatureString(mFeatureString);

  return true;
}

string CompositeClassifier::GetFeatureString() const
{
  return mFeatureString;
}

int CompositeClassifier::GetNumClasses() const
{
  return (int)mClassNames.size();
}

string CompositeClassifier::GetClassName(int i) const
{
  if ( (i < 0) || (i >= (int)mClassNames.size()) )
    return "";
  return mClassNames[i];
}

bool CompositeClassifier::Print(FILE* file)
{
  bool retVal = true;
  int i, numClassifiers;
  if ( !file )
  {
    fprintf(stderr, "CompositeClassifier::Print - Invalid parameter\n");
    return false;
  }

  numClassifiers = mClassifiers.size();
  fprintf(file, "CompositeClassifier\n");
  fprintf(file, "Feature %s\n", mFeatureString.c_str());
  fprintf(file, "Num Classifiers %d\n", numClassifiers);
  for (i = 0; i < numClassifiers; i++)
  {
    fprintf(file, "Class Name %s\n", mClassNames[i].c_str());
    if ( mClassifiers[i] )
      retVal &= mClassifiers[i]->Print(file);
    else
    {
      retVal = false;
      fprintf(file, "Missing Classifier %d\n", i);
    }
  }

  return retVal;
}

bool CompositeClassifier::Save(const char* filename)
{
  FILE* file;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  bool retCode = true;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "CompositeClassifier::Save - Invalid parameter\n");
    return false;
  }

  file = fopen(filename, "w");
  if ( !file )
  {
    fprintf(stderr, "CompositeClassifier::Save - Failed opening %s\n", filename);
    return false;
  }

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document, NULL, (const xmlChar *)COMPOSITE_CLASSIFIER_STR, NULL);
  xmlDocSetRootElement(document, rootNode);

  retCode = Save(rootNode);

  xmlDocFormatDump(file, document, 1);
  fclose(file);
  xmlFreeDoc(document);

  return retCode;
}

bool CompositeClassifier::Save(xmlNodePtr classifierNode)
{
  int i, numClassifiers;
  bool retCode = true;
  xmlNodePtr adaboostNode;

  numClassifiers = mClassifiers.size();
  SetStringValue(classifierNode, FEATURE_STR, mFeatureString);
  for (i = 0; (i < numClassifiers) && retCode; i++)
  {
    adaboostNode = xmlNewNode(NULL, (const xmlChar*)ADABOOST_CLASSIFIER_STR);
    xmlAddChild(classifierNode, adaboostNode);
    SetStringValue(adaboostNode, CLASS_NAME_STR, mClassNames[i]);
    retCode = mClassifiers[i]->Save(adaboostNode);
  }

  return retCode;
}

bool CompositeClassifier::Load(const char* filename)
{
  bool retCode = true;
  xmlDocPtr document;
  xmlNodePtr node;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "CompositeClassifier::Load - Bad filename\n");
    return false;
  }

  document = xmlParseFile(filename);

  if ( !document )
  {
    fprintf(stderr, "CompositeClassifier::Load - Failed parsing %s\n", filename);
    return false;
  }

  node = xmlDocGetRootElement(document);
  if ( !node )
  {
    xmlFreeDoc(document);
    fprintf(stderr, "CompositeClassifier::Load - No root node in %s\n", filename);
    return false;
  }

  retCode = Load(node);

  xmlFreeDoc(document);

  return retCode;
}

bool CompositeClassifier::Load(xmlNodePtr classifierNode)
{
  bool retCode = true;
  AdaboostClassifier* classifier;
  mFeatureString = GetStringValue(classifierNode, FEATURE_STR);
  xmlNodePtr node;

  node = classifierNode->children;
  while ( node && retCode )
  {
    if ( !strcmp((char *)node->name, ADABOOST_CLASSIFIER_STR) )
    {
      classifier = new AdaboostClassifier;
      if ( classifier->Load(node) )
      {
        mClassNames.push_back( GetStringValue(node, CLASS_NAME_STR) );
        mClassifiers.push_back(classifier);
      }
      else
      {
        delete classifier;
        fprintf(stderr, "CompositeClassifier::Load - Failed loading classifier %d\n", mClassifiers.size());
        retCode = false;
      }
    }
    else if ( strcmp((char *)node->name, "text") )
    {
      fprintf(stderr, "CompositeClassifier::Load - Found unknown node %s\n", (char*)node->name);
      retCode = false;
    }
    node = node->next;
  }

  return retCode;
}