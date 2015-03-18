#ifndef MAP_H
#define MAP_H

#include <string>
#include <list>
#include <map>

class Entity;

class Map {
  public:
    Map(int id);
    Map() : Map(-1) {}
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
    
    enum class MoveDir {
      Left,
      Right,
      Up,
      Down
    };
    
    struct EntityData {
      std::string name;
      std::pair<int, int> position;
    };
    
    struct Adjacent {
      MoveType type = MoveType::Wall;
      int map = -1;
    };
    
    static MoveType moveTypeForShort(std::string str);
    static MoveDir moveDirForShort(std::string str);
    static bool moveTypeTakesMap(MoveType type);
    
    int getID() const;
    const int* getMapdata() const;
    std::string getTitle() const;
    const Adjacent& getAdjacent(MoveDir dir) const;
    
    void createEntities(std::list<std::shared_ptr<Entity>>& entities) const;
    bool operator==(const Map& other) const;
    bool operator!=(const Map& other) const;
    
    void setMapdata(int* mapdata);
    void setTitle(std::string title);
    void setAdjacent(MoveDir dir, MoveType type, int map);
    void addEntity(EntityData& data);
  private:
    int* mapdata;
    std::string title;
    int id;
    std::list<EntityData> entities;
    std::map<MoveDir, Adjacent> adjacents;
};

#endif
