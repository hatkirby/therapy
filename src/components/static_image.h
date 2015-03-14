#ifndef STATIC_IMAGE_H
#define STATIC_IMAGE_H

#include "entity.h"
#include "renderer.h"

class Game;

class StaticImageComponent : public Component {
  public:
    StaticImageComponent(const char* filename);
    void render(Game& game, Entity& entity, Texture& buffer);
    
  private:
    Texture sprite;
};

#endif
