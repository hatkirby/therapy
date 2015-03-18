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
  leftMap = map.leftMap;
  rightMap = map.rightMap;
  downMap = map.downMap;
  upMap = map.upMap;
  leftType = map.leftType;
  rightType = map.rightType;
  upType = map.upType;
  downType = map.downType;
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
  std::swap(first.leftMap, second.leftMap);
  std::swap(first.rightMap, second.rightMap);
  std::swap(first.downMap, second.downMap);
  std::swap(first.upMap, second.upMap);
  std::swap(first.leftType, second.leftType);
  std::swap(first.rightType, second.rightType);
  std::swap(first.upType, second.upType);
  std::swap(first.downType, second.downType);
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

int Map::getLeftMapID() const
{
  return leftMap;
}

int Map::getRightMapID() const
{
  return rightMap;
}

int Map::getUpMapID() const
{
  return upMap;
}

int Map::getDownMapID() const
{
  return downMap;
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

void Map::setLeftMoveType(MoveType type)
{
  leftType = type;
}

void Map::setRightMoveType(MoveType type)
{
  rightType = type;
}

void Map::setUpMoveType(MoveType type)
{
  upType = type;
}

void Map::setDownMoveType(MoveType type)
{
  downType = type;
}

void Map::setLeftMapID(int id)
{
  leftMap = id;
}

void Map::setRightMapID(int id)
{
  rightMap = id;
}

void Map::setUpMapID(int id)
{
  upMap = id;
}

void Map::setDownMapID(int id)
{
  downMap = id;
}

void Map::addEntity(EntityData& data)
{
  entities.push_back(data);
}

