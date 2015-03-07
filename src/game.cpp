#include "game.h"
#include "renderer.h"

Game::Game()
{
  window = initRenderer();
  glfwSwapInterval(1);
  
  m = new Map("../maps/embarass.txt");
  m2 = new Map("../maps/second.txt");
  
  m->setLeftMap(m2);
  m2->setRightMap(m);
  
  world = new World();
  
  auto player = std::make_shared<Entity>(world);
  
  auto player_input = std::make_shared<UserMovementComponent>(*player);
  player->addComponent(player_input);
  
  auto player_physics = std::make_shared<PlayerPhysicsComponent>(*player);
  player_physics->position = std::make_pair(100.0,100.0);
  player_physics->size = std::make_pair(10.0,12.0);
  player->addComponent(player_physics);
  
  auto player_anim = std::make_shared<PlayerSpriteComponent>(*player, *player_physics);
  player->addComponent(player_anim);
  
  world->addEntity(player);
  world->player = player;
  
  loadMap(m);
}

Game::~Game()
{
  if (world != 0)
  {
    delete world;
  }
  
  if (nextWorld != 0)
  {
    delete nextWorld;
  }
  
  delete m;
  delete m2;
  
  destroyRenderer();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  (void)window;
  (void)scancode;
  (void)mods;
  
  if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
  {
    Game::getInstance().shouldQuit = true;
  }
  
  Game::getInstance().input(key, action);
}

void Game::execute()
{
  glfwSetKeyCallback(window, key_callback);
  
  Texture* buffer = createTexture(GAME_WIDTH, GAME_HEIGHT);

  while (!(shouldQuit || glfwWindowShouldClose(window)))
  {
    // Should we load a new world?
    if (nextWorld != 0)
    {
      delete world;
      world = nextWorld;
      world->player->world = world;
      nextWorld = 0;
    }
    
    // Handle input
    glfwPollEvents();
    
    // Tick!
    world->tick();
  
    // Do rendering
    world->render(buffer);
    renderScreen(buffer);
  }
  
  destroyTexture(buffer);
}

void Game::input(int key, int action)
{
  if (world != NULL)
  {
    world->input(key, action);
  }
}

void Game::loadMap(Map* map)
{
  nextWorld = new World();
  
  nextWorld->player = world->player;
  
  auto mapEn = std::make_shared<Entity>(nextWorld);
  
  auto map_render = std::make_shared<MapRenderComponent>(*mapEn, map);
  mapEn->addComponent(map_render);
  
  auto map_collision = std::make_shared<MapCollisionComponent>(*mapEn, map);
  mapEn->addComponent(map_collision);
  nextWorld->bodies.push_back(map_collision.get());
  
  nextWorld->addEntity(mapEn);
  nextWorld->addEntity(nextWorld->player);
}
