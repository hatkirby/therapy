#include "map.h"
#include <libxml/parser.h>

Map::Map()
{
  mapdata = (int*) calloc(MAP_WIDTH * MAP_HEIGHT, sizeof(int));
}

Map::Map(const std::string filename)
{
  xmlDocPtr doc = xmlParseFile(filename.c_str());
  if (doc == nullptr)
  {
    throw MapLoadException(filename);
  }
  
  xmlNodePtr top = xmlDocGetRootElement(doc);
  if (top == nullptr)
  {
    throw MapLoadException(filename);
  }
  
  if (xmlStrcmp(top->name, (const xmlChar*) "map-def"))
  {
    throw MapLoadException(filename);
  }
  
  for (xmlNodePtr node = top->xmlChildrenNode; node != NULL; node = node->next)
  {
    if (!xmlStrcmp(node->name, (const xmlChar*) "name"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      title = (char*) key;
      xmlFree(key);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "environment"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      mapdata = (int*) malloc(MAP_WIDTH*MAP_HEIGHT*sizeof(int));
      mapdata[0] = atoi(strtok((char*) key, ",\n"));
      for (int i=1; i<(MAP_WIDTH*MAP_HEIGHT); i++)
      {
        mapdata[i] = atoi(strtok(NULL, ",\n"));
      }
      xmlFree(key);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "leftmap"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      leftmap = (char*) key;
      xmlFree(key);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "rightmap"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      rightmap = (char*) key;
      xmlFree(key);
    }
  }
  
  xmlFreeDoc(doc);
}

Map::Map(const Map& map)
{
  mapdata = (int*) malloc(MAP_WIDTH*MAP_HEIGHT*sizeof(int));
  memcpy(mapdata, map.mapdata, MAP_WIDTH*MAP_HEIGHT*sizeof(int));
  
  title = map.title;
  leftmap = map.leftmap;
  rightmap = map.rightmap;
}

Map::Map(Map&& map) : Map()
{
  swap(*this, map);
}

Map::~Map()
{
  free(mapdata);
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
  std::swap(first.leftmap, second.leftmap);
  std::swap(first.rightmap, second.rightmap);
}