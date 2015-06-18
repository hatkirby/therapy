#ifndef ENTITY_MANAGER_CPP_42D78C22
#define ENTITY_MANAGER_CPP_42D78C22

#include "entity_manager.h"

template <>
std::set<int> EntityManager::getEntitiesWithComponents<>(std::set<std::type_index>& componentTypes)
{
  if (cachedComponents.count(componentTypes) == 1)
  {
    return cachedComponents[componentTypes];
  }
  
  std::set<int>& cache = cachedComponents[componentTypes];
  for (auto& entity : entities)
  {
    EntityData& data = entity.second;
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
      cache.insert(entity.first);
    }
  }
  
  return cache;
}

#endif /* end of include guard: ENTITY_MANAGER_CPP_42D78C22 */
