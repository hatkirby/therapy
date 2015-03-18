#ifndef MAP_H
#define MAP_H

#include <string>
#include <exception>
#include <utility>
#include <list>
#include <memory>
#include <wx/treectrl.h>

class MapObject;
class World;
class MapeditFrame;

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
    
    enum class MoveType {
      Wall,
      Wrap,
      Warp,
      ReverseWarp
    };
    
    static std::list<MoveType> listMoveTypes();
    static std::string stringForMoveType(MoveType type);
    static bool moveTypeTakesMap(MoveType type);
    static std::string shortForMoveType(MoveType type);
    static MoveType moveTypeForShort(std::string str);
    
    int getID() const;
    std::string getTitle() const;
    int getTileAt(int x, int y) const;
    const std::list<std::shared_ptr<MapObjectEntry>>& getObjects() const;
    wxTreeItemId getTreeItemId() const;
    std::list<std::shared_ptr<Map>> getChildren() const;
    bool getExpanded() const;
    World* getWorld() const;
    bool getHidden() const;
    MoveType getLeftMoveType() const;
    MoveType getRightMoveType() const;
    MoveType getUpMoveType() const;
    MoveType getDownMoveType() const;
    int getLeftMoveMapID() const;
    int getRightMoveMapID() const;
    int getUpMoveMapID() const;
    int getDownMoveMapID() const;
    
    void setTitle(std::string title, bool dirty = true);
    void setTileAt(int x, int y, int tile, bool dirty = true);
    void setMapdata(int* mapdata, bool dirty = true);
    void addObject(std::shared_ptr<MapObjectEntry> obj, bool dirty = true);
    void removeObject(std::shared_ptr<MapObjectEntry> obj, bool dirty = true);
    void setTreeItemId(wxTreeItemId id);
    void addChild(int id);
    void setExpanded(bool exp);
    void setHidden(bool hid);
    void setLeftMoveType(MoveType move, bool dirty = true);
    void setRightMoveType(MoveType move, bool dirty = true);
    void setUpMoveType(MoveType move, bool dirty = true);
    void setDownMoveType(MoveType move, bool dirty = true);
    void setLeftMoveMapID(int id, bool dirty = true);
    void setRightMoveMapID(int id, bool dirty = true);
    void setUpMoveMapID(int id, bool dirty = true);
    void setDownMoveMapID(int id, bool dirty = true);
    
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
    MoveType leftType = MoveType::Wall;
    MoveType rightType = MoveType::Wall;
    MoveType upType = MoveType::Wall;
    MoveType downType = MoveType::Wall;
    int leftMap = 0;
    int rightMap = 0;
    int upMap = 0;
    int downMap = 0;
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
