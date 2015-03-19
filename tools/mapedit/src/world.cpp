#include "world.h"
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <sstream>
#include "frame.h"
#include "map.h"
#include "object.h"
#include "consts.h"

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
    throw MapLoadException("file not found");
  }
  
  xmlNodePtr top = xmlDocGetRootElement(doc);
  if (top == nullptr)
  {
    throw MapLoadException("no root element");
  }
  
  if (xmlStrcmp(top->name, (const xmlChar*) "world"))
  {
    throw MapLoadException("no world element");
  }
  
  xmlChar* nextmapKey = xmlGetProp(top, (xmlChar*) "nextmap");
  if (nextmapKey != 0)
  {
    nextMapID = atoi((char*) nextmapKey);
  }
  xmlFree(nextmapKey);
  
  xmlChar* lastmapKey = xmlGetProp(top, (xmlChar*) "lastmap");
  if (lastmapKey != 0)
  {
    lastmap = atoi((char*) lastmapKey);
  }
  xmlFree(lastmapKey);
  
  xmlChar* startxKey = xmlGetProp(top, (xmlChar*) "startx");
  if (startxKey == 0) throw MapLoadException("world missing startx attribute");
  startingPosition.first = atoi((char*) startxKey);
  xmlFree(startxKey);
  
  xmlChar* startyKey = xmlGetProp(top, (xmlChar*) "starty");
  if (startyKey == 0) throw MapLoadException("world missing starty attribute");
  startingPosition.second = atoi((char*) startyKey);
  xmlFree(startyKey);
  
  xmlChar* startmapKey = xmlGetProp(top, (xmlChar*) "startmap");
  if (startxKey == 0) throw MapLoadException("world missing startmap attribute");
  startingMap = atoi((char*) startmapKey);
  xmlFree(startmapKey);
  
  for (xmlNodePtr node = top->xmlChildrenNode; node != NULL; node = node->next)
  {
    if (!xmlStrcmp(node->name, (const xmlChar*) "root"))
    {
      xmlChar* key = xmlNodeGetContent(node);
      if (key == 0) throw MapLoadException("root missing content");
      rootChildren.push_back(atoi((char*) key));
      xmlFree(key);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "map"))
    {
      xmlChar* idKey = xmlGetProp(node, (xmlChar*) "id");
      if (idKey == 0) throw MapLoadException("map missing id attribute");
      int id = atoi((char*) idKey);
      xmlFree(idKey);
      
      auto map = std::make_shared<Map>(id, this);
      
      xmlChar* expandKey = xmlGetProp(node, (xmlChar*) "expanded");
      if ((expandKey != 0) && (!xmlStrcmp(expandKey, (const xmlChar*) "true")))
      {
        map->setExpanded(true);
      }
      xmlFree(expandKey);
      
      xmlChar* titleKey = xmlGetProp(node, (xmlChar*) "title");
      if (titleKey == 0) throw MapLoadException("map missing title attribute");
      map->setTitle((char*) titleKey, false);
      xmlFree(titleKey);
      
      for (xmlNodePtr mapNode = node->xmlChildrenNode; mapNode != NULL; mapNode = mapNode->next)
      {
        if (!xmlStrcmp(mapNode->name, (const xmlChar*) "environment"))
        {
          xmlChar* key = xmlNodeGetContent(mapNode);
          if (key == 0) throw MapLoadException("map missing environment content");
          int* mapdata = (int*) malloc(MAP_WIDTH*MAP_HEIGHT*sizeof(int));
          mapdata[0] = atoi(strtok((char*) key, ",\n"));
          for (int i=1; i<(MAP_WIDTH*MAP_HEIGHT); i++)
          {
            mapdata[i] = atoi(strtok(NULL, ",\n"));
          }
          map->setMapdata(mapdata, false);
          xmlFree(key);
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "entity"))
        {
          xmlChar* typeKey = xmlGetProp(mapNode, (const xmlChar*) "type");
          if (typeKey == 0) throw MapLoadException("entity missing type attribute");
          const MapObject& obj = MapObject::getAllObjects().at((char*) typeKey);
          xmlFree(typeKey);
          
          xmlChar* xKey = xmlGetProp(mapNode, (const xmlChar*) "x");
          if (xKey == 0) throw MapLoadException("entity missing x attribute");
          int xpos = atoi((char*) xKey);
          xmlFree(xKey);
          
          xmlChar* yKey = xmlGetProp(mapNode, (const xmlChar*) "y");
          if (yKey == 0) throw MapLoadException("entity missing y attribute");
          int ypos = atoi((char*) yKey);
          xmlFree(yKey);
          
          auto data = std::make_shared<MapObjectEntry>(obj, xpos, ypos);
          
          map->addObject(data, false);
          
          for (xmlNodePtr objectNode = mapNode->xmlChildrenNode; objectNode != NULL; objectNode = objectNode->next)
          {
            if (!xmlStrcmp(objectNode->name, (const xmlChar*) "item"))
            {
              xmlChar* key = xmlGetProp(objectNode, (const xmlChar*) "id");
              if (key == 0) throw MapLoadException("item missing id attribute");
              std::string itemID = (char*) key;
              xmlFree(key);
              
              MapObjectEntry::Item item;
              item.type = data->getObject().getInput(itemID).type;
              
              key = xmlNodeGetContent(objectNode);
              if (key == 0) throw MapLoadException("item missing content");
              switch (item.type)
              {
                case MapObject::Input::Type::Choice:
                case MapObject::Input::Type::Slider:
                {
                  item.intvalue = atoi((char*) key);
                  break;
                }
              }
              
              data->addItem(itemID, item);
            }
          }
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "adjacent"))
        {
          Map::MoveDir direction;
          Map::MoveType moveType;
          int mapId = 0;
          
          xmlChar* dirKey = xmlGetProp(mapNode, (const xmlChar*) "dir");
          if (dirKey == 0) throw MapLoadException("adjacent missing dir attribute");
          direction = Map::moveDirForShort((char*) dirKey);
          xmlFree(dirKey);
          
          xmlChar* typeKey = xmlGetProp(mapNode, (const xmlChar*) "type");
          if (typeKey == 0) throw MapLoadException("adjacent missing type attribute");
          moveType = Map::moveTypeForShort((char*) typeKey);
          xmlFree(typeKey);
          
          xmlChar* mapIdKey = xmlGetProp(mapNode, (const xmlChar*) "map");
          if (mapIdKey != 0)
          {
            mapId = atoi((char*) mapIdKey);
          }
          xmlFree(mapIdKey);
          
          map->setAdjacent(direction, moveType, mapId, false);
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "child"))
        {
          xmlChar* key = xmlNodeGetContent(mapNode);
          if (key != 0)
          {
            map->addChild(atoi((char*) key));
          }
          xmlFree(key);
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
  
  //   nextmap=
  rc = xmlTextWriterWriteFormatAttribute(writer, (xmlChar*) "nextmap", "%d", nextMapID);
  if (rc < 0) throw MapWriteException(name);
  
  //   lastmap=
  rc = xmlTextWriterWriteFormatAttribute(writer, (xmlChar*) "lastmap", "%d", lastmap);
  if (rc < 0) throw MapWriteException(name);
  
  //   startx=
  rc = xmlTextWriterWriteFormatAttribute(writer, (xmlChar*) "startx", "%d", startingPosition.first);
  if (rc < 0) throw MapWriteException(name);
  
  //   starty=
  rc = xmlTextWriterWriteFormatAttribute(writer, (xmlChar*) "starty", "%d", startingPosition.second);
  if (rc < 0) throw MapWriteException(name);
  
  //   startmap=
  rc = xmlTextWriterWriteFormatAttribute(writer, (xmlChar*) "startmap", "%d", startingMap);
  if (rc < 0) throw MapWriteException(name);
  
  // ASSUMPTION: There will always be at least one child of the invisible root element. i.e. you cannot delete to zero maps.
  wxTreeItemId root = mapTree->GetRootItem();
  wxTreeItemIdValue cookie1;
  for (wxTreeItemId it = mapTree->GetFirstChild(root, cookie1); it.IsOk(); it = mapTree->GetNextChild(root, cookie1))
  {
    // <root>
    MapPtrCtr* ctl = (MapPtrCtr*) mapTree->GetItemData(it);
    rc = xmlTextWriterWriteFormatElement(writer, (xmlChar*) "root", "%d", ctl->map->getID());
    if (rc < 0) throw MapWriteException(name);
  }
  
  for (auto mapPair : maps)
  {
    Map& map = *mapPair.second;
    
    if (map.getHidden()) continue;
    
    // <map>
    rc = xmlTextWriterStartElement(writer, (xmlChar*) "map");
    if (rc < 0) throw MapWriteException(name);
    
    // id=
    rc = xmlTextWriterWriteFormatAttribute(writer, (xmlChar*) "id", "%d", map.getID());
    if (rc < 0) throw MapWriteException(name);
    
    // expanded=
    wxTreeItemId node = map.getTreeItemId();
    if (mapTree->IsExpanded(node))
    {
      rc = xmlTextWriterWriteAttribute(writer, (xmlChar*) "expanded", (xmlChar*) "true");
      if (rc < 0) throw MapWriteException(name);
    } else {
      rc = xmlTextWriterWriteAttribute(writer, (xmlChar*) "expanded", (xmlChar*) "false");
      if (rc < 0) throw MapWriteException(name);
    }
    
    // title=
    rc = xmlTextWriterWriteAttribute(writer, (xmlChar*) "title", (xmlChar*) map.getTitle().c_str());
    if (rc < 0) throw MapWriteException(name);
  
    //   <environment
    rc = xmlTextWriterStartElement(writer, (xmlChar*) "environment");
    if (rc < 0) throw MapWriteException(name);
    
    //   type=
    rc = xmlTextWriterWriteAttribute(writer, (xmlChar*) "type", (xmlChar*) "0");
    if (rc < 0) throw MapWriteException(name);
    
    //   >
    std::ostringstream mapdata_out;
    for (int y=0; y<MAP_HEIGHT; y++)
    {
      for (int x=0; x<MAP_WIDTH; x++)
      {
        mapdata_out << map.getTileAt(x,y) << ",";
      }
    
      mapdata_out << std::endl;
    }
  
    rc = xmlTextWriterWriteString(writer, (xmlChar*) mapdata_out.str().c_str());
    if (rc < 0) throw MapWriteException(name);
    
    //   </environment>
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) throw MapWriteException(name);
    
    for (auto object : map.getObjects())
    {
      // <entity>
      rc = xmlTextWriterStartElement(writer, (xmlChar*) "entity");
      if (rc < 0) throw MapWriteException(name);
    
      // type=
      rc = xmlTextWriterWriteAttribute(writer, (xmlChar*) "type", (xmlChar*) object->getObject().getID().c_str());
      if (rc < 0) throw MapWriteException(name);
    
      // x=
      rc = xmlTextWriterWriteFormatAttribute(writer, (xmlChar*) "x", "%d", object->getPosition().first);
      if (rc < 0) throw MapWriteException(name);
      
      // y=
      rc = xmlTextWriterWriteFormatAttribute(writer, (xmlChar*) "y", "%d", object->getPosition().second);
      if (rc < 0) throw MapWriteException(name);
      
      for (auto item : object->getItems())
      {
        // <item
        rc = xmlTextWriterStartElement(writer, (xmlChar*) "item");
        if (rc < 0) throw MapWriteException(name);
        
        // id=
        rc = xmlTextWriterWriteAttribute(writer, (xmlChar*) "id", (xmlChar*) item.first.c_str());
        if (rc < 0) throw MapWriteException(name);
        
        // >
        switch (item.second.type)
        {
          case MapObject::Input::Type::Slider:
          case MapObject::Input::Type::Choice:
          {
            rc = xmlTextWriterWriteFormatString(writer, "%d", item.second.intvalue);
            break;
          }
        }
        
        // </item>
        rc = xmlTextWriterEndElement(writer);
        if (rc < 0) throw MapWriteException(name);
      }
    
      // </entity>
      rc = xmlTextWriterEndElement(writer);
      if (rc < 0) throw MapWriteException(name);
    }
    
    for (auto adjacent : map.getAdjacents())
    {
      // <adjacent>
      rc = xmlTextWriterStartElement(writer, (xmlChar*) "adjacent");
      if (rc < 0) throw MapWriteException(name);
      
      // dir=
      rc = xmlTextWriterWriteAttribute(writer, (xmlChar*) "dir", (xmlChar*) Map::shortForMoveDir(adjacent.first).c_str());
      if (rc < 0) throw MapWriteException(name);
      
      // type=
      rc = xmlTextWriterWriteAttribute(writer, (xmlChar*) "type", (xmlChar*) Map::shortForMoveType(adjacent.second.type).c_str());
      if (rc < 0) throw MapWriteException(name);
      
      // map=
      if (Map::moveTypeTakesMap(adjacent.second.type))
      {
        rc = xmlTextWriterWriteFormatAttribute(writer, (xmlChar*) "map", "%d", adjacent.second.map);
        if (rc < 0) throw MapWriteException(name);
      }
      
      // </adjacent>
      rc = xmlTextWriterEndElement(writer);
      if (rc < 0) throw MapWriteException(name);
    }
    
    if (mapTree->ItemHasChildren(node))
    {
      wxTreeItemIdValue cookie2;
      for (wxTreeItemId it = mapTree->GetFirstChild(node, cookie2); it.IsOk(); it = mapTree->GetNextChild(node, cookie2))
      {
      // <child/>
        MapPtrCtr* ctl = (MapPtrCtr*) mapTree->GetItemData(it);
        rc = xmlTextWriterWriteFormatElement(writer, (xmlChar*) "child", "%d", ctl->map->getID());
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
