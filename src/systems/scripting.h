#ifndef AUTOMATING_H_E6E5D76E
#define AUTOMATING_H_E6E5D76E

#include "system.h"
#include <sol.hpp>

class ScriptingSystem : public System {
public:

  ScriptingSystem(Game& game);

  void tick(double dt);

  void killScript(id_type entity);

  void startBehavior(id_type entity);

  void stopBehavior(id_type entity);

  void onTouch(id_type entity, id_type player);

private:

  template <typename... Args>
  sol::optional<id_type> runScript(
    std::string table,
    std::string event,
    id_type entity,
    Args&&... args);

  sol::state engine_;
};

#endif /* end of include guard: AUTOMATING_H_E6E5D76E */
