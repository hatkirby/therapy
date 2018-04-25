#ifndef ENTITY_MANAGER_H_C5832F11
#define ENTITY_MANAGER_H_C5832F11

#include <map>
#include <vector>
#include <typeindex>
#include <set>
#include <stdexcept>
#include "component.h"
#include "util.h"

class EntityManager {
private:

  struct EntityData {
    std::map<std::type_index, std::unique_ptr<Component>> components;
  };

  using database_type = std::vector<EntityData>;

public:

  using id_type = database_type::size_type;

private:

  database_type entities;
  std::vector<bool> slotAvailable;
  std::set<id_type> allEntities;

  mutable std::map<std::set<std::type_index>, std::set<id_type>>
    cachedComponents;

  id_type nextEntityID = 0;

  template <class T, class... R>
  std::set<id_type> getEntitiesWithComponentsHelper(
    std::set<std::type_index>& componentTypes) const
  {
    componentTypes.insert(typeid(T));

    return getEntitiesWithComponents<R...>(componentTypes);
  }

  template <class... R>
  std::set<id_type> getEntitiesWithComponents(
    std::set<std::type_index>& componentTypes) const
  {
    return getEntitiesWithComponentsHelper<R...>(componentTypes);
  }

public:

  EntityManager() = default;

  EntityManager(const EntityManager& copy) = delete;

  id_type emplaceEntity()
  {
    if (nextEntityID >= entities.size())
    {
      // If the database is saturated, add a new element for the new entity.
      entities.emplace_back();
      slotAvailable.push_back(false);
      allEntities.insert(nextEntityID);

      return nextEntityID++;
    } else {
      // If there is an available slot in the database, use it.
      id_type id = nextEntityID++;
      slotAvailable[id] = false;
      allEntities.insert(id);

      // Fast forward the next available slot pointer to an available slot.
      while ((nextEntityID < entities.size()) && !slotAvailable[nextEntityID])
      {
        nextEntityID++;
      }

      return id;
    }
  }

  void deleteEntity(id_type entity)
  {
    if ((entity >= entities.size()) || slotAvailable[entity])
    {
      throw std::invalid_argument("Cannot delete non-existent entity");
    }

    // Uncache components
    for (auto& cache : cachedComponents)
    {
      cache.second.erase(entity);
    }

    allEntities.erase(entity);

    // Destroy the data
    entities[entity].components.clear();

    // Mark the slot as available
    slotAvailable[entity] = true;

    if (entity < nextEntityID)
    {
      nextEntityID = entity;
    }
  }

  template <class T, class... Args>
  T& emplaceComponent(id_type entity, Args&&... args)
  {
    if ((entity >= entities.size()) || slotAvailable[entity])
    {
      throw std::invalid_argument("Cannot get non-existent entity");
    }

    EntityData& data = entities[entity];
    std::type_index componentType = typeid(T);

    if (data.components.count(componentType))
    {
      throw std::invalid_argument("Cannot emplace already-existent component");
    }

    // Initialize the component
    std::unique_ptr<T> ptr(new T(std::forward<Args>(args)...));
    T& component = *ptr;
    data.components[componentType] = std::move(ptr);

    // Invalidate related caches
    erase_if(
      cachedComponents,
      [&componentType] (
        std::pair<const std::set<std::type_index>, std::set<id_type>>& cache) {
          return cache.first.count(componentType) == 1;
        });

    return component;
  }

  template <class T>
  void removeComponent(id_type entity)
  {
    if ((entity >= entities.size()) || slotAvailable[entity])
    {
      throw std::invalid_argument("Cannot get non-existent entity");
    }

    EntityData& data = entities[entity];
    std::type_index componentType = typeid(T);

    if (!data.components.count(componentType))
    {
      throw std::invalid_argument("Cannot delete non-existent component");
    }

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
  const T& getComponent(id_type entity) const
  {
    if ((entity >= entities.size()) || slotAvailable[entity])
    {
      throw std::invalid_argument("Cannot get non-existent entity");
    }

    const EntityData& data = entities[entity];
    std::type_index componentType = typeid(T);

    if (!data.components.count(componentType))
    {
      throw std::invalid_argument("Cannot get non-existent component");
    }

    return *dynamic_cast<const T*>(data.components.at(componentType).get());
  }

  template <class T>
  T& getComponent(id_type entity)
  {
    return const_cast<T&>(
      static_cast<const EntityManager&>(*this).getComponent<T>(entity));
  }

  template <class T>
  bool hasComponent(id_type entity) const
  {
    if ((entity >= entities.size()) || slotAvailable[entity])
    {
      throw std::invalid_argument("Cannot get non-existent entity");
    }

    const EntityData& data = entities[entity];
    std::type_index componentType = typeid(T);

    return data.components.count(componentType);
  }

  template <class... R>
  std::set<id_type> getEntitiesWithComponents() const
  {
    std::set<std::type_index> componentTypes;

    return getEntitiesWithComponentsHelper<R...>(componentTypes);
  }

  const std::set<id_type>& getEntities() const
  {
    return allEntities;
  }
};

template <>
std::set<EntityManager::id_type> EntityManager::getEntitiesWithComponents<>(
  std::set<std::type_index>& componentTypes) const;

#endif /* end of include guard: ENTITY_MANAGER_H_C5832F11 */
