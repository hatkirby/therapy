#ifndef ENTITY_MANAGER_H_C5832F11
#define ENTITY_MANAGER_H_C5832F11

#include <map>
#include <typeindex>
#include <set>
#include <cassert>
#include "component.h"

class EntityManager {
  private:
    struct EntityData {
      int parent = -1;
      std::map<std::type_index, std::unique_ptr<Component>> components;
    };
  
    std::map<int, EntityData> entities;
    std::map<int, std::set<int>> cachedChildren;
    std::map<std::set<std::type_index>, std::set<int>> cachedComponents;
  
    int nextEntityID = 0;
    
    bool ensureNoParentCycles(int entity, int parent)
    {
      EntityData& data = entities[parent];
      if (data.parent == entity)
      {
        return false;
      } else if (data.parent == -1)
      {
        return true;
      }
      
      return ensureNoParentCycles(entity, data.parent);
    }
    
    std::set<int> getEntitiesWithComponents(std::set<std::type_index>& componentTypes)
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
    
    template <class T, class... R> std::set<int> getEntitiesWithComponents(std::set<std::type_index>& componentTypes)
    {
      componentTypes.insert(typeid(T));
      
      return getEntitiesWithComponents<R...>(componentTypes);
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
      
      EntityData& data = entities[entity];
      
      // Destroy the children
      std::set<int> children = getChildren(entity);
      for (int child : children)
      {
        EntityData& childData = entities[child];
        childData.parent = -1;
        
        deleteEntity(child);
      }
      
      // Uncache children
      cachedChildren.erase(entity);
      
      if ((data.parent != -1) && (cachedChildren.count(data.parent) == 1))
      {
        cachedChildren[data.parent].erase(entity);
      }
      
      // Uncache components
      for (auto& cache : cachedComponents)
      {
        cache.second.erase(entity);
      }
      
      // Destroy the data
      entities.erase(entity);
    }
    
    std::set<int> getChildren(int parent)
    {
      assert(entities.count(parent) == 1);
      
      if (cachedChildren.count(parent) == 1)
      {
        return cachedChildren[parent];
      }
      
      std::set<int>& cache = cachedChildren[parent];
      for (auto& entity : entities)
      {
        EntityData& data = entity.second;
        if (data.parent == parent)
        {
          cache.insert(entity.first);
        }
      }
      
      return cache;
    }
    
    void setParent(int entity, int parent)
    {
      assert(entities.count(entity) == 1);
      assert(entities.count(parent) == 1);
      assert(ensureNoParentCycles(entity, parent));
      
      EntityData& data = entities[entity];
      
      // Remove from old parent
      if (data.parent != -1)
      {
        if (cachedChildren.count(data.parent) == 1)
        {
          cachedChildren[data.parent].erase(entity);
        }
      }
      
      data.parent = parent;
      
      // Cache new parent
      if (cachedChildren.count(parent) == 1)
      {
        cachedChildren[parent].insert(entity);
      }
    }
    
    void setNoParent(int entity)
    {
      assert(entities.count(entity) == 1);
      
      EntityData& data = entities[entity];
      
      // Remove from old parent
      if (data.parent != -1)
      {
        if (cachedChildren.count(data.parent) == 1)
        {
          cachedChildren[data.parent].erase(entity);
        }
      }
      
      data.parent = -1;
    }
    
    int getParent(int entity)
    {
      assert(entities.count(entity) == 1);
      
      EntityData& data = entities[entity];
      return data.parent;
    }
    
    template <class T, class... Args> T& emplaceComponent(int entity, Args&&... args)
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
      std::remove_if(begin(cachedComponents), end(cachedComponents), [&] (std::pair<std::set<std::type_index>, int>& cache) {
        return cache.first.count(componentType) == 1;
      });
      
      return component;
    }
    
    template <class T> void removeComponent(int entity)
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
    
    template <class T> T& getComponent(int entity)
    {
      assert(entities.count(entity) == 1);
      
      EntityData& data = entities[entity];
      std::type_index componentType = typeid(T);
      
      assert(data.components.count(componentType) == 1);
      
      return *(data.components[componentType]);
    }
    
    template <class T, class... R> std::set<int> getEntitiesWithComponents()
    {
      std::set<std::type_index> componentTypes;
      componentTypes.insert(typeid(T));
      
      return getEntitiesWithComponents<R...>(componentTypes);
    }
};

#endif /* end of include guard: ENTITY_MANAGER_H_C5832F11 */
