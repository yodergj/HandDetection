#include <string.h>
#include "CompositeClassifier.h"
#include "Matrix.h"
#include "AdaboostClassifier.h"
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

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

  InitXerces();
  xercesc::DOMImplementation* impl = xercesc::DOMImplementationRegistry::getDOMImplementation(XMLSTR("Core"));
  xercesc::DOMLSOutput* output = impl->createLSOutput();
  xercesc::DOMLSSerializer* serializer = impl->createLSSerializer();
  xercesc::MemBufFormatTarget* formatTarget = new xercesc::MemBufFormatTarget();
  output->setByteStream(formatTarget);
  xercesc::DOMDocument* doc = impl->createDocument(0, XMLSTR(ADABOOST_CLASSIFIER_STR), 0);
  xercesc::DOMElement* rootElem = doc->getDocumentElement();

  Save(doc, rootElem);

  serializer->write(doc, output);
  const unsigned char* data = formatTarget->getRawBuffer();
  unsigned int len = formatTarget->getLen();
  fwrite(data, 1, len, file);

  doc->release();
  delete formatTarget;
  fclose(file);

  return retCode;
}

bool CompositeClassifier::Save(xercesc::DOMDocument* doc, xercesc::DOMElement* classifierNode)
{
  int i, numClassifiers;
  bool retCode = true;
  xercesc::DOMElement* adaboostNode;

  numClassifiers = mClassifiers.size();
  SetStringValue(classifierNode, FEATURE_STR, mFeatureString.c_str());
  for (i = 0; (i < numClassifiers) && retCode; i++)
  {
    adaboostNode = doc->createElement(XMLSTR(ADABOOST_CLASSIFIER_STR));
    classifierNode->appendChild(adaboostNode);
    SetStringValue(adaboostNode, CLASS_NAME_STR, mClassNames[i].c_str());
    retCode = mClassifiers[i]->Save(doc, adaboostNode);
  }

  return retCode;
}

bool CompositeClassifier::Load(const char* filename)
{
  bool retCode = true;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "CompositeClassifier::Load - Bad filename\n");
    return false;
  }

  InitXerces();

  xercesc::XercesDOMParser* domParser = new xercesc::XercesDOMParser();
  xercesc::DOMDocument* doc = 0;

  domParser->parse(XMLSTR(filename));
  doc = domParser->getDocument();
  xercesc::DOMElement* rootElem = doc->getDocumentElement();
  
  retCode = Load(rootElem);

  delete domParser;

  return retCode;
}

bool CompositeClassifier::Load(xercesc::DOMElement* classifierNode)
{
  bool retCode = true;
  AdaboostClassifier* classifier;
  mFeatureString = GetStringValue(classifierNode, FEATURE_STR);

  xercesc::DOMNodeList* nodeList = classifierNode->getChildNodes();
  for (XMLSize_t j = 0; retCode && (j < nodeList->getLength()); j++)
  {
    xercesc::DOMNode* node = nodeList->item(j);
    xercesc::DOMElement* elem = dynamic_cast<xercesc::DOMElement*>(node);
    if ( !elem )
      continue;

    std::string nodeName = CHAR(elem->getNodeName());
    if ( !strcmp(nodeName.c_str(), ADABOOST_CLASSIFIER_STR) )
    {
      classifier = new AdaboostClassifier;
      if ( classifier->Load(elem) )
      {
        mClassNames.push_back( GetStringValue(elem, CLASS_NAME_STR) );
        mClassifiers.push_back(classifier);
      }
      else
      {
        delete classifier;
        fprintf(stderr, "CompositeClassifier::Load - Failed loading classifier %d\n", mClassifiers.size());
        retCode = false;
      }
    }
    else if ( strcmp(nodeName.c_str(), "text") )
    {
      fprintf(stderr, "CompositeClassifier::Load - Found unknown node %s\n", nodeName.c_str());
      retCode = false;
    }
  }

  return retCode;
}
