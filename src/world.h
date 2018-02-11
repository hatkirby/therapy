#ifndef WORLD_H_153C698B
#define WORLD_H_153C698B

#include <map>
#include <string>
#include "map.h"

class World {
public:

  explicit World(std::string filename);

  inline const Map& getMap(size_t id) const
  {
    return maps_.at(id);
  }

  inline size_t getStartingMapId() const
  {
    return startMap_;
  }

  inline int getStartingX() const
  {
    return startX_;
  }

  inline int getStartingY() const
  {
    return startY_;
  }

private:

  std::map<size_t, Map> maps_;
  size_t startMap_;
  int startX_;
  int startY_;
};

#endif /* end of include guard: WORLD_H_153C698B */
