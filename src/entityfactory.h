#ifndef ENTITYFACTORY_H
#define ENTITYFACTORY_H

#include <string>
#include <map>

class Entity;
class Map;

class EntityFactory {
  public:
    static std::shared_ptr<Entity> createNamedEntity(const std::string name, const std::map<std::string, int>& items);
};

#endif
