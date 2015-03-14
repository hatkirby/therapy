#include "game.h"
#include "renderer.h"
#include "muxer.h"
#include "map.h"
#include "components/user_movement.h"
#include "components/player_physics.h"
#include "components/player_sprite.h"
#include "components/map_render.h"
#include "components/map_collision.h"

Game::Game()
{ 
  player = std::make_shared<Entity>();
  player->position = std::make_pair(100.0,100.0);
  player->size = std::make_pair(10.0,12.0);
  
  auto player_input = std::make_shared<UserMovementComponent>();
  player->addComponent(player_input);
  
  auto player_physics = std::make_shared<PlayerPhysicsComponent>();
  player->addComponent(player_physics);
  
  auto player_anim = std::make_shared<PlayerSpriteComponent>();
  player->addComponent(player_anim);
  
  Map& startingMap = Map::getNamedMap("embarass");
  save = {&startingMap, player->position};
  
  loadMap(startingMap, player->position);
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
  const double dt = 0.01;
  double accumulator = 0.0;
  
  while (!(shouldQuit || glfwWindowShouldClose(window)))
  {
    double currentTime = glfwGetTime();
    double frameTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // Should we load a new world?
    if (newWorld)
    {
      newWorld = false;
      entities.clear();
      entities = std::move(nextEntities);
      
      player->position = nextPosition;
    }
    
    // Handle input
    glfwPollEvents();
    
    // Tick!
    accumulator += frameTime;
    while (accumulator >= dt)
    {
      for (auto entity : entities)
      {
        entity->tick(*this, dt);
      }
      
      accumulator -= dt;
    }
    
    // Do any scheduled tasks
    for (auto& task : scheduled)
    {
      task.first -= frameTime;
      
      if (task.first <= 0)
      {
        task.second();
      }
    }
    
    scheduled.remove_if([] (std::pair<double, std::function<void ()>> value) { return value.first <= 0; });
  
    // Do rendering
    buffer.fill(buffer.entirety(), 0, 0, 0);
    for (auto entity : entities)
    {
      entity->render(*this, buffer);
    }

    buffer.renderScreen();
  }
}

void Game::loadMap(const Map& map, std::pair<double, double> position)
{
  auto mapEn = std::make_shared<Entity>();
  
  auto map_render = std::make_shared<MapRenderComponent>(map);
  mapEn->addComponent(map_render);
  
  auto map_collision = std::make_shared<MapCollisionComponent>(map);
  mapEn->addComponent(map_collision);
  
  // Map in the back, player on top, rest of entities in between
  nextEntities.clear();
  nextEntities.push_back(mapEn);
  map.createEntities(nextEntities);
  nextEntities.push_back(player);
  
  newWorld = true;
  
  currentMap = &map;
  nextPosition = position;
}

void Game::detectCollision(Entity& collider, std::pair<double, double> old_position)
{
  for (auto entity : entities)
  {
    entity->detectCollision(*this, collider, old_position);
  }
}

void Game::saveGame()
{
  save = {currentMap, player->position};
}

void Game::schedule(double time, std::function<void ()> callback)
{
  scheduled.emplace_front(time, std::move(callback));
}

void Game::playerDie()
{
  player->send(*this, Message::Type::die);
  
  playSound("../res/Hit_Hurt5.wav", 0.25);
  
  schedule(0.75, [&] () {
    if (*currentMap != *save.map)
    {
      loadMap(*save.map, save.position);
    } else {
      player->position = save.position;
    }
    
    player->send(*this, Message::Type::stopDying);
  });
}
