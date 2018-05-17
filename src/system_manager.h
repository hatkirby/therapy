#ifndef SYSTEM_MANAGER_H_544E6056
#define SYSTEM_MANAGER_H_544E6056

#include <list>
#include <memory>
#include <map>
#include <typeindex>
#include <stdexcept>
#include "system.h"

class SystemManager {
private:

  std::list<std::unique_ptr<System>> loop;
  std::map<std::type_index, System*> systems;

public:

  template <class T, class... Args>
  void emplaceSystem(Game& game, Args&&... args)
  {
    std::unique_ptr<T> ptr(new T(game, std::forward<Args>(args)...));
    std::type_index systemType = typeid(T);

    systems[systemType] = ptr.get();
    loop.push_back(std::move(ptr));
  }

  template <class T>
  T& getSystem()
  {
    std::type_index systemType = typeid(T);

    if (!systems.count(systemType))
    {
      throw std::invalid_argument("Cannot get non-existent system");
    }

    return *dynamic_cast<T*>(systems[systemType]);
  }

  void tick(double dt)
  {
    for (std::unique_ptr<System>& sys : loop)
    {
      sys->tick(dt);
    }
  }

  virtual void render(Texture& texture)
  {
    for (std::unique_ptr<System>& sys : loop)
    {
      sys->render(texture);
    }
  }

  virtual void input(int key, int action)
  {
    for (std::unique_ptr<System>& sys : loop)
    {
      sys->input(key, action);
    }
  }

};

#endif /* end of include guard: SYSTEM_MANAGER_H_544E6056 */
