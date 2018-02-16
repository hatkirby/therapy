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

Game::Game() : world_("res/maps.xml")
{
  systemManager_.emplaceSystem<ControllingSystem>(*this);
  systemManager_.emplaceSystem<OrientingSystem>(*this);
  systemManager_.emplaceSystem<PonderingSystem>(*this);
  systemManager_.emplaceSystem<MappingSystem>(*this);
  systemManager_.emplaceSystem<AnimatingSystem>(*this);

  int player = entityManager_.emplaceEntity();

  AnimationSet playerGraphics {"res/Starla.png", 10, 12, 6};
  playerGraphics.emplaceAnimation("stillLeft", 3, 1, 1);
  playerGraphics.emplaceAnimation("stillRight", 0, 1, 1);
  playerGraphics.emplaceAnimation("walkingLeft", 4, 2, 10);
  playerGraphics.emplaceAnimation("walkingRight", 1, 2, 10);

  entityManager_.emplaceComponent<AnimatableComponent>(
    player,
    std::move(playerGraphics),
    "stillLeft");

  entityManager_.emplaceComponent<TransformableComponent>(
    player,
    203, 44, 10, 12);

  systemManager_.getSystem<PonderingSystem>().initializeBody(
    player,
    PonderableComponent::Type::freefalling);

  entityManager_.emplaceComponent<ControllableComponent>(player);
  entityManager_.emplaceComponent<OrientableComponent>(player);

  systemManager_.getSystem<MappingSystem>().loadMap(world_.getStartingMapId());

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
