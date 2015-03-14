#ifndef PLAYER_SPRITE_H
#define PLAYER_SPRITE_H

#include "entity.h"
#include "renderer.h"

class Game;

class PlayerSpriteComponent : public Component {
  public:
    PlayerSpriteComponent();
    void render(Game& game, Entity& entity, Texture& buffer);
    void receive(Game& game, Entity& entity, const Message& msg);
    
  private:
    Texture sprite;
    int animFrame = 0;
    bool facingLeft = false;
    bool isMoving = false;
    bool dying = false;
};

#endif
