#ifndef ENTITY_MANAGER_CPP_42D78C22
#define ENTITY_MANAGER_CPP_42D78C22

#include "entity_manager.h"

template <>
std::set<EntityManager::id_type> EntityManager::getEntitiesWithComponents<>(
  std::set<std::type_index>& componentTypes) const
{
  if (cachedComponents.count(componentTypes) == 1)
  {
    return cachedComponents[componentTypes];
  }

  std::set<id_type>& cache = cachedComponents[componentTypes];
  for (id_type entity = 0; entity < entities.size(); entity++)
  {
    const EntityData& data = entities[entity];
    bool cacheEntity = true;

    for (auto& componentType : componentTypes)
    {
      if (data.components.count(componentType) == 0)
      {
        cacheEntity = false;
        break;
      }
    }

    if (cacheEntity)
    {
      cache.insert(entity);
    }
  }

  return cache;
}

#endif /* end of include guard: ENTITY_MANAGER_CPP_42D78C22 */
