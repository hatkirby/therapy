#ifndef ENTITYFACTORY_H
#define ENTITYFACTORY_H

#include <string>

class Entity;
class Map;

class EntityFactory {
  public:
    static std::shared_ptr<Entity> createNamedEntity(const std::string name);
};

#endif
