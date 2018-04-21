#include "prototype_manager.h"
#include <utility>
#include <stdexcept>
#include "xml.h"

PrototypeManager::PrototypeManager(std::string path)
{
  xmlDocPtr doc = xmlParseFile(path.c_str());
  if (doc == nullptr)
  {
    throw std::invalid_argument("Cannot find prototypes file");
  }

  xmlNodePtr top = xmlDocGetRootElement(doc);
  if (top == nullptr)
  {
    throw std::invalid_argument("Error parsing prototypes file");
  }

  if (xmlStrcmp(top->name, reinterpret_cast<const xmlChar*>("entities")))
  {
    throw std::invalid_argument("Error parsing prototypes file");
  }

  xmlChar* key = nullptr;

  for (xmlNodePtr node = top->xmlChildrenNode;
    node != nullptr;
    node = node->next)
  {
    if (!xmlStrcmp(node->name, reinterpret_cast<const xmlChar*>("entity")))
    {
      key = getProp(node, "id");
      std::string prototypeName(reinterpret_cast<char*>(key));
      xmlFree(key);

      key = getProp(node, "sprite");
      std::string spritePath(reinterpret_cast<char*>(key));
      xmlFree(key);

      key = getProp(node, "width");
      int width = atoi(reinterpret_cast<char*>(key));
      xmlFree(key);

      key = getProp(node, "height");
      int height = atoi(reinterpret_cast<char*>(key));
      xmlFree(key);

      key = getProp(node, "action");
      std::string actionStr(reinterpret_cast<char*>(key));
      xmlFree(key);

      Prototype::Action pAction = Prototype::Action::none;
      if (actionStr == "save")
      {
        pAction = Prototype::Action::save;
      }

      prototypes_.emplace(
        std::piecewise_construct,
        std::tie(prototypeName),
        std::tie(
          width,
          height,
          spritePath,
          pAction));
    }
  }

  xmlFreeDoc(doc);
}
