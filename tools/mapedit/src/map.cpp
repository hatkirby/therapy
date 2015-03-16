#include "map.h"
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <sstream>
#include "frame.h"

Map::Map()
{
  mapdata = (int*) calloc(MAP_WIDTH * MAP_HEIGHT, sizeof(int));
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
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "entities"))
    {
      for (xmlNodePtr entityNode = node->xmlChildrenNode; entityNode != NULL; entityNode = entityNode->next)
      {
        if (!xmlStrcmp(entityNode->name, (const xmlChar*) "entity"))
        {
          auto data = std::make_shared<MapObjectEntry>();
          
          for (xmlNodePtr entityDataNode = entityNode->xmlChildrenNode; entityDataNode != NULL; entityDataNode = entityDataNode->next)
          {
            if (!xmlStrcmp(entityDataNode->name, (const xmlChar*) "entity-type"))
            {
              xmlChar* key = xmlNodeListGetString(doc, entityDataNode->xmlChildrenNode, 1);
              data->object = MapObject::getAllObjects().at((char*) key).get();
              xmlFree(key);
            } else if (!xmlStrcmp(entityDataNode->name, (const xmlChar*) "entity-position"))
            {
              xmlChar* key = xmlNodeListGetString(doc, entityDataNode->xmlChildrenNode, 1);
              sscanf((char*) key, "%lf,%lf", &data->position.first, &data->position.second);
              xmlFree(key);
            }
          }
        
          objects.push_back(data);
        }
      }
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
  dirty = map.dirty;
  objects = map.objects;
  frame = map.frame;
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
  std::swap(first.objects, second.objects);
  std::swap(first.frame, second.frame);
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
  
  rc = xmlTextWriterWriteElement(writer, (xmlChar*) "leftmap", (xmlChar*) leftmap.c_str());
  if (rc < 0) throw MapWriteException(name);

  rc = xmlTextWriterWriteElement(writer, (xmlChar*) "rightmap", (xmlChar*) rightmap.c_str());
  if (rc < 0) throw MapWriteException(name);
  
  rc = xmlTextWriterStartElement(writer, (xmlChar*) "entities");
  if (rc < 0) throw MapWriteException(name);
  
  for (auto object : objects)
  {
    rc = xmlTextWriterStartElement(writer, (xmlChar*) "entity");
    if (rc < 0) throw MapWriteException(name);
    
    rc = xmlTextWriterWriteElement(writer, (xmlChar*) "entity-type", (xmlChar*) object->object->getType().c_str());
    if (rc < 0) throw MapWriteException(name);
    
    std::ostringstream entpos_out;
    entpos_out << object->position.first << "," << object->position.second;
    
    rc = xmlTextWriterWriteElement(writer, (xmlChar*) "entity-position", (xmlChar*) entpos_out.str().c_str());
    if (rc < 0) throw MapWriteException(name);
    
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) throw MapWriteException(name);
  }
  
  rc = xmlTextWriterEndElement(writer);
  if (rc < 0) throw MapWriteException(name);
  
  rc = xmlTextWriterEndElement(writer);
  if (rc < 0) throw MapWriteException(name);
  
  rc = xmlTextWriterEndDocument(writer);
  if (rc < 0) throw MapWriteException(name);
  
  xmlFreeTextWriter(writer);
  
  setDirty(false);
}

bool Map::hasUnsavedChanges() const
{
  return dirty;
}

void Map::setTileAt(int x, int y, int tile)
{
  setDirty(true);
  mapdata[x+y*MAP_WIDTH] = tile;
}

int Map::getTileAt(int x, int y) const
{
  return mapdata[x+y*MAP_WIDTH];
}

std::string Map::getTitle() const
{
  return title;
}

void Map::setTitle(std::string title)
{
  setDirty(true);
  this->title = title;
}

const std::list<std::shared_ptr<MapObjectEntry>>& Map::getObjects() const
{
  return objects;
}

void Map::addObject(std::shared_ptr<MapObjectEntry>& obj)
{
  setDirty(true);
  objects.push_back(obj);
}

void Map::removeObject(std::shared_ptr<MapObjectEntry>& obj)
{
  setDirty(true);
  objects.remove(obj);
}

bool Map::getDirty() const
{
  return dirty;
}

void Map::setDirty(bool dirty)
{
  this->dirty = dirty;
  
  if (frame != nullptr)
  {
    frame->MapDirtyDidChange(dirty);
  }
}
