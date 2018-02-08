#ifndef GAME_H_1014DDC9
#define GAME_H_1014DDC9

#include "renderer.h"
#include "entity_manager.h"
#include "system_manager.h"

class Game {
public:

  Game(GLFWwindow* window);

  void execute();

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

  EntityManager entityManager_;
  SystemManager systemManager_;
  GLFWwindow* const window_;
  bool shouldQuit_ = false;
};

#endif /* end of include guard: GAME_H_1014DDC9 */
