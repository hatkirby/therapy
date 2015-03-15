#include "map.h"
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <sstream>

Map::Map()
{
  mapdata = (int*) calloc(MAP_WIDTH * MAP_HEIGHT, sizeof(int));
  dirty = true;
}

Map::Map(std::string filename)
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
  
  dirty = false;
}

Map::Map(const Map& map)
{
  mapdata = (int*) malloc(MAP_WIDTH*MAP_HEIGHT*sizeof(int));
  memcpy(mapdata, map.mapdata, MAP_WIDTH*MAP_HEIGHT*sizeof(int));
  
  title = map.title;
  leftmap = map.leftmap;
  rightmap = map.rightmap;
  dirty = map.dirty;
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
  std::swap(first.dirty, second.dirty);
}

#define MY_ENCODING "ISO-8859-1"

void Map::save(std::string name)
{
  if (!dirty) return;
  
  int rc;
  
  xmlTextWriterPtr writer = xmlNewTextWriterFilename(name.c_str(), 0);
  if (writer == NULL) throw MapWriteException(name);

  rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);
  if (rc < 0) throw MapWriteException(name);
  
  rc = xmlTextWriterStartElement(writer, (xmlChar*) "map-def");
  if (rc < 0) throw MapWriteException(name);
  
  rc = xmlTextWriterWriteElement(writer, (xmlChar*) "name", (xmlChar*) title.c_str());
  if (rc < 0) throw MapWriteException(name);
  
  std::ostringstream mapdata_out;
  for (int y=0; y<MAP_HEIGHT; y++)
  {
    for (int x=0; x<MAP_WIDTH; x++)
    {
      mapdata_out << mapdata[x+y*MAP_WIDTH] << ",";
    }
    
    mapdata_out << std::endl;
  }
  
  rc = xmlTextWriterWriteElement(writer, (xmlChar*) "environment", (xmlChar*) mapdata_out.str().c_str());
  if (rc < 0) throw MapWriteException(name);
  
  if (leftmap != "")
  {
    rc = xmlTextWriterWriteElement(writer, (xmlChar*) "leftmap", (xmlChar*) leftmap.c_str());
    if (rc < 0) throw MapWriteException(name);
  }
  
  if (rightmap != "")
  {
    rc = xmlTextWriterWriteElement(writer, (xmlChar*) "rightmap", (xmlChar*) rightmap.c_str());
    if (rc < 0) throw MapWriteException(name);
  }
  
  rc = xmlTextWriterEndElement(writer);
  if (rc < 0) throw MapWriteException(name);
  
  rc = xmlTextWriterEndDocument(writer);
  if (rc < 0) throw MapWriteException(name);
  
  xmlFreeTextWriter(writer);
  
  dirty = false;
}

bool Map::hasUnsavedChanges() const
{
  return dirty;
}

void Map::setTileAt(int x, int y, int tile)
{
  dirty = true;
  mapdata[x+y*MAP_WIDTH] = tile;
}

int Map::getTileAt(int x, int y) const
{
  return mapdata[x+y*MAP_WIDTH];
}
