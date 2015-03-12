#include <ctime>
#include <list>
#include "renderer.h"
#include <cstdlib>
#include "game.h"
#include "muxer.h"

int main()
{
  srand(time(NULL));
  
  GLFWwindow* window = initRenderer();
  initMuxer();
  
  // Put this in a block so game goes out of scope before we destroy the renderer
  {
    Game game;
    game.execute(window);
  }
  
  destroyMuxer();
  destroyRenderer();
  
  return 0;
}
