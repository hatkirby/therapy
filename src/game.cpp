#include "game.h"
#include "components/animatable.h"
#include "components/transformable.h"
#include "components/controllable.h"
#include "components/droppable.h"
#include "components/ponderable.h"
#include "systems/rendering.h"
#include "systems/controlling.h"
#include "systems/pondering.h"

void key_callback(GLFWwindow* window, int key, int, int action, int)
{
  Game& game = *((Game*) glfwGetWindowUserPointer(window));
  
  if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
  {
    game.shouldQuit = true;
    
    return;
  }
  
  game.systemManager.getSystem<ControllingSystem>().input(key, action);
}

Game::Game(GLFWwindow* window) : window(window)
{
  systemManager.emplaceSystem<ControllingSystem>(*this);
  systemManager.emplaceSystem<RenderingSystem>(*this);
  systemManager.emplaceSystem<PonderingSystem>(*this);
  
  int player = entityManager.emplaceEntity();
  entityManager.emplaceComponent<AnimatableComponent>(player, "res/Starla.png", 10, 12, 6);
  entityManager.emplaceComponent<TransformableComponent>(player, 203, 44, 10, 12);
  entityManager.emplaceComponent<DroppableComponent>(player);
  entityManager.emplaceComponent<PonderableComponent>(player);
  entityManager.emplaceComponent<ControllableComponent>(player);
  
  glfwSwapInterval(1);
  glfwSetWindowUserPointer(window, this);
  glfwSetKeyCallback(window, key_callback);
}

void Game::execute()
{
  double lastTime = glfwGetTime();
  const double dt = 0.01;
  double accumulator = 0.0;
  
  while (!(shouldQuit || glfwWindowShouldClose(window)))
  {
    double currentTime = glfwGetTime();
    double frameTime = currentTime - lastTime;
    lastTime = currentTime;
    
    glfwPollEvents();
    
    accumulator += frameTime;
    while (accumulator >= dt)
    {
      systemManager.getSystem<ControllingSystem>().tick(dt);
      systemManager.getSystem<PonderingSystem>().tick(dt);
      
      accumulator -= dt;
    }
    
    systemManager.getSystem<RenderingSystem>().tick(frameTime);
  }
}

EntityManager& Game::getEntityManager()
{
  return entityManager;
}


