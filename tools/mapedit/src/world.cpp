#include "world.h"
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include "frame.h"
#include <sstream>

World::World()
{
  newMap();
  
  rootChildren.push_back(0);
  empty = true;
}

World::World(std::string filename)
{
  this->filename = filename;
  
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
  
  if (xmlStrcmp(top->name, (const xmlChar*) "world"))
  {
    throw MapLoadException(filename);
  }
  
  for (xmlNodePtr node = top->xmlChildrenNode; node != NULL; node = node->next)
  {
    if (!xmlStrcmp(node->name, (const xmlChar*) "nextmapid"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      if (key != 0)
      {
        nextMapID = atoi((char*) key);
      }
      xmlFree(key);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "lastmap"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      if (key != 0)
      {
        lastmap = atoi((char*) key);
      }
      xmlFree(key);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "root"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      if (key != 0)
      {
        rootChildren.push_back(atoi((char*) key));
      }
      xmlFree(key);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "startpos"))
    {
      xmlChar* idKey = xmlGetProp(node, (xmlChar*) "id");
      if (idKey == 0) throw MapLoadException(filename);
      startingMap = atoi((char*) idKey);
      xmlFree(idKey);
      
      xmlChar* posKey = xmlGetProp(node, (xmlChar*) "pos");
      if (posKey == 0) throw MapLoadException(filename);
      sscanf((char*) posKey, "%d,%d", &startingPosition.first, &startingPosition.second);
      xmlFree(posKey);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "map"))
    {
      xmlChar* idKey = xmlGetProp(node, (xmlChar*) "id");
      if (idKey == 0) throw MapLoadException(filename);
      int id = atoi((char*) idKey);
      xmlFree(idKey);
      
      auto map = std::make_shared<Map>(id, this);
      
      for (xmlNodePtr mapNode = node->xmlChildrenNode; mapNode != NULL; mapNode = mapNode->next)
      {
        if (!xmlStrcmp(mapNode->name, (const xmlChar*) "name"))
        {
          xmlChar* key = xmlNodeListGetString(doc, mapNode->xmlChildrenNode, 1);
          if (key != 0)
          {
            map->setTitle((char*) key, false);
          }
          
          xmlFree(key);
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "environment"))
        {
          xmlChar* key = xmlNodeListGetString(doc, mapNode->xmlChildrenNode, 1);
          int* mapdata = (int*) malloc(MAP_WIDTH*MAP_HEIGHT*sizeof(int));
          mapdata[0] = atoi(strtok((char*) key, ",\n"));
          for (int i=1; i<(MAP_WIDTH*MAP_HEIGHT); i++)
          {
            mapdata[i] = atoi(strtok(NULL, ",\n"));
          }
          map->setMapdata(mapdata, false);
          xmlFree(key);
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "leftmap"))
        {
          xmlChar* key = xmlNodeListGetString(doc, mapNode->xmlChildrenNode, 1);
          if (key != 0)
          {
            map->setLeftmap(atoi((char*) key), false);
          }
          xmlFree(key);
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "rightmap"))
        {
          xmlChar* key = xmlNodeListGetString(doc, mapNode->xmlChildrenNode, 1);
          if (key != 0)
          {
            map->setRightmap(atoi((char*) key));
          }
          xmlFree(key);
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "entities"))
        {
          for (xmlNodePtr entityNode = mapNode->xmlChildrenNode; entityNode != NULL; entityNode = entityNode->next)
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
                  sscanf((char*) key, "%d,%d", &data->position.first, &data->position.second);
                  xmlFree(key);
                }
              }
        
              map->addObject(data, false);
            }
          }
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "child"))
        {
          xmlChar* key = xmlNodeListGetString(doc, mapNode->xmlChildrenNode, 1);
          if (key != 0)
          {
            map->addChild(atoi((char*) key));
          }
          xmlFree(key);
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "expanded"))
        {
          xmlChar* key = xmlNodeListGetString(doc, mapNode->xmlChildrenNode, 1);
          if ((key != 0) && ((char) key[0] == '1'))
          {
            map->setExpanded(true);
          }
        }
      }
      
      maps[map->getID()] = map;
    }
  }
  
  xmlFreeDoc(doc);
}

