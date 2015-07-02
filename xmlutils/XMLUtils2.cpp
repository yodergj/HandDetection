#include "XMLUtils2.h"
#include <xercesc/util/PlatformUtils.hpp>

void InitXerces()
{
  static bool initialized = false;
  if ( initialized )
    return;

  xercesc::XMLPlatformUtils::Initialize();
  initialized = true;
}

double GetDoubleValue(xercesc::DOMElement* node, const char* propertyName, double defaultValue)
{
  double value = defaultValue;
  std::string str = CHAR( node->getAttribute(XMLSTR(propertyName)) );
  if ( !str.empty() )
    value = atof(str.c_str());

  return value;
}

void SetDoubleValue(xercesc::DOMElement* node, const char* propertyName, double value)
{
  char buf[32];
  sprintf(buf,"%.15e",value);
  node->setAttribute(XMLSTR(propertyName), XMLSTR(buf));
}

int GetIntValue(xercesc::DOMElement* node, const char* propertyName, int defaultValue)
{
  int value = defaultValue;
  std::string str = CHAR( node->getAttribute(XMLSTR(propertyName)) );
  if ( !str.empty() )
    value = atoi(str.c_str());

  return value;
}

void SetIntValue(xercesc::DOMElement* node, const char* propertyName, int value)
{
  char buf[32];
  sprintf(buf, "%d", value);
  node->setAttribute(XMLSTR(propertyName), XMLSTR(buf));
}

bool GetBoolValue(xercesc::DOMElement* node, const char* propertyName, bool defaultValue)
{
  bool value = defaultValue;
  std::string str = CHAR( node->getAttribute(XMLSTR(propertyName)) );
  if ( !str.empty() )
  {
    if ( (str[0] == 'y') || (str[0] == 'Y') ||
         (str[0] == 't') || (str[0] == 'T') )
      value = true;
    else
      value = false;
  }

  return value;
}

void SetBoolValue(xercesc::DOMElement* node, const char* propertyName, bool value)
{
  if ( value )
    node->setAttribute(XMLSTR(propertyName), XMLSTR("true"));
  else
    node->setAttribute(XMLSTR(propertyName), XMLSTR("false"));
}

std::string GetStringValue(xercesc::DOMElement* node, const char* propertyName)
{
  std::string str = CHAR( node->getAttribute(XMLSTR(propertyName)) );
  return str;
}

void SetStringValue(xercesc::DOMElement* node, const char* propertyName, const char* value)
{
  node->setAttribute(XMLSTR(propertyName), XMLSTR(value));
}
