#ifndef SYSTEM_H_B61A8CEA
#define SYSTEM_H_B61A8CEA

#include "entity_manager.h"

class Game;
class Texture;

class System {
public:

  using id_type = EntityManager::id_type;

  System(Game& game) : game_(game)
  {
  }

  virtual ~System() = default;

  /**
   * Updates the state of a system.
   *
   * @param dt - The amount of time in seconds that have passed since the last
   *             update.
   */
  virtual void tick(double)
  {
  }

  /**
   * Renders to a texture.
   *
   * @param texture - The surface to render to.
   */
  virtual void render(Texture&)
  {
  }

  /**
   * Processes keyboard input.
   *
   * @param key - The relevant key.
   *
   * @param action - The action performed (press, released, etc).
   */
  virtual void input(int, int)
  {
  }

protected:

  Game& game_;
};

#endif /* end of include guard: SYSTEM_H_B61A8CEA */
