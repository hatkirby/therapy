#include "game.h"
#include "renderer.h"
#include "components.h"

Game::Game()
{ 
  m.setLeftMap(&m2);
  m2.setRightMap(&m);
  
  player = std::make_shared<Entity>();
  player->position = std::make_pair(100.0,100.0);
  player->size = std::make_pair(10.0,12.0);
  
  auto player_input = std::make_shared<UserMovementComponent>();
  player->addComponent(player_input);
  
  auto player_physics = std::make_shared<PlayerPhysicsComponent>();
  player->addComponent(player_physics);
  
  auto player_anim = std::make_shared<PlayerSpriteComponent>();
  player->addComponent(player_anim);
  
  loadMap(m);
}

void key_callback(GLFWwindow* window, int key, int, int action, int)
{
  Game* game = (Game*) glfwGetWindowUserPointer(window);
  
  if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
  {
    game->shouldQuit = true;
  }
  
  for (auto entity : game->entities)
  {
    entity->input(*game, key, action);
  }
}

void Game::execute(GLFWwindow* window)
{
  glfwSwapInterval(1);
  glfwSetWindowUserPointer(window, this);
  glfwSetKeyCallback(window, key_callback);
  
  Texture buffer(GAME_WIDTH, GAME_HEIGHT);

  double lastTime = glfwGetTime();
  int nbFrames = 0;
  while (!(shouldQuit || glfwWindowShouldClose(window)))
  {
    double currentTime = glfwGetTime();
    nbFrames++;
    if (currentTime - lastTime >= 1.0)
    {
      printf("%f ms/frame\n", 1000.0/double(nbFrames));
      nbFrames = 0;
      lastTime += 1.0;
    }
    
    // Should we load a new world?
    if (newWorld)
    {
      newWorld = false;
      entities.clear();
      entities = std::move(nextEntities);
    }
    
    // Handle input
    glfwPollEvents();
    
    // Tick!
    for (auto entity : entities)
    {
      entity->tick(*this);
    }
  
    // Do rendering
    buffer.fill(buffer.entirety(), 0, 0, 0);
    for (auto entity : entities)
    {
      entity->render(*this, buffer);
    }

    buffer.renderScreen();
  }
}

void Game::loadMap(Map& map)
{
  auto mapEn = std::make_shared<Entity>();
  
  auto map_render = std::make_shared<MapRenderComponent>(map);
  mapEn->addComponent(map_render);
  
  auto map_collision = std::make_shared<MapCollisionComponent>(map);
  mapEn->addComponent(map_collision);
  
  nextEntities.clear();
  nextEntities.push_back(mapEn);
  nextEntities.push_back(player);
  
  newWorld = true;
}

void Game::detectCollision(Entity& collider, std::pair<double, double> old_position)
{
  for (auto entity : entities)
  {
    entity->detectCollision(*this, collider, old_position);
  }
}
