#include "world.h"
#include <stdexcept>
#include <cstring>
#include "consts.h"
#include "xml.h"

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
      Map::Adjacent leftAdj;
      Map::Adjacent rightAdj;
      Map::Adjacent upAdj;
      Map::Adjacent downAdj;
      std::list<Map::Object> objects;

      for (xmlNodePtr mapNode = node->xmlChildrenNode;
        mapNode != nullptr;
        mapNode = mapNode->next)
      {
        if (!xmlStrcmp(
          mapNode->name,
          reinterpret_cast<const xmlChar*>("environment")))
        {
          key = xmlNodeGetContent(mapNode);
          if (key == nullptr)
          {
            throw std::invalid_argument("Error parsing XML file");
          }

          mapTiles.clear();
          mapTiles.push_back(atoi(strtok(reinterpret_cast<char*>(key), ",\n")));
          for (size_t i = 1; i < (MAP_WIDTH * MAP_HEIGHT); i++)
          {
            mapTiles.push_back(atoi(strtok(nullptr, ",\n")));
          }

          xmlFree(key);
        } else if (!xmlStrcmp(
          mapNode->name,
          reinterpret_cast<const xmlChar*>("entity")))
        {
          key = getProp(mapNode, "type");
          std::string entType(reinterpret_cast<char*>(key));
          xmlFree(key);

          key = getProp(mapNode, "x");
          int entX = atoi(reinterpret_cast<char*>(key));
          xmlFree(key);

          key = getProp(mapNode, "y");
          int entY = atoi(reinterpret_cast<char*>(key));
          xmlFree(key);

          key = getProp(mapNode, "index");
          int entIndex = atoi(reinterpret_cast<char*>(key));
          xmlFree(key);

          std::map<std::string, int> items;

          for (xmlNodePtr entityNode = mapNode->xmlChildrenNode;
            entityNode != nullptr;
            entityNode = entityNode->next)
          {
            key = getProp(entityNode, "id");
            std::string itemId(reinterpret_cast<char*>(key));
            xmlFree(key);

            key = xmlNodeGetContent(entityNode);
            if (key == nullptr)
            {
              throw std::invalid_argument("Error parsing XML file");
            }

            items[itemId] = atoi(reinterpret_cast<char*>(key));
            xmlFree(key);
          }

          objects.emplace_back(
            std::move(entType),
            entX,
            entY,
            entIndex,
            std::move(items));
        } else if (!xmlStrcmp(
          mapNode->name,
          reinterpret_cast<const xmlChar*>("adjacent")))
        {
          key = getProp(mapNode, "type");
          std::string adjTypeStr(reinterpret_cast<char*>(key));
          xmlFree(key);

          Map::Adjacent::Type adjType;
          if (adjTypeStr == "wall")
          {
            adjType = Map::Adjacent::Type::wall;
          } else if (adjTypeStr == "wrap")
          {
            adjType = Map::Adjacent::Type::wrap;
          } else if (adjTypeStr == "warp")
          {
            adjType = Map::Adjacent::Type::warp;
          } else if (adjTypeStr == "reverseWarp")
          {
            adjType = Map::Adjacent::Type::reverse;
          } else {
            throw std::logic_error("Invalid adjacency type");
          }

          key = getProp(mapNode, "map");
          int adjMapId = atoi(reinterpret_cast<char*>(key));
          xmlFree(key);

          key = getProp(mapNode, "dir");
          std::string adjDir(reinterpret_cast<char*>(key));
          xmlFree(key);

          if (adjDir == "left")
          {
            leftAdj = {adjType, adjMapId};
          } else if (adjDir == "right")
          {
            rightAdj = {adjType, adjMapId};
          } else if (adjDir == "up")
          {
            upAdj = {adjType, adjMapId};
          } else if (adjDir == "down")
          {
            downAdj = {adjType, adjMapId};
          } else {
            throw std::logic_error("Invalid adjacency direction");
          }
        }
      }

      maps_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(mapId),
        std::forward_as_tuple(
          mapId,
          std::move(mapTiles),
          std::move(mapTitle),
          leftAdj,
          rightAdj,
          upAdj,
          downAdj,
          std::move(objects)));
    }
  }

  xmlFreeDoc(doc);
}
