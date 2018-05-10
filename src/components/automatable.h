#ifndef AUTOMATABLE_H_3D519131
#define AUTOMATABLE_H_3D519131

#include "component.h"
#include <sol.hpp>
#include <memory>

class AutomatableComponent : public Component {
public:

  std::unique_ptr<sol::thread> runner;
  std::unique_ptr<sol::coroutine> behavior;

  sol::environment origBehavior;

  /**
   * If this flag is disabled, the entity will be ignored by the automating
   * system.
   *
   * @managed_by RealizingSystem
   */
  bool active = false;
};

#endif /* end of include guard: AUTOMATABLE_H_3D519131 */
