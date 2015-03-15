#ifndef MAP_H
#define MAP_H

#include <string>
#include <exception>

const int TILE_WIDTH = 8;
const int TILE_HEIGHT = 8;
const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 200;
const int MAP_WIDTH = GAME_WIDTH/TILE_WIDTH;
const int MAP_HEIGHT = GAME_HEIGHT/TILE_HEIGHT - 1;

class MapLoadException: public std::exception
{
  public:
    MapLoadException(std::string mapname) : mapname(mapname) {}
    
    virtual const char* what() const throw()
    {
      return ("An error occured loading map " + mapname).c_str();
    }
    
  private:
    std::string mapname;
};

class MapWriteException: public std::exception
{
  public:
    MapWriteException(std::string mapname) : mapname(mapname) {}
    
    virtual const char* what() const throw()
    {
      return ("An error occured writing map " + mapname).c_str();
    }
    
  private:
    std::string mapname;
};

class Map {
  public:
    Map();
    Map(std::string name);
    Map(const Map& map);
    Map(Map&& map);
    ~Map();
    Map& operator= (Map other);
    friend void swap(Map& first, Map& second);
    
    std::string getTitle() const;
    void setTitle(std::string title);
    void save(std::string name);
    bool hasUnsavedChanges() const;
    void setTileAt(int x, int y, int tile);
    int getTileAt(int x, int y) const;
    
  private:
    int* mapdata;
    std::string title;
    std::string leftmap;
    std::string rightmap;
    bool dirty;
};

#endif