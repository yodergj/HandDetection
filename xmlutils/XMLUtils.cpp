///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004-2008 Gabriel Yoder
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////
#include "XMLUtils.h"
#include <string.h>

/*!
  @param node - the XML DOM node which contains the specified property
  @param propertyName - the name of the desired property
  @param defaultValue - the value to return if the property isn't found
  @return the double value of the property with the specified name
  @return defaultValue is returned if the property is not found
*/
double GetDoubleValue(xmlNodePtr node, const char* propertyName,
                      double defaultValue)
{
  xmlChar *propertyValue;
  double value;

  value = defaultValue;
  propertyValue = xmlGetProp(node, (const xmlChar *)propertyName);
  if ( propertyValue )
  {
    value = atof((const char*)propertyValue);
    xmlFree(propertyValue);
  }
  return value;
}

void SetDoubleValue(xmlNodePtr node,const char* propertyName, double value)
{
  char buf[32];
  sprintf(buf,"%f",value);
  xmlNewProp(node, (const xmlChar*)propertyName, (const xmlChar*)buf);
}

/*!
  @param node - the XML DOM node which contains the specified property
  @param propertyName - the name of the desired property
  @param defaultValue - the value to return if the property isn't found
  @return the integer value of the property with the specified name
  @return defaultValue is returned if the property is not found
*/
int GetIntValue(xmlNodePtr node, const char* propertyName, int defaultValue)
{
  xmlChar *propertyValue;
  int value;

  value = defaultValue;
  propertyValue = xmlGetProp(node, (const xmlChar *)propertyName);
  if ( propertyValue )
  {
    value = atoi((const char*)propertyValue);
    xmlFree(propertyValue);
  }
  return value;
}

void SetIntValue(xmlNodePtr node, const char* propertyName, int value)
{
  char buf[32];
  sprintf(buf,"%d",value);
  xmlNewProp(node, (const xmlChar*)propertyName, (const xmlChar*)buf);
}

bool GetBoolValue(xmlNodePtr node, const char* propertyName, bool defaultValue)
{
  xmlChar *propertyValue;
  int value;

  value = defaultValue;
  propertyValue = xmlGetProp(node, (const xmlChar *)propertyName);
  if ( propertyValue )
  {
    if ( (propertyValue[0] == 'y') || (propertyValue[0] == 'Y') ||
         (propertyValue[0] == 't') || (propertyValue[0] == 'T') )
      value = true;
    else
      value = false;
    xmlFree(propertyValue);
  }
  return value;
}

void SetBoolValue(xmlNodePtr node, const char* propertyName, bool value)
{
  if ( value )
    xmlNewProp(node, (const xmlChar*)propertyName, (const xmlChar*)"true");
  else
    xmlNewProp(node, (const xmlChar*)propertyName, (const xmlChar*)"false");
}

/*!
  @param node - the XML DOM node which contains the specified property
  @param propertyName - the name of the desired property
  @return the string value of the property with the specified name
  @return "" is returned if the property is not found
*/
string GetStringValue(xmlNodePtr node, const char* propertyName)
{
  xmlChar *propertyValue;
  int maxLen, retcode;
  string value="";
  char* buf;

  propertyValue = xmlGetProp(node, (const xmlChar *)propertyName);
  if ( propertyValue )
  {
    maxLen = strlen((char *)propertyValue)+1;
    buf = new char[maxLen];
    retcode = UTF8Toisolat1((unsigned char*)buf,&maxLen,(unsigned char*)propertyValue,&maxLen);
    if ( retcode >= 0 )
      value = buf;
    xmlFree(propertyValue);
    delete[] buf;
  }
  return value;
}

void SetStringValue(xmlNodePtr node,const char* propertyName,string value)
{
  xmlNewProp(node, (const xmlChar*)propertyName, (const xmlChar*)value.c_str());
}
