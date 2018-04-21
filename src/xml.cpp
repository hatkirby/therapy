#include "xml.h"
#include <stdexcept>

xmlChar* getProp(xmlNodePtr node, const char* attr)
{
  xmlChar* key = xmlGetProp(node, reinterpret_cast<const xmlChar*>(attr));
  if (key == nullptr)
  {
    throw std::invalid_argument("Error parsing XML file");
  }

  return key;
}
