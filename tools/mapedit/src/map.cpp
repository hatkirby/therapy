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
  adjacents = map.adjacents;
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
  std::swap(first.adjacents, second.adjacents);
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
    case MoveType::Warp: return "Teleport";
    case MoveType::Wrap: return "Wrap";
    case MoveType::ReverseWarp: return "Reverse Teleport";
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

std::string Map::shortForMoveDir(MoveDir dir)
{
  switch (dir)
  {
    case MoveDir::Left: return "left";
    case MoveDir::Right: return "right";
    case MoveDir::Up: return "up";
    case MoveDir::Down: return "down";
  }
}

Map::MoveType Map::moveTypeForShort(std::string str)
{
  if (str == "wrap") return MoveType::Wrap;
  if (str == "warp") return MoveType::Warp;
  if (str == "reverseWarp") return MoveType::ReverseWarp;
  
  return MoveType::Wall;
}

Map::MoveDir Map::moveDirForShort(std::string str)
{
  if (str == "right") return MoveDir::Right;
  if (str == "up") return MoveDir::Up;
  if (str == "down") return MoveDir::Down;
  
  return MoveDir::Left;
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

const std::map<Map::MoveDir, Map::Adjacent>& Map::getAdjacents() const
{
  return adjacents;
}

const Map::Adjacent& Map::getAdjacent(MoveDir direction) const
{
  if (adjacents.count(direction) > 0)
  {
    return adjacents.at(direction);
  } else {
    return defaultAdjacent;
  }
}

size_t Map::getNextObjectIndex() const
{
  return nextObjectIndex;
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

void Map::setAdjacent(MoveDir direction, MoveType type, int map, bool dirty)
{
  Adjacent& cur = adjacents[direction];
  cur.type = type;
  if (map != -1) cur.map = map;
  
  if (dirty)
  {
    world->setDirty(true);
  }
}

size_t Map::getAndIncrementNextObjectIndex()
{
  return nextObjectIndex++;
}

void Map::setNextObjectIndex(size_t v)
{
  nextObjectIndex = v;
}
