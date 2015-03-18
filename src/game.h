#ifndef GAME_H
#define GAME_H

#include <memory>
#include <functional>
#include <list>
#include <map>
#include "map.h"

class Entity;
struct GLFWwindow;

struct Savefile {
  const Map* map;
  std::pair<double, double> position;
};

class Game {
  public:
    Game(const char* maps);
    void execute(GLFWwindow* window);
    void loadMap(const Map& map, std::pair<double, double> position);
    void detectCollision(Entity& collider, std::pair<double, double> old_position);
    void saveGame();
    void schedule(double time, std::function<void ()> callback);
    void playerDie();
    const Map& getMap(int id) const;
    
  private:
    friend void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    std::map<int, Map> maps;
    int startMap;
    std::pair<int, int> startPos;
    std::list<std::shared_ptr<Entity>> entities;
    std::list<std::shared_ptr<Entity>> nextEntities;
    std::pair<double, double> nextPosition;
    bool newWorld;
    std::shared_ptr<Entity> player;
    const Map* currentMap;
    Savefile save;
    std::list<std::pair<double, std::function<void ()>>> scheduled;
    bool shouldQuit = false;
};

#endif
