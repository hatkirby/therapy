#ifndef GAME_H_1014DDC9
#define GAME_H_1014DDC9

#include "renderer.h"
#include "entity_manager.h"
#include "system_manager.h"

class Game {
  public:
    Game(GLFWwindow* window);

    void execute();
    EntityManager& getEntityManager();

    friend void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

  private:
    EntityManager entityManager;
    SystemManager systemManager;
    GLFWwindow* const window;
    bool shouldQuit = false;
};

#endif /* end of include guard: GAME_H_1014DDC9 */
