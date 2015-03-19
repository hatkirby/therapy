#include "world.h"
#include <libxml/parser.h>
#include "consts.h"

World::World(const char* filename)
{
  xmlDocPtr doc = xmlParseFile(filename);
  if (doc == nullptr)
  {
    exit(2);
  }
  
  xmlNodePtr top = xmlDocGetRootElement(doc);
  if (top == nullptr)
  {
    exit(2);
  }
  
  if (xmlStrcmp(top->name, (const xmlChar*) "world"))
  {
    exit(2);
  }
  
  xmlChar* startxKey = xmlGetProp(top, (xmlChar*) "startx");
  if (startxKey == 0) exit(2);
  startX = atoi((char*) startxKey);
  xmlFree(startxKey);
  
  xmlChar* startyKey = xmlGetProp(top, (xmlChar*) "starty");
  if (startyKey == 0) exit(2);
  startY = atoi((char*) startyKey);
  xmlFree(startyKey);
  
  xmlChar* startmapKey = xmlGetProp(top, (xmlChar*) "startmap");
  if (startxKey == 0) exit(2);
  startMap = atoi((char*) startmapKey);
  xmlFree(startmapKey);
  
  for (xmlNodePtr node = top->xmlChildrenNode; node != NULL; node = node->next)
  {
    if (!xmlStrcmp(node->name, (const xmlChar*) "map"))
    {
      xmlChar* idKey = xmlGetProp(node, (xmlChar*) "id");
      if (idKey == 0) exit(2);
      int theId = atoi((char*) idKey);
      xmlFree(idKey);
      
      maps.emplace(theId, theId);
      Map& map = maps[theId];
      
      xmlChar* titleKey = xmlGetProp(node, (xmlChar*) "title");
      if (titleKey == 0) exit(2);
      map.setTitle((char*) titleKey);
      xmlFree(titleKey);
      
      for (xmlNodePtr mapNode = node->xmlChildrenNode; mapNode != NULL; mapNode = mapNode->next)
      {
        if (!xmlStrcmp(mapNode->name, (const xmlChar*) "environment"))
        {
          xmlChar* key = xmlNodeGetContent(mapNode);
          int* mapdata = (int*) malloc(MAP_WIDTH*MAP_HEIGHT*sizeof(int));
          mapdata[0] = atoi(strtok((char*) key, ",\n"));
          for (int i=1; i<(MAP_WIDTH*MAP_HEIGHT); i++)
          {
            mapdata[i] = atoi(strtok(NULL, ",\n"));
          }
          map.setMapdata(mapdata);
          xmlFree(key);
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "entity"))
        {
          Map::EntityData data;
          
          xmlChar* typeKey = xmlGetProp(mapNode, (const xmlChar*) "type");
          if (typeKey == 0) exit(2);
          data.name = (char*) typeKey;
          xmlFree(typeKey);
          
          xmlChar* xKey = xmlGetProp(mapNode, (const xmlChar*) "x");
          if (xKey == 0) exit(2);
          data.position.first = atoi((char*) xKey);
          xmlFree(xKey);
          
          xmlChar* yKey = xmlGetProp(mapNode, (const xmlChar*) "y");
          if (yKey == 0) exit(2);
          data.position.second = atoi((char*) yKey);
          xmlFree(yKey);
          
          for (xmlNodePtr entityNode = mapNode->xmlChildrenNode; entityNode != NULL; entityNode = entityNode->next)
          {
            if (!xmlStrcmp(entityNode->name, (xmlChar*) "item"))
            {
              xmlChar* itemIdKey = xmlGetProp(entityNode, (const xmlChar*) "id");
              if (itemIdKey == 0) exit(2);
              std::string itemId = (char*) itemIdKey;
              xmlFree(itemIdKey);
              
              xmlChar* itemIdVal = xmlNodeGetContent(entityNode);
              if (itemIdVal == 0) exit(2);
              data.items[itemId] = atoi((char*) itemIdVal);
              xmlFree(itemIdVal);
            }
          }
          
          map.addEntity(data);
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "adjacent"))
        {
          Map::MoveDir direction;
          Map::MoveType moveType;
          int mapId = 0;
          
          xmlChar* dirKey = xmlGetProp(mapNode, (const xmlChar*) "dir");
          if (dirKey == 0) exit(2);
          direction = Map::moveDirForShort((char*) dirKey);
          xmlFree(dirKey);
          
          xmlChar* typeKey = xmlGetProp(mapNode, (const xmlChar*) "type");
          if (typeKey == 0) exit(2);
          moveType = Map::moveTypeForShort((char*) typeKey);
          xmlFree(typeKey);
          
          xmlChar* mapIdKey = xmlGetProp(mapNode, (const xmlChar*) "map");
          if (mapIdKey != 0)
          {
            mapId = atoi((char*) mapIdKey);
          }
          xmlFree(mapIdKey);
          
          map.setAdjacent(direction, moveType, mapId);
        }
      }
    }
  }
  
  xmlFreeDoc(doc);
}

const Map& World::getMap(int id) const
{
  return maps.at(id);
}

const Map& World::getStartingMap() const
{
  return maps.at(startMap);
}

std::pair<int, int> World::getStartingPosition() const
{
  return std::make_pair(startX, startY);
}
