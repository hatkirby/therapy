#include "game.h"
#include "renderer.h"
#include "components.h"
#include "muxer.h"
#include "entityfactory.h"
#include "map.h"

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
  
  loadMap(startingMap);
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

void Game::loadMap(const Map& map)
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
}

void Game::detectCollision(Entity& collider, std::pair<double, double> old_position)
{
  for (auto entity : entities)
  {
    entity->detectCollision(*this, collider, old_position);
  }
}

void Game::saveGame(const Map& map, std::pair<double, double> position)
{
  save = {&map, position};
}

void Game::schedule(double time, std::function<void ()> callback)
{
  scheduled.emplace_front(time, std::move(callback));
}

void Game::playerDie(Entity& player, const Map& curMap)
{
  Message msg(Message::Type::die);
  player.send(*this, msg);
  
  playSound("../res/Hit_Hurt5.wav", 0.25);
  
  schedule(0.75, [&] () {
    if (&curMap != save.map)
    {
      loadMap(*(save.map));
    }
    
    player.position = save.position;
    
    Message msg2(Message::Type::stopDying);
    player.send(*this, msg2);
  });
}
