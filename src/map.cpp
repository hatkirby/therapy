#include "map.h"
#include <cstdlib>
#include <cstring>
#include <map>
#include "entityfactory.h"
#include "entity.h"
#include "game.h"
#include "consts.h"

Map::Map(int id)
{
  this->id = id;
  mapdata = (int*) calloc(1, sizeof(int));
}

Map::Map(const Map& map)
{
  mapdata = (int*) malloc(MAP_WIDTH*MAP_HEIGHT*sizeof(int));
  memcpy(mapdata, map.mapdata, MAP_WIDTH*MAP_HEIGHT*sizeof(int));
  
  id = map.id;
  title = map.title;
  adjacents = map.adjacents;
  entities = map.entities;
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
  std::swap(first.adjacents, second.adjacents);
  std::swap(first.id, second.id);
  std::swap(first.entities, second.entities);
}

int Map::getID() const
{
  return id;
}

const int* Map::getMapdata() const
{
  return mapdata;
}

std::string Map::getTitle() const
{
  return title;
}

void Map::createEntities(std::list<std::shared_ptr<Entity>>& entities) const
{
  for (auto data : this->entities)
  {
    auto entity = EntityFactory::createNamedEntity(data.name);
    entity->position = data.position;
    
    entities.push_back(entity);
  }
}

bool Map::operator==(const Map& other) const
{
  return id == other.id;
}

bool Map::operator!=(const Map& other) const
{
  return id != other.id;
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

static const Map::Adjacent defaultAdjacent {};
const Map::Adjacent& Map::getAdjacent(MoveDir dir) const
{
  if (adjacents.count(dir) > 0)
  {
    return adjacents.at(dir);
  } else {
    return defaultAdjacent;
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

void Map::setMapdata(int* mapdata)
{
  free(this->mapdata);
  this->mapdata = mapdata;
}

void Map::setTitle(std::string title)
{
  this->title = title;
}

void Map::setAdjacent(MoveDir dir, MoveType type, int map)
{
  Adjacent& cur = adjacents[dir];
  cur.type = type;
  if (map != -1) cur.map = map;
}

void Map::addEntity(EntityData& data)
{
  entities.push_back(data);
}

