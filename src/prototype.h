#ifndef PROTOTYPE_H_BB208E0F
#define PROTOTYPE_H_BB208E0F

#include <string>
#include <map>
#include "map.h"
#include "entity_manager.h"

class Game;

class Prototype {
public:

  using id_type = EntityManager::id_type;

  enum class Action {
    none,
    save
  };

  Prototype(
    int w,
    int h,
    std::string spritePath,
    Action action) :
      w_(w),
      h_(h),
      spritePath_(std::move(spritePath)),
      action_(action)
  {
  }

  id_type instantiate(
    Game& game,
    const Map::Object& object) const;

private:

  int w_;
  int h_;
  std::string spritePath_;
  Action action_;
};

#endif /* end of include guard: PROTOTYPE_H_BB208E0F */
