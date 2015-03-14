#ifndef MAP_H
#define MAP_H

#include <string>
#include <list>

class Entity;

class Map {
  public:
    Map();
    Map(const std::string name);
    Map(const Map& map);
    Map(Map&& map);
    ~Map();
    Map& operator= (Map other);
    friend void swap(Map& first, Map& second);
    
    static Map& getNamedMap(const std::string name);
    
    const int* getMapdata() const;
    const char* getTitle() const;
    const Map* getLeftMap() const;
    const Map* getRightMap() const;
    void setLeftMap(const Map* m);
    void setRightMap(const Map* m);
    void createEntities(std::list<std::shared_ptr<Entity>>& entities) const;
  private:
    struct EntityData {
      std::string name;
      std::pair<double, double> position;
    };
    
    int* mapdata;
    char* title;
    const Map* leftMap = nullptr;
    const Map* rightMap = nullptr;
    std::list<EntityData> entities;
};

#endif
