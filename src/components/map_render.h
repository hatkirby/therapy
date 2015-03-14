#ifndef MAP_RENDER_H
#define MAP_RENDER_H

#include "entity.h"
#include "renderer.h"

class Map;
class Game;

class MapRenderComponent : public Component {
  public:
    MapRenderComponent(const Map& map);
    void render(Game& game, Entity& entity, Texture& buffer);
    
  private:
    Texture screen;
};

#endif