std::shared_ptr<Map> World::newMap()
{
  auto nm = std::make_shared<Map>(nextMapID++, this);
  maps[nm->getID()] = nm;
  return nm;
}

std::shared_ptr<Map> World::getMap(int id) const
{
  return maps.at(id);
}

void World::setDirty(bool dirty)
{
  if (dirty) empty = false;
  this->dirty = dirty;
  parent->MapDirtyDidChange(dirty);
}

bool World::getDirty() const
{
  return dirty;
}

std::string World::getFilename() const
{
  return filename;
}

void World::setParent(MapeditFrame* parent)
{
  this->parent = parent;
}

Map* World::getLastMap() const
{
  return getMap(lastmap).get();
}

#define MY_ENCODING "ISO-8859-1"

void World::save(std::string name, wxTreeCtrl* mapTree)
{
  int rc;
  
  xmlTextWriterPtr writer = xmlNewTextWriterFilename(name.c_str(), 0);
  if (writer == NULL) throw MapWriteException(name);

  rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);
  if (rc < 0) throw MapWriteException(name);
  
  // <world>
  rc = xmlTextWriterStartElement(writer, (xmlChar*) "world");
  if (rc < 0) throw MapWriteException(name);
  
  //   <nextmapid/>
  std::ostringstream nextMap_out;
  nextMap_out << nextMapID;
  rc = xmlTextWriterWriteElement(writer, (xmlChar*) "nextmapid", (xmlChar*) nextMap_out.str().c_str());
  if (rc < 0) throw MapWriteException(name);
  
  //   <lastmap/>
  std::ostringstream lastMap_out;
  lastMap_out << lastmap;
  rc = xmlTextWriterWriteElement(writer, (xmlChar*) "lastmap", (xmlChar*) lastMap_out.str().c_str());
  if (rc < 0) throw MapWriteException(name);
  
  // ASSUMPTION: There will always be at least one child of the invisible root element. i.e. you cannot delete to zero maps.
  wxTreeItemId root = mapTree->GetRootItem();
  wxTreeItemIdValue cookie1;
  for (wxTreeItemId it = mapTree->GetFirstChild(root, cookie1); it.IsOk(); it = mapTree->GetNextChild(root, cookie1))
  {
    // <root>
    MapPtrCtr* ctl = (MapPtrCtr*) mapTree->GetItemData(it);
    std::ostringstream rootid_out;
    rootid_out << ctl->map->getID();
    rc = xmlTextWriterWriteElement(writer, (xmlChar*) "root", (xmlChar*) rootid_out.str().c_str());
    if (rc < 0) throw MapWriteException(name);
  }
  
  //   <startpos>
  rc = xmlTextWriterStartElement(writer, (xmlChar*) "startpos");
  if (rc < 0) throw MapWriteException(name);
  
  //   id=
  rc = xmlTextWriterWriteFormatAttribute(writer, (xmlChar*) "id", "%d", startingMap);
  if (rc < 0) throw MapWriteException(name);
  
  //   pos=
  rc = xmlTextWriterWriteFormatAttribute(writer, (xmlChar*) "pos", "%d,%d", startingPosition.first, startingPosition.second);
  if (rc < 0) throw MapWriteException(name);
  
  //   </startpos>
  rc = xmlTextWriterEndElement(writer);
  if (rc < 0) throw MapWriteException(name);
  
  for (auto mapPair : maps)
  {
    Map& map = *mapPair.second;
    
    if (map.getHidden()) continue;
    
    // <map>
    rc = xmlTextWriterStartElement(writer, (xmlChar*) "map");
    if (rc < 0) throw MapWriteException(name);
    
    // id=
    std::ostringstream id_out;
    id_out << map.getID();
    rc = xmlTextWriterWriteAttribute(writer, (xmlChar*) "id", (xmlChar*) id_out.str().c_str());
    if (rc < 0) throw MapWriteException(name);
    
    //   <name/>
    rc = xmlTextWriterWriteElement(writer, (xmlChar*) "name", (xmlChar*) map.getTitle().c_str());
    if (rc < 0) throw MapWriteException(name);
  
    //   <environment/>
    std::ostringstream mapdata_out;
    for (int y=0; y<MAP_HEIGHT; y++)
    {
      for (int x=0; x<MAP_WIDTH; x++)
      {
        mapdata_out << map.getTileAt(x,y) << ",";
      }
    
      mapdata_out << std::endl;
    }
  
    rc = xmlTextWriterWriteElement(writer, (xmlChar*) "environment", (xmlChar*) mapdata_out.str().c_str());
    if (rc < 0) throw MapWriteException(name);
  
    //   <leftmap/>
    std::ostringstream leftmap_out;
    if (map.getLeftmap())
    {
      leftmap_out << map.getLeftmap()->getID();
    }
    rc = xmlTextWriterWriteElement(writer, (xmlChar*) "leftmap", (xmlChar*) leftmap_out.str().c_str());
    if (rc < 0) throw MapWriteException(name);

    //   <rightmap/>
    std::ostringstream rightmap_out;
    if (map.getRightmap())
    {
      rightmap_out << map.getRightmap()->getID();
    }
    rc = xmlTextWriterWriteElement(writer, (xmlChar*) "rightmap", (xmlChar*) rightmap_out.str().c_str());
    if (rc < 0) throw MapWriteException(name);
  
    //   <entities>
    rc = xmlTextWriterStartElement(writer, (xmlChar*) "entities");
    if (rc < 0) throw MapWriteException(name);
  
    for (auto object : map.getObjects())
    {
      //   <entity>
      rc = xmlTextWriterStartElement(writer, (xmlChar*) "entity");
      if (rc < 0) throw MapWriteException(name);
    
      //     <entity-type/>
      rc = xmlTextWriterWriteElement(writer, (xmlChar*) "entity-type", (xmlChar*) object->object->getType().c_str());
      if (rc < 0) throw MapWriteException(name);
    
      //     <entity-position/>
      std::ostringstream entpos_out;
      entpos_out << object->position.first << "," << object->position.second;
      rc = xmlTextWriterWriteElement(writer, (xmlChar*) "entity-position", (xmlChar*) entpos_out.str().c_str());
      if (rc < 0) throw MapWriteException(name);
    
      //   </entity>
      rc = xmlTextWriterEndElement(writer);
      if (rc < 0) throw MapWriteException(name);
    }
  
    //   </entities>
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) throw MapWriteException(name);
    
    wxTreeItemId node = map.getTreeItemId();
    if (mapTree->ItemHasChildren(node))
    {
      wxTreeItemIdValue cookie2;
      for (wxTreeItemId it = mapTree->GetFirstChild(node, cookie2); it.IsOk(); it = mapTree->GetNextChild(node, cookie2))
      {
      // <child/>
        MapPtrCtr* ctl = (MapPtrCtr*) mapTree->GetItemData(it);
        std::ostringstream childid_out;
        childid_out << ctl->map->getID();
        rc = xmlTextWriterWriteElement(writer, (xmlChar*) "child", (xmlChar*) childid_out.str().c_str());
        if (rc < 0) throw MapWriteException(name);
      }
      
      if (mapTree->IsExpanded(node))
      {
      // <expanded/>
        rc = xmlTextWriterWriteElement(writer, (xmlChar*) "expanded", (xmlChar*) "1");
        if (rc < 0) throw MapWriteException(name);
      }
    }
    
    // </map>
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) throw MapWriteException(name);
  }
  
  // </world>
  rc = xmlTextWriterEndDocument(writer);
  if (rc < 0) throw MapWriteException(name);
  
  xmlFreeTextWriter(writer);
  
  setDirty(false);
}

std::list<std::shared_ptr<Map>> World::getRootMaps() const
{
  std::list<std::shared_ptr<Map>> ret;
  
  for (auto id : rootChildren)
  {
    ret.push_back(getMap(id));
  }
  
  return ret;
}

const std::map<int, std::shared_ptr<Map>> World::getMaps() const
{
  return maps;
}

void World::setLastMap(Map* map)
{
  lastmap = map->getID();
}

bool World::getEmpty() const
{
  return empty;
}

Map* World::getStartingMap() const
{
  return getMap(startingMap).get();
}

std::pair<double, double> World::getStartingPosition() const
{
  return startingPosition;
}

void World::setStart(Map* map, std::pair<double, double> startPos)
{
  startingMap = map->getID();
  startingPosition = startPos;
  
  setDirty(true);
}
