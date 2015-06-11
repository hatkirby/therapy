#include <ctime>
#include <list>
#include <cstdlib>
#include "renderer.h"
#include "muxer.h"
#include "entity_manager.h"

int main()
{
  srand(time(NULL));
  
  GLFWwindow* window = initRenderer();
  initMuxer();
  
  // Put this in a block so game goes out of scope before we destroy the renderer
  {
    EntityManager manager;
    
    int eRef = manager.emplaceEntity();
    int eRef2 = manager.emplaceEntity();
    manager.setParent(eRef, eRef2);
    printf("%d\n", manager.getParent(eRef));
  }
  
  destroyMuxer();
  destroyRenderer();
  
  return 0;
}
