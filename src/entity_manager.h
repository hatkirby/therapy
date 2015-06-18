#ifndef ENTITY_MANAGER_H_C5832F11
#define ENTITY_MANAGER_H_C5832F11

#include <map>
#include <typeindex>
#include <set>
#include <cassert>
#include "component.h"
#include "algorithms.h"

class EntityManager {
  private:
    struct EntityData {
      std::map<std::type_index, std::unique_ptr<Component>> components;
    };
  
    std::map<int, EntityData> entities;
    std::map<std::set<std::type_index>, std::set<int>> cachedComponents;
  
    int nextEntityID = 0;
    
    template <class T, class... R>
    std::set<int> getEntitiesWithComponentsHelper(std::set<std::type_index>& componentTypes)
    {
      componentTypes.insert(typeid(T));
      
      return getEntitiesWithComponents<R...>(componentTypes);
    }
    
    template <class... R>
    std::set<int> getEntitiesWithComponents(std::set<std::type_index>& componentTypes)
    {
      return getEntitiesWithComponentsHelper<R...>(componentTypes);
    }
    
  public:
    EntityManager() = default;
    EntityManager(const EntityManager& copy) = delete;
    
    int emplaceEntity()
    {
      // Find a suitable entity ID
      while ((entities.count(nextEntityID) == 1) && (nextEntityID >= 0))
      {
        nextEntityID++;
      }
        
      if (nextEntityID < 0)
      {
        nextEntityID = 0;
        
        while ((entities.count(nextEntityID) == 1) && (nextEntityID >= 0))
        {
          nextEntityID++;
        }
        
        assert(nextEntityID >= 0);
      }
      
      // Initialize the data
      int id = nextEntityID++;
      entities[id];
      
      return id;
    }
    
    void deleteEntity(int entity)
    {
      assert(entities.count(entity) == 1);
      
      // Uncache components
      for (auto& cache : cachedComponents)
      {
        cache.second.erase(entity);
      }
      
      // Destroy the data
      entities.erase(entity);
    }
    
    template <class T, class... Args>
    T& emplaceComponent(int entity, Args&&... args)
    {
      assert(entities.count(entity) == 1);
      
      EntityData& data = entities[entity];
      std::type_index componentType = typeid(T);
      
      assert(data.components.count(componentType) == 0);
      
      // Initialize the component
      std::unique_ptr<T> ptr = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
      T& component = *ptr;
      data.components[componentType] = std::move(ptr);
      
      // Invalidate related caches
      erase_if(cachedComponents, [&componentType] (std::pair<const std::set<std::type_index>, std::set<int>>& cache) {
        return cache.first.count(componentType) == 1;
      });
      
      return component;
    }
    
    template <class T>
    void removeComponent(int entity)
    {
      assert(entities.count(entity) == 1);
      
      EntityData& data = entities[entity];
      std::type_index componentType = typeid(T);
      
      assert(data.components.count(componentType) == 1);
      
      // Destroy the component
      data.components.erase(componentType);
      
      // Uncache the component
      for (auto& cache : cachedComponents)
      {
        if (cache.first.count(componentType) == 1)
        {
          cache.second.erase(entity);
        }
      }
    }
    
    template <class T>
    T& getComponent(int entity)
    {
      assert(entities.count(entity) == 1);
      
      EntityData& data = entities[entity];
      std::type_index componentType = typeid(T);
      
      assert(data.components.count(componentType) == 1);
      
      return *((T*)data.components[componentType].get());
    }
    
    template <class... R>
    std::set<int> getEntitiesWithComponents()
    {
      std::set<std::type_index> componentTypes;
      
      return getEntitiesWithComponentsHelper<R...>(componentTypes);
    }
};

template <>
std::set<int> EntityManager::getEntitiesWithComponents<>(std::set<std::type_index>& componentTypes);

#endif /* end of include guard: ENTITY_MANAGER_H_C5832F11 */
