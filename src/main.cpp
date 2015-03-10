#include <ctime>
#include <list>
#include "renderer.h"
#include <cstdlib>
#include "game.h"

int main()
{
  srand(time(NULL));
  
  GLFWwindow* window = initRenderer();
  
  // Put this in a block so game goes out of scope before we destroy the renderer
  {
    Game game;
    game.execute(window);
  }
  
  destroyRenderer();
  
  return 0;
}
