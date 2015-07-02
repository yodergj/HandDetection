#ifndef _XMLUTILS_2_H
#define _XMLUTILS_2_H

#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <string>

class XStr
{
public :
  XStr(const char* const toTranscode)
  {
    // Call the private transcoding method
    mUnicodeForm = xercesc::XMLString::transcode(toTranscode);
    mASCIIForm = 0;
  }

  XStr(const XMLCh* const toTranscode)
  {
    // Call the private transcoding method
    mASCIIForm = xercesc::XMLString::transcode(toTranscode);
    mUnicodeForm = 0;
  }

  ~XStr()
  {
    if ( mUnicodeForm )
      xercesc::XMLString::release(&mUnicodeForm);
    if ( mASCIIForm )
      xercesc::XMLString::release(&mASCIIForm);
  }

  const XMLCh* unicodeForm() const
  {
    return mUnicodeForm;
  }

  const char* asciiForm() const
  {
    return mASCIIForm;
  }

private :
  XMLCh* mUnicodeForm;
  char* mASCIIForm;
};

#define XMLSTR(str) XStr(str).unicodeForm()
#define CHAR(str) XStr(str).asciiForm()

void InitXerces();
double GetDoubleValue(xercesc::DOMElement* node, const char* propertyName, double defaultValue);
void SetDoubleValue(xercesc::DOMElement* node, const char* propertyName, double value);
int GetIntValue(xercesc::DOMElement* node, const char* propertyName, int defaultValue);
void SetIntValue(xercesc::DOMElement* node, const char* propertyName, int value);
bool GetBoolValue(xercesc::DOMElement* node, const char* propertyName, bool defaultValue);
void SetBoolValue(xercesc::DOMElement* node, const char* propertyName, bool value);
std::string GetStringValue(xercesc::DOMElement* node, const char* propertyName);
void SetStringValue(xercesc::DOMElement* node, const char* propertyName, const char* value);

#endif
