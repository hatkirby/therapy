#include "map.h"
#include "game.h"
#include <cstdlib>
#include <cstring>

Map::Map()
{
  
}

Map::Map(const char* filename)
{
  FILE* f = fopen(filename, "r");
  
  m_mapdata = (int*) malloc(MAP_WIDTH*(MAP_HEIGHT-1)*sizeof(int));
  for (int i=0; i<MAP_HEIGHT-1; i++)
  {
    for (int j=0; j<MAP_WIDTH; j++)
    {
      fscanf(f, "%d,", &(m_mapdata[i*MAP_WIDTH + j]));
    }
    
    fgetc(f);
  }
  
  m_title = (char*) calloc(41, sizeof(char));
  fgets(m_title, 41, f);
  
  fclose(f);
}

Map::Map(Map& map)
{
  m_mapdata = (int*) malloc(MAP_WIDTH*(MAP_HEIGHT-1)*sizeof(int));
  memcpy(m_mapdata, map.m_mapdata, MAP_WIDTH*(MAP_HEIGHT-1)*sizeof(int));
  
  m_title = (char*) malloc((MAP_WIDTH+1)*sizeof(char));
  strncpy(m_title, map.m_title, MAP_WIDTH+1);
  
  m_leftMap = map.m_leftMap;
  m_rightMap = map.m_rightMap;
}

Map::Map(Map&& map) : Map()
{
  swap(*this, map);
}

Map::~Map()
{
  free(m_mapdata);
  free(m_title);
}

Map& Map::operator= (Map map)
{
  swap(*this, map);
  
  return *this;
}

void swap(Map& first, Map& second)
{
  std::swap(first.m_mapdata, second.m_mapdata);
  std::swap(first.m_title, second.m_title);
  std::swap(first.m_leftMap, second.m_leftMap);
  std::swap(first.m_rightMap, second.m_rightMap);
}

const int* Map::mapdata() const
{
  return m_mapdata;
}

const char* Map::title() const
{
  return m_title;
}

const Map* Map::getLeftMap() const
{
  return m_leftMap;
}

const Map* Map::getRightMap() const
{
  return m_rightMap;
}

void Map::setLeftMap(const Map* m)
{
  m_leftMap = m;
}

void Map::setRightMap(const Map* m)
{
  m_rightMap = m;
}
