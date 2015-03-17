#ifndef MAP_H
#define MAP_H

class Map;

#include <string>
#include <exception>
#include <utility>
#include <list>
#include "object.h"
#include <memory>
#include "world.h"
#include <wx/treectrl.h>

class MapeditFrame;

const int TILE_WIDTH = 8;
const int TILE_HEIGHT = 8;
const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 200;
const int MAP_WIDTH = GAME_WIDTH/TILE_WIDTH;
const int MAP_HEIGHT = GAME_HEIGHT/TILE_HEIGHT - 1;
const int PLAYER_WIDTH[5] = {10, 0, 0, 0, 0};
const int PLAYER_HEIGHT[5] = {12, 0, 0, 0, 0};

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

struct MapObjectEntry {
  MapObject* object;
  std::pair<int, int> position;
  
  bool operator==(MapObjectEntry& other) const
  {
    return (object == other.object) && (position == other.position);
  }
  
  bool operator!=(MapObjectEntry& other) const
  {
    return (object != other.object) && (position != other.position);
  }
};

class Map {
  public:
    Map(int id, World* world);
    Map(const Map& map);
    Map(Map&& map);
    ~Map();
    Map& operator= (Map other);
    friend void swap(Map& first, Map& second);
    
    int getID() const;
    std::string getTitle() const;
    int getTileAt(int x, int y) const;
    const std::list<std::shared_ptr<MapObjectEntry>>& getObjects() const;
    std::shared_ptr<Map> getLeftmap() const;
    std::shared_ptr<Map> getRightmap() const;
    wxTreeItemId getTreeItemId() const;
    std::list<std::shared_ptr<Map>> getChildren() const;
    bool getExpanded() const;
    World* getWorld() const;
    bool getHidden() const;
    
    void setTitle(std::string title, bool dirty = true);
    void setTileAt(int x, int y, int tile, bool dirty = true);
    void setMapdata(int* mapdata, bool dirty = true);
    void addObject(std::shared_ptr<MapObjectEntry> obj, bool dirty = true);
    void removeObject(std::shared_ptr<MapObjectEntry> obj, bool dirty = true);
    void setLeftmap(int id, bool dirty = true);
    void setRightmap(int id, bool dirty = true);
    void setTreeItemId(wxTreeItemId id);
    void addChild(int id);
    void setExpanded(bool exp);
    void setHidden(bool hid);
    
  private:
    int id;
    World* world;
    std::list<std::shared_ptr<MapObjectEntry>> objects;
    int* mapdata;
    std::string title {"Untitled Map"};
    std::list<int> children;
    int leftmap = -1;
    int rightmap = -1;
    wxTreeItemId treeItemId;
    bool expanded = false;
    bool hidden = false;
};

#endif
