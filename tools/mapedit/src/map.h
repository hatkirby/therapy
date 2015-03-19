#ifndef MAP_H
#define MAP_H

#include <string>
#include <exception>
#include <utility>
#include <list>
#include <memory>
#include <wx/treectrl.h>
#include <map>
#include "object.h"

class World;
class MapeditFrame;
class MapEntryObject;

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
    Map(int id, World* world);
    Map(const Map& map);
    Map(Map&& map);
    ~Map();
    Map& operator= (Map other);
    friend void swap(Map& first, Map& second);
    
    enum class MoveDir {
      Left,
      Right,
      Up,
      Down
    };
    
    enum class MoveType {
      Wall,
      Wrap,
      Warp,
      ReverseWarp
    };
    
    struct Adjacent {
      MoveType type = MoveType::Wall;
      int map = 0;
    };
    
    static std::list<MoveType> listMoveTypes();
    static std::string stringForMoveType(MoveType type);
    static bool moveTypeTakesMap(MoveType type);
    static std::string shortForMoveType(MoveType type);
    static std::string shortForMoveDir(MoveDir dir);
    static MoveType moveTypeForShort(std::string str);
    static MoveDir moveDirForShort(std::string str);
    
    int getID() const;
    std::string getTitle() const;
    int getTileAt(int x, int y) const;
    const std::list<std::shared_ptr<MapObjectEntry>>& getObjects() const;
    wxTreeItemId getTreeItemId() const;
    std::list<std::shared_ptr<Map>> getChildren() const;
    bool getExpanded() const;
    World* getWorld() const;
    bool getHidden() const;
    const std::map<MoveDir, Adjacent>& getAdjacents() const;
    const Adjacent& getAdjacent(MoveDir direction) const;
    
    void setTitle(std::string title, bool dirty = true);
    void setTileAt(int x, int y, int tile, bool dirty = true);
    void setMapdata(int* mapdata, bool dirty = true);
    void addObject(std::shared_ptr<MapObjectEntry> obj, bool dirty = true);
    void removeObject(std::shared_ptr<MapObjectEntry> obj, bool dirty = true);
    void setTreeItemId(wxTreeItemId id);
    void addChild(int id);
    void setExpanded(bool exp);
    void setHidden(bool hid);
    void setAdjacent(MoveDir direction, MoveType type, int map = -1, bool dirty = true);
    
  private:
    int id;
    World* world;
    std::list<std::shared_ptr<MapObjectEntry>> objects;
    int* mapdata;
    std::string title {"Untitled Map"};
    std::list<int> children;
    wxTreeItemId treeItemId;
    bool expanded = false;
    bool hidden = false;
    std::map<MoveDir, Adjacent> adjacents;
    const Adjacent defaultAdjacent {};
};

class MapPtrCtr : public wxTreeItemData {
  public:
    Map* map;
  
    MapPtrCtr(Map* map) : map(map) {}
};

class MoveTypeCtr {
  public:
    Map::MoveType type;
    
    MoveTypeCtr(Map::MoveType type) : type(type) {}
};

#endif
