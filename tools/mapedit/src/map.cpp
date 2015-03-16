#include "map.h"
#include "frame.h"

Map::Map(int id, World* world) : id(id), world(world)
{
  mapdata = (int*) calloc(MAP_WIDTH * MAP_HEIGHT, sizeof(int));
}

Map::Map(const Map& map)
{
  mapdata = (int*) malloc(MAP_WIDTH*MAP_HEIGHT*sizeof(int));
  memcpy(mapdata, map.mapdata, MAP_WIDTH*MAP_HEIGHT*sizeof(int));
  
  id = map.id;
  title = map.title;
  leftmap = map.leftmap;
  rightmap = map.rightmap;
  objects = map.objects;
  world = map.world;
  treeItemId = map.treeItemId;
  children = map.children;
}

Map::Map(Map&& map) : Map(-1, map.world)
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
  std::swap(first.objects, second.objects);
  std::swap(first.id, second.id);
  std::swap(first.world, second.world);
  std::swap(first.treeItemId, second.treeItemId);
  std::swap(first.children, second.children);
}

int Map::getID() const
{
  return id;
}

std::string Map::getTitle() const
{
  return title;
}

int Map::getTileAt(int x, int y) const
{
  return mapdata[x+y*MAP_WIDTH];
}

const std::list<std::shared_ptr<MapObjectEntry>>& Map::getObjects() const
{
  return objects;
}

std::shared_ptr<Map> Map::getLeftmap() const
{
  if (leftmap == -1)
  {
    return std::shared_ptr<Map>();
  } else {
    return world->getMap(leftmap);
  }
}

std::shared_ptr<Map> Map::getRightmap() const
{
  if (rightmap == -1)
  {
    return std::shared_ptr<Map>();
  } else {
    return world->getMap(rightmap);
  }
}

wxTreeItemId Map::getTreeItemId() const
{
  return treeItemId;
}

std::list<std::shared_ptr<Map>> Map::getChildren() const
{
  std::list<std::shared_ptr<Map>> ret;
  
  for (auto id : children)
  {
    ret.push_back(world->getMap(id));
  }
  
  return ret;
}

bool Map::getExpanded() const
{
  return expanded;
}

void Map::setTitle(std::string title, bool dirty)
{
  this->title = title;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setTileAt(int x, int y, int tile, bool dirty)
{
  mapdata[x+y*MAP_WIDTH] = tile;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setMapdata(int* mapdata, bool dirty)
{
  free(this->mapdata);
  this->mapdata = mapdata;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::addObject(std::shared_ptr<MapObjectEntry>& obj, bool dirty)
{
  objects.push_back(obj);
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::removeObject(std::shared_ptr<MapObjectEntry>& obj, bool dirty)
{
  objects.remove(obj);
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setLeftmap(int id, bool dirty)
{
  leftmap = id;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setRightmap(int id, bool dirty)
{
  rightmap = id;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setTreeItemId(wxTreeItemId id)
{
  this->treeItemId = id;
}

void Map::addChild(int id)
{
  children.push_back(id);
}

void Map::setExpanded(bool exp)
{
  expanded = exp;
}
