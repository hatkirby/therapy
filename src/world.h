#ifndef WORLD_H
#define WORLD_H

#include <map>
#include "map.h"

class World {
  public:
    World(const char* filename);
    const Map& getMap(int id) const;
    const Map& getStartingMap() const;
    std::pair<int, int> getStartingPosition() const;
    
  private:
    std::map<int, Map> maps;
    int startMap;
    int startX;
    int startY;
};

#endif /* end of include guard: WORLD_H */
