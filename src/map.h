#ifndef MAP_H
#define MAP_H

#include <string>
#include <list>

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
    
    struct EntityData {
      std::string name;
      std::pair<int, int> position;
    };
    
    static MoveType moveTypeForShort(std::string str);
    static bool moveTypeTakesMap(MoveType type);
    
    int getID() const;
    const int* getMapdata() const;
    std::string getTitle() const;
    MoveType getLeftMoveType() const;
    MoveType getRightMoveType() const;
    MoveType getUpMoveType() const;
    MoveType getDownMoveType() const;
    int getLeftMapID() const;
    int getRightMapID() const;
    int getUpMapID() const;
    int getDownMapID() const;
    
    void createEntities(std::list<std::shared_ptr<Entity>>& entities) const;
    bool operator==(const Map& other) const;
    bool operator!=(const Map& other) const;
    
    void setMapdata(int* mapdata);
    void setTitle(std::string title);
    void setLeftMoveType(MoveType type);
    void setRightMoveType(MoveType type);
    void setUpMoveType(MoveType type);
    void setDownMoveType(MoveType type);
    void setLeftMapID(int id);
    void setRightMapID(int id);
    void setUpMapID(int id);
    void setDownMapID(int id);
    void addEntity(EntityData& data);
  private:
    int* mapdata;
    std::string title;
    int id;
    std::list<EntityData> entities;
    MoveType leftType, rightType, upType, downType;
    int leftMap, rightMap, upMap, downMap;
};

#endif
