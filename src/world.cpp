#include "world.h"
#include <libxml/parser.h>
#include <stdexcept>
#include <cstring>
#include "consts.h"

inline xmlChar* getProp(xmlNodePtr node, const char* attr)
{
  xmlChar* key = xmlGetProp(node, reinterpret_cast<const xmlChar*>(attr));
  if (key == nullptr)
  {
    throw std::invalid_argument("Error parsing world file");
  }

  return key;
}

World::World(std::string filename)
{
  xmlDocPtr doc = xmlParseFile(filename.c_str());
  if (doc == nullptr)
  {
    throw std::invalid_argument("Cannot find world file");
  }

  xmlNodePtr top = xmlDocGetRootElement(doc);
  if (top == nullptr)
  {
    throw std::invalid_argument("Error parsing world file");
  }

  if (xmlStrcmp(top->name, reinterpret_cast<const xmlChar*>("world")))
  {
    throw std::invalid_argument("Error parsing world file");
  }

  xmlChar* key = nullptr;

  key = getProp(top, "startx");
  startX_ = atoi(reinterpret_cast<char*>(key));
  xmlFree(key);

  key = getProp(top, "starty");
  startY_ = atoi(reinterpret_cast<char*>(key));
  xmlFree(key);

  key = getProp(top, "startmap");
  startMap_ = atoi(reinterpret_cast<char*>(key));
  xmlFree(key);

  for (xmlNodePtr node = top->xmlChildrenNode;
    node != nullptr;
    node = node->next)
  {
    if (!xmlStrcmp(node->name, reinterpret_cast<const xmlChar*>("map")))
    {
      key = getProp(node, "id");
      size_t mapId = atoi(reinterpret_cast<char*>(key));
      xmlFree(key);

      key = getProp(node, "title");
      std::string mapTitle(reinterpret_cast<char*>(key));
      xmlFree(key);

      std::vector<int> mapTiles;

      for (xmlNodePtr mapNode = node->xmlChildrenNode;
        mapNode != nullptr;
        mapNode = mapNode->next)
      {
        if (!xmlStrcmp(
          mapNode->name,
          reinterpret_cast<const xmlChar*>("environment")))
        {
          key = xmlNodeGetContent(mapNode);

          mapTiles.clear();
          mapTiles.push_back(atoi(strtok(reinterpret_cast<char*>(key), ",\n")));
          for (size_t i = 1; i < (MAP_WIDTH * MAP_HEIGHT); i++)
          {
            mapTiles.push_back(atoi(strtok(nullptr, ",\n")));
          }

          xmlFree(key);
        }
      }

      maps_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(mapId),
        std::forward_as_tuple(
          mapId,
          std::move(mapTiles),
          std::move(mapTitle)));
    }
  }

  xmlFreeDoc(doc);
}
