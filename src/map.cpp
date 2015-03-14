#include "map.h"
#include "game.h"
#include <cstdlib>
#include <cstring>
#include <libxml/parser.h>
#include <map>
#include "entityfactory.h"
#include "entity.h"

static std::map<std::string, Map> maps;

Map::Map()
{
  title = (char*) calloc(1, sizeof(char));
  mapdata = (int*) calloc(1, sizeof(int));
}

Map::Map(const std::string name)
{
  this->name = name;
  
  xmlDocPtr doc = xmlParseFile(("../maps/" + name + ".xml").c_str());
  if (doc == nullptr)
  {
    fprintf(stderr, "Error reading map %s\n", name.c_str());
    exit(-1);
  }
  
  xmlNodePtr top = xmlDocGetRootElement(doc);
  if (top == nullptr)
  {
    fprintf(stderr, "Empty map %s\n", name.c_str());
    exit(-1);
  }
  
  if (xmlStrcmp(top->name, (const xmlChar*) "map-def"))
  {
    fprintf(stderr, "Invalid map definition %s\n", name.c_str());
    exit(-1);
  }
  
  for (xmlNodePtr node = top->xmlChildrenNode; node != NULL; node = node->next)
  {
    if (!xmlStrcmp(node->name, (const xmlChar*) "name"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      title = (char*) calloc(xmlStrlen(key) + 1, sizeof(char));
      strcpy(title, (char*) key);
      xmlFree(key);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "environment"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      mapdata = (int*) malloc(MAP_WIDTH*(MAP_HEIGHT-1)*sizeof(int));
      mapdata[0] = atoi(strtok((char*) key, ",\n"));
      for (int i=1; i<(MAP_WIDTH*(MAP_HEIGHT-1)); i++)
      {
        mapdata[i] = atoi(strtok(NULL, ",\n"));
      }
      xmlFree(key);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "entities"))
    {
      for (xmlNodePtr entityNode = node->xmlChildrenNode; entityNode != NULL; entityNode = entityNode->next)
      {
        if (!xmlStrcmp(entityNode->name, (const xmlChar*) "entity"))
        {
          EntityData data;
          for (xmlNodePtr entityDataNode = entityNode->xmlChildrenNode; entityDataNode != NULL; entityDataNode = entityDataNode->next)
          {
            if (!xmlStrcmp(entityDataNode->name, (const xmlChar*) "entity-type"))
            {
              xmlChar* key = xmlNodeListGetString(doc, entityDataNode->xmlChildrenNode, 1);
              data.name = std::string((char*) key);
              xmlFree(key);
            } else if (!xmlStrcmp(entityDataNode->name, (const xmlChar*) "entity-position"))
            {
              xmlChar* key = xmlNodeListGetString(doc, entityDataNode->xmlChildrenNode, 1);
              sscanf((char*) key, "%lf,%lf", &(data.position.first), &(data.position.second));
              xmlFree(key);
            }
          }
        
          entities.push_back(data);
        }
      }
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "leftmap"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      leftMap = &Map::getNamedMap(std::string((char*) key));
      xmlFree(key);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "rightmap"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      rightMap = &Map::getNamedMap(std::string((char*) key));
      xmlFree(key);
    }
  }
  
  xmlFreeDoc(doc);
}

Map::Map(const Map& map)
{
  mapdata = (int*) malloc(MAP_WIDTH*(MAP_HEIGHT-1)*sizeof(int));
  memcpy(mapdata, map.mapdata, MAP_WIDTH*(MAP_HEIGHT-1)*sizeof(int));
  
  title = (char*) malloc((MAP_WIDTH+1)*sizeof(char));
  strncpy(title, map.title, MAP_WIDTH+1);
  
  leftMap = map.leftMap;
  rightMap = map.rightMap;
  
  entities = map.entities;
  
  name = map.name;
}

Map::Map(Map&& map) : Map()
{
  swap(*this, map);
}

Map::~Map()
{
  free(mapdata);
  free(title);
}

Map& Map::operator= (Map map)
{
  swap(*this, map);
  
  return *this;
}

void swap(Map& first, Map& second)
{
  std::swap(first.mapdata, second.mapdata);
  std::swap(first.title, second.title);
  std::swap(first.leftMap, second.leftMap);
  std::swap(first.rightMap, second.rightMap);
  std::swap(first.entities, second.entities);
  std::swap(first.name, second.name);
}

const int* Map::getMapdata() const
{
  return mapdata;
}

const char* Map::getTitle() const
{
  return title;
}

const Map* Map::getLeftMap() const
{
  return leftMap;
}

const Map* Map::getRightMap() const
{
  return rightMap;
}

void Map::setLeftMap(const Map* m)
{
  leftMap = m;
}

void Map::setRightMap(const Map* m)
{
  rightMap = m;
}

void Map::createEntities(std::list<std::shared_ptr<Entity>>& entities) const
{
  for (auto data : this->entities)
  {
    auto entity = EntityFactory::createNamedEntity(data.name, *this);
    entity->position = data.position;
    
    entities.push_back(entity);
  }
}

bool Map::operator==(const Map& other) const
{
  return name == other.name;
}

bool Map::operator!=(const Map& other) const
{
  return name != other.name;
}

Map& Map::getNamedMap(const std::string name)
{
  if (maps.count(name) == 0)
  {
    maps[name] = Map {name};
  }
  
  return maps[name];
}
