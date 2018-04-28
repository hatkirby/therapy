#include "game.h"
#include "components/animatable.h"
#include "components/transformable.h"
#include "components/controllable.h"
#include "components/ponderable.h"
#include "components/orientable.h"
#include "systems/controlling.h"
#include "systems/pondering.h"
#include "systems/animating.h"
#include "systems/mapping.h"
#include "systems/orienting.h"
#include "systems/playing.h"
#include "systems/scheduling.h"
#include "systems/realizing.h"
#include "animation.h"
#include "consts.h"

void key_callback(GLFWwindow* window, int key, int, int action, int)
{
  Game& game = *static_cast<Game*>(glfwGetWindowUserPointer(window));

  if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
  {
    game.shouldQuit_ = true;

    return;
  }

  game.systemManager_.input(key, action);
}

Game::Game()
{
  systemManager_.emplaceSystem<RealizingSystem>(*this);
  systemManager_.emplaceSystem<PlayingSystem>(*this);
  systemManager_.emplaceSystem<SchedulingSystem>(*this);
  systemManager_.emplaceSystem<ControllingSystem>(*this);
  systemManager_.emplaceSystem<OrientingSystem>(*this);
  systemManager_.emplaceSystem<PonderingSystem>(*this);
  systemManager_.emplaceSystem<MappingSystem>(*this);
  systemManager_.emplaceSystem<AnimatingSystem>(*this);

  systemManager_.getSystem<RealizingSystem>().initSingleton(
    "res/maps.xml",
    "res/entities.xml");

  systemManager_.getSystem<PlayingSystem>().initPlayer();

  glfwSwapInterval(1);
  glfwSetWindowUserPointer(renderer_.getWindow().getHandle(), this);
  glfwSetKeyCallback(renderer_.getWindow().getHandle(), key_callback);
}

void Game::execute()
{
  double lastTime = glfwGetTime();
  const double dt = 0.01;
  double accumulator = 0.0;
  Texture texture(GAME_WIDTH, GAME_HEIGHT);

  while (!(shouldQuit_ ||
    glfwWindowShouldClose(renderer_.getWindow().getHandle())))
  {
    double currentTime = glfwGetTime();
    double frameTime = currentTime - lastTime;
    lastTime = currentTime;

    glfwPollEvents();

    accumulator += frameTime;
    while (accumulator >= dt)
    {
      systemManager_.tick(dt);

      accumulator -= dt;
    }

    // Render
    renderer_.fill(texture, texture.entirety(), 0, 0, 0);
    systemManager_.render(texture);
    renderer_.renderScreen(texture);
  }
}
