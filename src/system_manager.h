#ifndef SYSTEM_MANAGER_H_544E6056
#define SYSTEM_MANAGER_H_544E6056

#include <list>
#include <memory>
#include <map>
#include <typeindex>
#include "system.h"

class SystemManager {
  private:
    std::list<std::unique_ptr<System>> loop;
    std::map<std::type_index, System*> systems;

  public:
    template <class T, class... Args>
    void emplaceSystem(Game& game, Args&&... args)
    {
      std::unique_ptr<T> ptr = std::unique_ptr<T>(new T(game, std::forward<Args>(args)...));
      std::type_index systemType = typeid(T);

      systems[systemType] = ptr.get();
      loop.push_back(std::move(ptr));
    }

    template <class T>
    T& getSystem()
    {
      std::type_index systemType = typeid(T);

      assert(systems.count(systemType) == 1);

      return *((T*)systems[systemType]);
    }
};

#endif /* end of include guard: SYSTEM_MANAGER_H_544E6056 */
