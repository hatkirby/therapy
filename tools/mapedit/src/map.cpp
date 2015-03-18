#include "map.h"
#include "frame.h"
#include "object.h"
#include "world.h"
#include "consts.h"

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
  objects = map.objects;
  world = map.world;
  treeItemId = map.treeItemId;
  children = map.children;
  hidden = map.hidden;
  leftType = map.leftType;
  rightType = map.rightType;
  upType = map.upType;
  downType = map.downType;
  leftMap = map.leftMap;
  rightMap = map.rightMap;
  downMap = map.downMap;
  upMap = map.upMap;
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
  std::swap(first.objects, second.objects);
  std::swap(first.id, second.id);
  std::swap(first.world, second.world);
  std::swap(first.treeItemId, second.treeItemId);
  std::swap(first.children, second.children);
  std::swap(first.hidden, second.hidden);
  std::swap(first.leftType, second.leftType);
  std::swap(first.rightType, second.rightType);
  std::swap(first.upType, second.upType);
  std::swap(first.downType, second.downType);
  std::swap(first.leftMap, second.leftMap);
  std::swap(first.rightMap, second.rightMap);
  std::swap(first.downMap, second.downMap);
  std::swap(first.upMap, second.upMap);
}

std::list<Map::MoveType> Map::listMoveTypes()
{
  return {MoveType::Wall, MoveType::Wrap, MoveType::Warp, MoveType::ReverseWarp};
}

std::string Map::stringForMoveType(MoveType type)
{
  switch (type)
  {
    case MoveType::Wall: return "Wall";
    case MoveType::Warp: return "Warp";
    case MoveType::Wrap: return "Wrap";
    case MoveType::ReverseWarp: return "Reverse Warp";
  }
}

bool Map::moveTypeTakesMap(MoveType type)
{
  switch (type)
  {
    case MoveType::Wall: return false;
    case MoveType::Wrap: return false;
    case MoveType::Warp: return true;
    case MoveType::ReverseWarp: return true;
  }
}

std::string Map::shortForMoveType(MoveType type)
{
  switch (type)
  {
    case MoveType::Wall: return "wall";
    case MoveType::Wrap: return "wrap";
    case MoveType::Warp: return "warp";
    case MoveType::ReverseWarp: return "reverseWarp";
  }
}

Map::MoveType Map::moveTypeForShort(std::string str)
{
  if (str == "wrap") return MoveType::Wrap;
  if (str == "warp") return MoveType::Warp;
  if (str == "reverseWarp") return MoveType::ReverseWarp;
  
  return MoveType::Wall;
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

World* Map::getWorld() const
{
  return world;
}

bool Map::getHidden() const
{
  return hidden;
}

Map::MoveType Map::getLeftMoveType() const
{
  return leftType;
}

Map::MoveType Map::getRightMoveType() const
{
  return rightType;
}

Map::MoveType Map::getUpMoveType() const
{
  return upType;
}

Map::MoveType Map::getDownMoveType() const
{
  return downType;
}

int Map::getLeftMoveMapID() const
{
  return leftMap;
}

int Map::getRightMoveMapID() const
{
  return rightMap;
}

int Map::getUpMoveMapID() const
{
  return upMap;
}

int Map::getDownMoveMapID() const
{
  return downMap;
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

void Map::addObject(std::shared_ptr<MapObjectEntry> obj, bool dirty)
{
  objects.push_back(obj);
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::removeObject(std::shared_ptr<MapObjectEntry> obj, bool dirty)
{
  objects.remove(obj);
  
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

void Map::setHidden(bool hid)
{
  hidden = hid;
}

void Map::setLeftMoveType(Map::MoveType move, bool dirty)
{
  leftType = move;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setRightMoveType(Map::MoveType move, bool dirty)
{
  rightType = move;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setUpMoveType(Map::MoveType move, bool dirty)
{
  upType = move;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setDownMoveType(Map::MoveType move, bool dirty)
{
  downType = move;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setLeftMoveMapID(int id, bool dirty)
{
  leftMap = id;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setRightMoveMapID(int id, bool dirty)
{
  rightMap = id;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setUpMoveMapID(int id, bool dirty)
{
  upMap = id;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

void Map::setDownMoveMapID(int id, bool dirty)
{
  downMap = id;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

