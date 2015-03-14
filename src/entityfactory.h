#ifndef ENTITYFACTORY_H
#define ENTITYFACTORY_H

#include "entity.h"
#include <memory>
#include <string>
#include <map>
#include "map.h"

class EntityFactory {
  public:
    static std::shared_ptr<Entity> createNamedEntity(const std::string name, const Map& map);
};

#endif
