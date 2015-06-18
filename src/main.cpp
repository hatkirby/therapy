#include <ctime>
#include <list>
#include <cstdlib>
#include "renderer.h"
#include "muxer.h"
#include "entity_manager.h"
#include "components/sprite_renderable.h"
#include "components/transformable.h"
#include "systems/rendering.h"

int main()
{
  srand(time(NULL));
  
  GLFWwindow* window = initRenderer();
  glfwSwapInterval(1);
  
  initMuxer();
  
  // Put this in a block so game goes out of scope before we destroy the renderer
  {
    EntityManager manager;
    
    int player = manager.emplaceEntity();
    manager.emplaceComponent<SpriteRenderableComponent>(player, "res/Starla.png", 10, 12, 6);
    manager.emplaceComponent<TransformableComponent>(player, 203, 44, 10, 12);
    
    std::list<std::unique_ptr<System>> loop;
    loop.push_back(std::unique_ptr<System>(new RenderingSystem()));
    
    while (!glfwWindowShouldClose(window))
    {
      for (auto& sys : loop)
      {
        sys->tick(manager, 1.0);
      }
      
      glfwPollEvents();
    }
  }
  
  destroyMuxer();
  destroyRenderer();
  
  return 0;
}
