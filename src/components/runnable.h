#ifndef AUTOMATABLE_H_3D519131
#define AUTOMATABLE_H_3D519131

#include "component.h"
#include <sol.hpp>
#include <memory>

class RunnableComponent : public Component {
public:

  std::unique_ptr<sol::thread> runner;
  std::unique_ptr<sol::coroutine> callable;
};

#endif /* end of include guard: AUTOMATABLE_H_3D519131 */
