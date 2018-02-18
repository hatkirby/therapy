#ifndef SCHEDULABLE_H_1DA3FA2A
#define SCHEDULABLE_H_1DA3FA2A

#include "component.h"
#include <tuple>
#include <list>
#include <functional>
#include "entity_manager.h"

class SchedulableComponent : public Component {
public:

  using id_type = EntityManager::id_type;

  using Callback = std::function<void(id_type)>;
  using Action = std::tuple<double, Callback>;

  std::list<Action> actions;
};

#endif /* end of include guard: SCHEDULABLE_H_1DA3FA2A */
