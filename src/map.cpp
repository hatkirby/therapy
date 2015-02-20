#include "map.h"
#include "mapview.h"

Map::Map(char* filename)
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

Map::~Map()
{
  free(m_mapdata);
  free(m_title);
}

const int* Map::mapdata()
{
  return m_mapdata;
}

const char* Map::title()
{
  return m_title;
}

Map* Map::getLeftMap()
{
  return m_leftMap;
}

Map* Map::getRightMap()
{
  return m_rightMap;
}

void Map::setLeftMap(Map* m)
{
  m_leftMap = m;
}

void Map::setRightMap(Map* m)
{
  m_rightMap = m;
}
