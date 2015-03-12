#ifndef GAME_H
#define GAME_H

class Game;

#include "map.h"
#include <memory>
#include "entity.h"
#include <functional>

const int TILE_WIDTH = 8;
const int TILE_HEIGHT = 8;
const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 200;
const int MAP_WIDTH = GAME_WIDTH/TILE_WIDTH;
const int MAP_HEIGHT = GAME_HEIGHT/TILE_HEIGHT;

const int FRAMES_PER_SECOND = 60;
const double SECONDS_PER_FRAME = 1.0 / FRAMES_PER_SECOND;

struct Savefile {
  const Map* map;
  std::pair<double, double> position;
};

class Game {
  public:
    Game();
    void execute(GLFWwindow* window);
    void loadMap(const Map& map);
    void detectCollision(Entity& collider, std::pair<double, double> old_position);
    void saveGame(const Map& map, std::pair<double, double> position);
    void loadGame(const Map& curMap);
    void schedule(int frames, std::function<void ()>&& callback);
    
  private:
    friend void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    std::list<std::shared_ptr<Entity>> entities;
    std::list<std::shared_ptr<Entity>> nextEntities;
    bool newWorld;
    std::shared_ptr<Entity> player;
    Map m{"../maps/embarass.txt"};
    Map m2{"../maps/second.txt"};
    Savefile save;
    std::list<std::pair<int, std::function<void ()>>> scheduled;
    bool shouldQuit = false;
};

#endif
