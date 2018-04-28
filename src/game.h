#ifndef GAME_H_1014DDC9
#define GAME_H_1014DDC9

#include "entity_manager.h"
#include "system_manager.h"
#include "renderer/renderer.h"

class Game {
public:

  Game();

  void execute();

  inline Renderer& getRenderer()
  {
    return renderer_;
  }

  inline EntityManager& getEntityManager()
  {
    return entityManager_;
  }

  inline SystemManager& getSystemManager()
  {
    return systemManager_;
  }

  friend void key_callback(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods);

private:

  Renderer renderer_;
  EntityManager entityManager_;
  SystemManager systemManager_;
  bool shouldQuit_ = false;
};

#endif /* end of include guard: GAME_H_1014DDC9 */
