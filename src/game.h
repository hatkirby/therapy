#ifndef GAME_H
#define GAME_H

#include "components.h"

const int TILE_WIDTH = 8;
const int TILE_HEIGHT = 8;
const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 200;
const int MAP_WIDTH = GAME_WIDTH/TILE_WIDTH;
const int MAP_HEIGHT = GAME_HEIGHT/TILE_HEIGHT;

const int FRAMES_PER_SECOND = 60;
const double SECONDS_PER_FRAME = 1.0 / FRAMES_PER_SECOND;

class Game {
  public:
    static Game& getInstance()
    {
      static Game instance;
  
      return instance;
    }
    
    ~Game();
    void execute();
    void loadMap(Map* map);
    void input(int key, int action);
    
    bool shouldQuit = false;
  private:
    Game();
    Game(Game const&);
    void operator=(Game const&);
    
    GLFWwindow* window;
    World* world;
    World* nextWorld;
    Map* m;
    Map* m2;
};

#endif
